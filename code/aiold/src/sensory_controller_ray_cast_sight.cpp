#include <aiold/sensory_controller_ray_cast_sight.hpp>
#include <aiold/action_controller.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <algorithm>

namespace aiold { namespace detail { namespace {


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

namespace aiold {

    
sensory_controller_ray_cast_sight::ray_cast_config::ray_cast_config(
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


sensory_controller_ray_cast_sight::ray_cast_info::ray_cast_info(
        natural_32_bit const  cell_x_,
        natural_32_bit const  cell_y_,
        vector2 const&  camera_coords_of_cell_coords_,
        vector3 const&  ray_origin_in_world_space_,
        vector3 const&  ray_direction_in_world_space_,
        float_32_bit const  parameter_to_coid_,
        angeo::collision_object_id const  coid_
        )
    : cell_x(cell_x_)
    , cell_y(cell_y_)
    , camera_coords_of_cell_coords(camera_coords_of_cell_coords_)
    , ray_origin_in_world_space(ray_origin_in_world_space_)
    , ray_direction_in_world_space(ray_direction_in_world_space_)
    , parameter_to_coid(parameter_to_coid_)
    , coid(coid_)
{}


sensory_controller_ray_cast_sight::sensory_controller_ray_cast_sight(
        blackboard_agent_weak_ptr const  blackboard_,
        sensory_controller_sight::camera_config const&  camera_config_,
        ray_cast_config const& ray_cast_config_)
    : sensory_controller_sight(blackboard_, camera_config_)
    , m_ray_cast_config(ray_cast_config_)
    , m_ray_casts_in_time()
    , m_generator()
    , m_time_buffer(0.0f)
{}


void  sensory_controller_ray_cast_sight::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    std::chrono::system_clock::time_point const  now = std::chrono::system_clock::now();
    auto const  num_seconds_till_now = [&now](std::chrono::system_clock::time_point const  time_point) {
        return std::chrono::duration<float_32_bit>(now - time_point).count();
    };

    while (!m_ray_casts_in_time.empty() && num_seconds_till_now(m_ray_casts_in_time.begin()->first) >= m_ray_cast_config.max_ray_cast_info_life_time_in_seconds)
        m_ray_casts_in_time.erase(m_ray_casts_in_time.begin());

    sensory_controller_sight::next_round(time_step_in_seconds); // Updates the camera.
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

        vector2 const  camera_coords_01 =
                detail::compute_ray_cast_camera_coordinates_from_cell_coordinates(
                        cell_x,
                        cell_y,
                        m_ray_cast_config.num_cells_along_any_axis,
                        m_ray_cast_config.distribution_of_cells_in_camera_space
                        );
        vector3 const  camera_coords{
                (get_camera()->left() + camera_coords_01(0) * (get_camera()->right() - get_camera()->left())),
                (get_camera()->bottom() + camera_coords_01(1) * (get_camera()->top() - get_camera()->bottom())),
                -get_camera()->near_plane()
                };
        float_32_bit const  scale_to_far_plane = get_camera()->far_plane() / get_camera()->near_plane();

        vector3 const&  ray_origin = get_camera()->coordinate_system()->origin();
        vector3 const  ray_end = transform_point(scale_to_far_plane * camera_coords, W);
        scene::collision_object_id  nearest_coid;
        float_32_bit  parameter_to_nearest_coid;
        if (!get_blackboard()->m_scene->get_collision_scene().ray_cast(ray_origin, ray_end, true, true, &nearest_coid,
                                                                       &parameter_to_nearest_coid, nullptr))
            continue;
        m_ray_casts_in_time.insert({
                now,
                {
                    cell_x,
                    cell_y,
                    contract32(camera_coords),
                    ray_origin,
                    ray_end - ray_origin,
                    parameter_to_nearest_coid,
                    nearest_coid
                    }
                });
    }
}


}
