#include <ai/sight_controller.hpp>
#include <ai/utils_ptree.hpp>
#include <angeo/coordinate_system.hpp>
#include <angeo/skeleton_kinematics.hpp>
#include <angeo/collision_class.hpp>
#include <com/simulation_context.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <unordered_set>
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


std::function<float_32_bit(float_32_bit)>  parse_function(std::string const&  expr)
{
    if (expr == "x")
        return [](float_32_bit const  x) -> float_32_bit { return x; };
    else if (expr == "x*x")
        return [](float_32_bit const  x) -> float_32_bit { return x * x; };
    UNREACHABLE();
}





std::function<bool(com::object_guid, angeo::COLLISION_CLASS)>  parse_filter(
        boost::property_tree::ptree const&  config,
        bool const  ignore_disabled_sensors,
        com::simulation_context const* const  context
        )
{
    ASSUMPTION(context != nullptr);
    std::unordered_set<angeo::COLLISION_CLASS> classes;
    for (auto const&  name_and_props : config)
        classes.insert(angeo::read_collison_class_from_string(get_value<std::string>(name_and_props.second)));
    return [classes, ignore_disabled_sensors, context](com::object_guid const  collider_guid, angeo::COLLISION_CLASS const  cc) {
                if (classes.count(cc) != 0UL)
                    return false;
                switch (cc)
                {
                case angeo::COLLISION_CLASS::SENSOR_WIDE_RANGE:
                case angeo::COLLISION_CLASS::SENSOR_NARROW_RANGE:
                case angeo::COLLISION_CLASS::SENSOR_DEDICATED:
                case angeo::COLLISION_CLASS::RAY_CAST_TARGET:
                    return !ignore_disabled_sensors || context->is_collider_enabled(collider_guid);
                default:
                    return true;
                }
            };
    UNREACHABLE();
}


}}}

namespace ai {


sight_controller::camera_config::camera_config(boost::property_tree::ptree const&  config)
    : camera_config(
            get_value<float_32_bit>("horizontal_fov_angle", config),
            get_value<float_32_bit>("vertical_fov_angle", config),
            get_value<float_32_bit>("near_plane", config),
            get_value<float_32_bit>("far_plane", config),
            get_value<float_32_bit>("origin_z_shift", config)
            )
{}


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
        boost::property_tree::ptree const&  config,
        simulation_context_const_ptr const  context
        )
    : ray_cast_config(
            get_value<bool>("do_directed_ray_casts", config),
            get_value<natural_32_bit>("num_random_ray_casts_per_second", config),
            get_value<bool>("do_update_depth_image", config),
            get_value<float_32_bit>("max_ray_cast_info_life_time_in_seconds", config),
            get_value<natural_16_bit>("num_cells_along_x_axis", config),
            get_value<natural_16_bit>("num_cells_along_y_axis", config),
            detail::parse_function(get_value<std::string>("distribution_of_cells_in_camera_space", config)),
            detail::parse_filter(
                    get_ptree("collider_filter", config),
                    get_value<bool>("ignore_disabled_sensors", config),
                    context.get()),
            detail::parse_function(get_value<std::string>("depth_image_func", config))
            )
{}


sight_controller::ray_cast_config::ray_cast_config(
        bool const  do_directed_ray_casts_,
        natural_32_bit const  num_random_ray_casts_per_second_,
        bool const  do_update_depth_image_,
        float_32_bit const  max_ray_cast_info_life_time_in_seconds_,
        natural_16_bit const  num_cells_along_x_axis_,
        natural_16_bit const  num_cells_along_y_axis_,
        std::function<float_32_bit(float_32_bit)> const&  distribution_of_cells_in_camera_space_,
        std::function<bool(com::object_guid, angeo::COLLISION_CLASS)> const&  collider_filter_,
        std::function<float_32_bit(float_32_bit)>  depth_image_func_
        )
    : do_directed_ray_casts(do_directed_ray_casts_)
    , num_random_ray_casts_per_second(num_random_ray_casts_per_second_)
    , do_update_depth_image(do_update_depth_image_)
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

    update_camera(time_step_in_seconds);
    if (get_camera() == nullptr)
        return;

    m_current_time += time_step_in_seconds;

    matrix44  from_camera_matrix;
    angeo::from_base_matrix(*get_camera()->coordinate_system(), from_camera_matrix);

    if (m_ray_cast_config.do_directed_ray_casts)
    {
        erase_obsolete_ray_casts(m_directed_ray_casts_in_time);
        perform_directed_ray_casts(from_camera_matrix);
    }
    if (m_ray_cast_config.num_random_ray_casts_per_second > 0U)
    {
        erase_obsolete_ray_casts(m_random_ray_casts_in_time);
        perform_random_ray_casts(time_step_in_seconds, from_camera_matrix);
    }

    if (m_ray_cast_config.do_update_depth_image)
    {
        std::fill(m_depth_image.begin(), m_depth_image.end(), 0.0f);
        update_depth_image(m_directed_ray_casts_in_time);
        update_depth_image(m_random_ray_casts_in_time);
    }
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


