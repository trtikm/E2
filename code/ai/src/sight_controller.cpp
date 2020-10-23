#include <ai/sight_controller.hpp>
#include <angeo/coordinate_system.hpp>
#include <angeo/skeleton_kinematics.hpp>
#include <com/simulation_context.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <algorithm>

namespace ai { namespace detail { namespace {


vector2  generate_random_ray_cast_camera_coordinates(
        std::function<float_32_bit(float_32_bit)> const&  distribution_of_cells_in_camera_space,
        random_generator_for_natural_32_bit&  generator
        )
{
    auto const  from_m1p1_to_01 = [](float_32_bit const  x) {
        return std::max(0.0f, std::min(1.0f, 0.5f * x + 0.5f));
    };
    vector2 const  u{
            get_random_float_32_bit_in_range(-1.0f, 1.0f, generator),
            get_random_float_32_bit_in_range(-1.0f, 1.0f, generator)
            };
    vector2 const  abs_u(std::fabs(u(0)), std::fabs(u(1)));
    static int const  I0[3] = { 0, 1, 0 };
    int const* const  I = I0 + (abs_u(0) >= abs_u(1) ? 0 : 1);
    if (abs_u(I[0]) < 1e-5f)
        return vector2_zero();
    else
    {
        vector2 v;
        v(I[0]) = distribution_of_cells_in_camera_space(abs_u(I[0])) * (u(I[0]) >= 0.0f ? 1.0f : -1.0f);
        INVARIANT(std::fabs(v(I[0])) < 1.0f + 1e-5f);
        v(I[1]) = u(I[1]) / u(I[0]) * v(I[0]);
        return { from_m1p1_to_01(v(0)), from_m1p1_to_01(v(1)) };
    }
}


}}}

namespace ai {


sight_controller::camera_config::camera_config(
        float_32_bit const  horizontal_fov_angle_,
        float_32_bit const  vertical_fov_angle_,
        float_32_bit const  near_plane_,
        float_32_bit const  far_plane_,
        float_32_bit const  origin_z_shift_
        )
    : horizontal_fov_angle(horizontal_fov_angle_)
    , vertical_fov_angle(vertical_fov_angle_)
    , near_plane(near_plane_)
    , far_plane(far_plane_)
    , origin_z_shift(origin_z_shift_)
{}


sight_controller::ray_cast_config::ray_cast_config(
        bool const  do_directed_ray_casts_,
        natural_32_bit const  num_random_ray_casts_per_second_,
        float_32_bit const  max_ray_cast_info_life_time_in_seconds_,
        natural_16_bit const  num_cells_along_x_axis_,
        natural_16_bit const  num_cells_along_y_axis_,
        std::function<float_32_bit(float_32_bit)> const&  distribution_of_cells_in_camera_space_,
        std::function<bool(com::object_guid, angeo::COLLISION_CLASS)> const&  collider_filter_,
        std::function<float_32_bit(float_32_bit)>  depth_image_func_
        )
    : do_directed_ray_casts(do_directed_ray_casts_)
    , num_random_ray_casts_per_second(num_random_ray_casts_per_second_)
    , max_ray_cast_info_life_time_in_seconds(max_ray_cast_info_life_time_in_seconds_)
    , num_cells_along_x_axis(num_cells_along_x_axis_)
    , num_cells_along_y_axis(num_cells_along_y_axis_)
    , distribution_of_cells_in_camera_space(distribution_of_cells_in_camera_space_)
    , collider_filter(collider_filter_)
    , depth_image_func(depth_image_func_)
{
    ASSUMPTION(
        max_ray_cast_info_life_time_in_seconds >= 0.0f &&
        num_cells_along_x_axis > 0U &&
        num_cells_along_y_axis > 0U &&
        distribution_of_cells_in_camera_space.operator bool() &&
        collider_filter.operator bool() &&
        depth_image_func.operator bool()
        );
}


sight_controller::ray_cast_info::ray_cast_info(
        natural_32_bit const  cell_x_,
        natural_32_bit const  cell_y_,
        vector2 const&  camera_coords_of_cell_coords_,
        vector3 const&  ray_origin_in_world_space_,
        vector3 const&  ray_direction_in_world_space_,
        float_32_bit const  parameter_to_coid_,
        com::object_guid const  collider_guid_,
        float_32_bit const  depth_inverted_
        )
    : cell_x(cell_x_)
    , cell_y(cell_y_)
    , camera_coords_of_cell_coords(camera_coords_of_cell_coords_)
    , ray_origin_in_world_space(ray_origin_in_world_space_)
    , ray_direction_in_world_space(ray_direction_in_world_space_)
    , parameter_to_coid(parameter_to_coid_)
    , collider_guid(collider_guid_)
    , depth_inverted(depth_inverted_)
{}


sight_controller::sight_controller(
        camera_config const&  camera_config_,
        ray_cast_config const& ray_cast_config_,
        skeletal_motion_templates const  motion_templates,
        scene_binding_ptr const  binding
        )
    : m_camera_config(camera_config_)
    , m_camera()
    , m_ray_cast_config(ray_cast_config_)
    , m_directed_ray_casts_in_time()
    , m_random_ray_casts_in_time()
    , m_depth_image(ray_cast_config_.num_cells_along_x_axis * ray_cast_config_.num_cells_along_y_axis, 0.0f)
    , m_current_time(0.0)
    , m_generator()
    , m_random_ray_casts_time_buffer(0.0f)
    , m_motion_templates(motion_templates)
    , m_binding(binding)
{}


void  sight_controller::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    if (!m_ray_cast_config.do_directed_ray_casts && m_ray_cast_config.num_random_ray_casts_per_second == 0U)
        return;

