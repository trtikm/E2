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


vector2  compute_ray_cast_camera_coordinates_from_cell_coordinates(
        natural_32_bit const  cell_x,
        natural_32_bit const  cell_y,
        natural_16_bit const  num_cells_along_any_axis,
        std::function<float_32_bit(float_32_bit)> const&  distribution_of_cells_in_camera_space
        )
{
    ASSUMPTION(num_cells_along_any_axis > 0U && cell_x < num_cells_along_any_axis && cell_y < num_cells_along_any_axis);

    auto const  from_01_to_m1p1 = [num_cells_along_any_axis](natural_32_bit const  cell_coord) {
        return num_cells_along_any_axis < 2U ?
                    0.0f :
                    2.0f * ((float_32_bit)cell_coord / (float_32_bit)(num_cells_along_any_axis - 1U) - 0.5f);
    };
    auto const  from_m1p1_to_01 = [](float_32_bit const  x) {
        return std::max(0.0f, std::min(1.0f, 0.5f * x + 0.5f));
    };
    vector2 const  u(from_01_to_m1p1(cell_x), from_01_to_m1p1(cell_y));
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
        float_32_bit const  fov_angle_,
        float_32_bit const  near_plane_,
        float_32_bit const  far_plane_,
        float_32_bit const  origin_z_shift_
        )
    : fov_angle(fov_angle_)
    , near_plane(near_plane_)
    , far_plane(far_plane_)
    , origin_z_shift(origin_z_shift_)
{}


sight_controller::ray_cast_config::ray_cast_config(
        natural_32_bit const  num_raycasts_per_second_,
        float_32_bit const  max_ray_cast_info_life_time_in_seconds_,
        natural_16_bit const  num_cells_along_any_axis_,
        std::function<float_32_bit(float_32_bit)> const&  distribution_of_cells_in_camera_space_
        )
    : num_raycasts_per_second(num_raycasts_per_second_)
    , max_ray_cast_info_life_time_in_seconds(max_ray_cast_info_life_time_in_seconds_)
    , num_cells_along_any_axis(num_cells_along_any_axis_)
    , distribution_of_cells_in_camera_space(distribution_of_cells_in_camera_space_)
{
    ASSUMPTION(
        num_raycasts_per_second > 0U &&
        max_ray_cast_info_life_time_in_seconds >= 0.0f &&
        num_cells_along_any_axis > 0U &&
        distribution_of_cells_in_camera_space.operator bool()
        );
}


sight_controller::ray_cast_info::ray_cast_info(
        natural_32_bit const  cell_x_,
        natural_32_bit const  cell_y_,
        vector2 const&  camera_coords_of_cell_coords_,
        vector3 const&  ray_origin_in_world_space_,
        vector3 const&  ray_unit_direction_in_world_space_,
        float_32_bit const  parameter_to_coid_,
        com::object_guid const  collider_guid_
        )
    : cell_x(cell_x_)
    , cell_y(cell_y_)
    , camera_coords_of_cell_coords(camera_coords_of_cell_coords_)
    , ray_origin_in_world_space(ray_origin_in_world_space_)
    , ray_unit_direction_in_world_space(ray_unit_direction_in_world_space_)
    , parameter_to_coid(parameter_to_coid_)
    , collider_guid(collider_guid_)
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
    , m_ray_casts_in_time()
    , m_generator()
    , m_time_buffer(0.0f)
    , m_motion_templates(motion_templates)
    , m_binding(binding)
{}


void  sight_controller::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    std::chrono::system_clock::time_point const  now = std::chrono::system_clock::now();
    auto const  num_seconds_till_now = [&now](std::chrono::system_clock::time_point const  time_point) {
        return std::chrono::duration<float_32_bit>(now - time_point).count();
    };

    while (!m_ray_casts_in_time.empty() && num_seconds_till_now(m_ray_casts_in_time.begin()->first) >= m_ray_cast_config.max_ray_cast_info_life_time_in_seconds)
        m_ray_casts_in_time.erase(m_ray_casts_in_time.begin());

    update_camera(time_step_in_seconds);
    if (get_camera() == nullptr)
        return;

    matrix44  W;
    angeo::from_base_matrix(*get_camera()->coordinate_system(), W);

    float_32_bit const  ray_cast_duration = 1.0f / (float_32_bit)m_ray_cast_config.num_raycasts_per_second;
    for (m_time_buffer += time_step_in_seconds; m_time_buffer >= ray_cast_duration; m_time_buffer -= ray_cast_duration)
    {
        natural_32_bit const  cell_x =
                get_random_natural_32_bit_in_range(0U, m_ray_cast_config.num_cells_along_any_axis - 1U, m_generator);
        natural_32_bit const  cell_y =
                get_random_natural_32_bit_in_range(0U, m_ray_cast_config.num_cells_along_any_axis - 1U, m_generator);

        vector2 const  camera_coords =
                detail::compute_ray_cast_camera_coordinates_from_cell_coordinates(
                        cell_x,
                        cell_y,
                        m_ray_cast_config.num_cells_along_any_axis,
                        m_ray_cast_config.distribution_of_cells_in_camera_space
                        );

        vector3 const  ray_direction_local{
                (get_camera()->left() + camera_coords(0) * (get_camera()->right() - get_camera()->left())),
                (get_camera()->bottom() + camera_coords(1) * (get_camera()->top() - get_camera()->bottom())),
                -get_camera()->near_plane()
                };

        vector3 const&  ray_origin = get_camera()->coordinate_system()->origin();
        vector3 const  ray_unit_direction = normalised(transform_vector(ray_direction_local, W));
        float_32_bit const  ray_length = get_camera()->far_plane();
        float_32_bit  parameter_to_nearest_collider;
        com::object_guid const  nearest_collider_guid = ctx().ray_cast_to_nearest_collider(
                ray_origin,
                ray_unit_direction,
                ray_length,
                true,
                true,
                &parameter_to_nearest_collider,
                nullptr
                );
        if (nearest_collider_guid == com::invalid_object_guid())
            continue;

        m_ray_casts_in_time.insert({
                now,
                {
                    cell_x,
                    cell_y,
                    camera_coords,
                    ray_origin,
                    ray_unit_direction,
                    parameter_to_nearest_collider * ray_length,
                    nearest_collider_guid
                    }
                });
    }
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
        float_32_bit const  tan_half_FOV_angle = std::tanf(m_camera_config.fov_angle * 0.5f);
        float_32_bit const  camera_window_half_size = m_camera_config.near_plane * tan_half_FOV_angle;

        m_camera = gfx::camera_perspective::create(
                        angeo::coordinate_system::create(vector3_zero(), quaternion_identity()),
                        m_camera_config.near_plane,
                        m_camera_config.far_plane,
                        -camera_window_half_size,
                        camera_window_half_size,
                        -camera_window_half_size,
                        camera_window_half_size
                        );
    }

    m_camera->set_coordinate_system(camera_coord_system);
}


}