void  sight_controller::perform_directed_ray_casts(matrix44 const&  from_camera_matrix)
{
    com::simulation_context const&  ctx = *m_binding->context;

    matrix44  to_camera_matrix;
    angeo::to_base_matrix(*get_camera()->coordinate_system(), to_camera_matrix);

    auto const  collider_acceptor = 
        [this, &ctx, &to_camera_matrix, &from_camera_matrix](com::object_guid const  collider_guid) -> bool {
            if (!get_ray_cast_config().collider_filter(collider_guid, ctx.collision_class_of(collider_guid)))
                return true;
            angeo::coordinate_system const&  frame = ctx.frame_coord_system_in_world_space(ctx.frame_of_collider(collider_guid));
            vector3 const  origin = transform_point(frame.origin(), to_camera_matrix);
            vector2  camera_coords_01;
            if (!get_camera()->pixel_coordinates_in_01_of_point_in_camera_space(origin, camera_coords_01))
                return true;
            ray_cast_info  info;
            if (perform_ray_cast(camera_coords_01, from_camera_matrix, info)/* && info.collider_guid == collider_guid */)
                m_directed_ray_casts_in_time.insert({ m_current_time, info });
            return true;
        };

    for (com::simulation_context::agent_guid_iterator  agent_it = ctx.agents_begin(), agent_end = ctx.agents_end();
            agent_it != agent_end; ++agent_it)
    {
        std::unordered_set<com::object_guid> const&  agent_colliders = ctx.colliders_of_agent(*agent_it);
        for (auto  collider_it = agent_colliders.begin(); collider_it != agent_colliders.end(); ++collider_it)
            collider_acceptor(*collider_it);
    }

    for (com::simulation_context::sensor_guid_iterator  sensor_it = ctx.sensors_begin(), sensor_end = ctx.sensors_end();
            sensor_it != sensor_end; ++sensor_it)
        collider_acceptor(ctx.collider_of_sensor(*sensor_it));
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
        vector2 const&  camera_coords_01,
        matrix44 const&  from_camera_matrix,
        ray_cast_info&  result
        ) const
{
    result.cell_x = std::max(0U, std::min(m_ray_cast_config.num_cells_along_x_axis - 1U,
                        (natural_32_bit)(camera_coords_01(0) * (m_ray_cast_config.num_cells_along_x_axis - 1U) + 0.5f)));
    result.cell_y = std::max(0U, std::min(m_ray_cast_config.num_cells_along_y_axis - 1U,
                        (natural_32_bit)(camera_coords_01(1) * (m_ray_cast_config.num_cells_along_y_axis - 1U) + 0.5f)));

    vector3  ray_begin_in_camera, ray_end_in_camera;
    get_camera()->ray_points_in_camera_space(camera_coords_01, ray_begin_in_camera, ray_end_in_camera);

    result.ray_origin_in_world_space = transform_point(ray_begin_in_camera, from_camera_matrix);;
    vector3 const  ray_end = transform_point(ray_end_in_camera, from_camera_matrix);

    result.collider_guid = ctx().ray_cast_to_nearest_collider(
            result.ray_origin_in_world_space,
            ray_end,
            true,
            true,
            0U,
            &result.parameter_to_coid,
            m_ray_cast_config.collider_filter
            );
    if (result.collider_guid == com::invalid_object_guid())
        return false;

    result.camera_coords_of_cell_coords = contract32(ray_begin_in_camera);
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