    m_current_time += time_step_in_seconds;

    erase_obsolete_ray_casts(m_directed_ray_casts_in_time);
    erase_obsolete_ray_casts(m_random_ray_casts_in_time);

    update_camera(time_step_in_seconds);
    if (get_camera() == nullptr)
        return;

    {
        matrix44  from_camera_matrix;
        angeo::from_base_matrix(*get_camera()->coordinate_system(), from_camera_matrix);
        perform_random_ray_casts(time_step_in_seconds, from_camera_matrix);
    }

    std::fill(m_depth_image.begin(), m_depth_image.end(), 0.0f);
    update_depth_image(m_directed_ray_casts_in_time);
    update_depth_image(m_random_ray_casts_in_time);
}


void  sight_controller::erase_obsolete_ray_casts(ray_casts_in_time&  ray_casts)
{
    while (!ray_casts.empty() && (float_32_bit)(m_current_time - ray_casts.begin()->first) >=
                                           m_ray_cast_config.max_ray_cast_info_life_time_in_seconds)
        ray_casts.erase(ray_casts.begin());
}


void  sight_controller::update_camera(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    angeo::coordinate_system  camera_coord_system;
    {
        TMPROF_BLOCK();

        std::vector<std::pair<vector3, vector3> >  look_at_props;
        std::vector<vector3>  eye_right_directions;
        for (auto const&  name_and_info : get_motion_templates().look_at())
        {
            angeo::coordinate_system const&  frame = ctx().frame_coord_system_in_world_space(
                    get_binding()->frame_guids_of_bones.at(name_and_info.second->end_effector_bone)
                    );
            look_at_props.push_back({ frame.origin(), angeo::axis_y(frame) });
            eye_right_directions.push_back(angeo::axis_x(frame));
        }

        if (look_at_props.empty())
        {
            m_camera = nullptr;
            return;
        }

        vector3  camera_origin;
        {
            camera_origin = vector3_zero();
            for (auto const&  pos_and_dir : look_at_props)
                camera_origin += pos_and_dir.first;
            camera_origin = (1.0f / (float_32_bit)look_at_props.size()) * camera_origin;
            //// Let us predict/estimate the position of the origin at the next round.
            //camera_origin += (1.0f * time_step_in_seconds) * get_blackboard()->m_action_controller->get_motion_object_motion().velocity.m_linear;
        }
        vector3 const  look_at_target =
                look_at_props.size() == 1UL ?
                        camera_origin + look_at_props.front().second :
                        angeo::compute_common_look_at_target_for_multiple_eyes(look_at_props);
        vector3  camera_z_axis;
        {
            camera_z_axis = camera_origin - look_at_target;

            float_32_bit const  d = length(camera_z_axis);
            if (std::fabs(d) < 0.001f)
            {
                m_camera = nullptr;
                return;
            }

            camera_z_axis = (1.0f / d) * camera_z_axis;
        }
        vector3  camera_x_axis, camera_y_axis;
        {
            camera_x_axis = vector3_zero();
            for (auto const& dir : eye_right_directions)
                camera_x_axis += dir;
            camera_x_axis = (1.0f / (float_32_bit)eye_right_directions.size()) * camera_x_axis;
            camera_y_axis = cross_product(camera_z_axis, camera_x_axis);

            float_32_bit const  d = length(camera_y_axis);
            if (std::fabs(d) < 0.001f)
            {
                m_camera = nullptr;
                return;
            }

            camera_y_axis = (1.0f / d) * camera_y_axis;
            camera_x_axis = cross_product(camera_y_axis, camera_z_axis);
        }
        matrix33 R;
        basis_to_rotation_matrix(camera_x_axis, camera_y_axis, camera_z_axis, R);

        camera_coord_system = { camera_origin + m_camera_config.origin_z_shift * camera_z_axis, rotation_matrix_to_quaternion(R) };
    }

    if (m_camera == nullptr)
    {
        float_32_bit const  tan_half_HFOV_angle = std::tanf(m_camera_config.horizontal_fov_angle * 0.5f);
        float_32_bit const  camera_window_half_width = m_camera_config.near_plane * tan_half_HFOV_angle;

        float_32_bit const  tan_half_VFOV_angle = std::tanf(m_camera_config.vertical_fov_angle * 0.5f);
        float_32_bit const  camera_window_half_height = m_camera_config.near_plane * tan_half_VFOV_angle;

        m_camera = gfx::camera_perspective::create(
                        angeo::coordinate_system::create(vector3_zero(), quaternion_identity()),
                        m_camera_config.near_plane,
                        m_camera_config.far_plane,
                        -camera_window_half_width,
                        camera_window_half_width,
                        -camera_window_half_height,
                        camera_window_half_height
                        );
    }

    m_camera->set_coordinate_system(camera_coord_system);
}


void  sight_controller::perform_random_ray_casts(float_32_bit const  time_step_in_seconds, matrix44 const&  from_camera_matrix)
{
    float_32_bit const  ray_cast_duration = 1.0f / (float_32_bit)m_ray_cast_config.num_random_ray_casts_per_second;
    for (m_random_ray_casts_time_buffer += time_step_in_seconds;
         m_random_ray_casts_time_buffer >= ray_cast_duration;
         m_random_ray_casts_time_buffer -= ray_cast_duration)
    {
        ray_cast_info  info;
        if (perform_ray_cast(
                    detail::generate_random_ray_cast_camera_coordinates(
                            m_ray_cast_config.distribution_of_cells_in_camera_space,
                            m_generator
                            ),
                    from_camera_matrix,
                    info))
            m_random_ray_casts_in_time.insert({ m_current_time, info });

    }
}


bool  sight_controller::perform_ray_cast(
        vector2 const  camera_coords_01,
        matrix44 const&  from_camera_matrix,
        ray_cast_info&  result
        ) const
{
    result.cell_x = std::max(0U, std::min(m_ray_cast_config.num_cells_along_x_axis - 1U,
                        (natural_32_bit)(camera_coords_01(0) * (m_ray_cast_config.num_cells_along_x_axis - 1U) + 0.5f)));
    result.cell_y = std::max(0U, std::min(m_ray_cast_config.num_cells_along_y_axis - 1U,
                        (natural_32_bit)(camera_coords_01(1) * (m_ray_cast_config.num_cells_along_y_axis - 1U) + 0.5f)));

    vector3 const  camera_coords{
            (get_camera()->left() + camera_coords_01(0) * (get_camera()->right() - get_camera()->left())),
            (get_camera()->bottom() + camera_coords_01(1) * (get_camera()->top() - get_camera()->bottom())),
            -get_camera()->near_plane()
            };
    float_32_bit const  scale_to_far_plane = get_camera()->far_plane() / get_camera()->near_plane();

    result.ray_origin_in_world_space = get_camera()->coordinate_system()->origin();
    vector3 const  ray_end = transform_point(scale_to_far_plane * camera_coords, from_camera_matrix);
    result.collider_guid = ctx().ray_cast_to_nearest_collider(
            result.ray_origin_in_world_space,
            ray_end,
            true,
            true,
            &result.parameter_to_coid,
            m_ray_cast_config.collider_filter
            );
    if (result.collider_guid == com::invalid_object_guid())
        return false;

    result.camera_coords_of_cell_coords = contract32(camera_coords);
    result.ray_direction_in_world_space = ray_end - result.ray_origin_in_world_space;

    auto const  to01 = [](float_32_bit const  x, float_32_bit const  lo, float_32_bit const  hi) {
        return (x - lo) / (hi - lo);
    };

    result.depth_inverted =
            to01(1.0f / result.parameter_to_coid, 1.0f / get_camera()->far_plane(), 1.0f / get_camera()->near_plane());

    return true;
}


void  sight_controller::update_depth_image(ray_casts_in_time const&  ray_casts)
{
    for (auto  it = ray_casts.begin(); it != ray_casts.end(); ++it)
    {
        natural_32_bit const  index = it->second.cell_x + it->second.cell_y * m_ray_cast_config.num_cells_along_x_axis;
        float_32_bit const  value =
                std::max(0.0f, std::min(1.0f, m_ray_cast_config.depth_image_func(it->second.depth_inverted)));
        m_depth_image.at(index) = value;
    }
}


}
