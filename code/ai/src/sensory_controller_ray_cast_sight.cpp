#include <ai/sensory_controller_ray_cast_sight.hpp>
#include <ai/action_controller.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <algorithm>

namespace ai {

    
sensory_controller_ray_cast_sight::ray_cast_config::ray_cast_config(
        natural_32_bit const  num_raycasts_per_second_,
        float_32_bit const  max_coid_life_time_in_seconds_,
        float_32_bit const  max_ray_cast_info_life_time_in_seconds_
        )
    : num_raycasts_per_second(num_raycasts_per_second_)
    , max_coid_life_time_in_seconds(max_coid_life_time_in_seconds_)
    , max_ray_cast_info_life_time_in_seconds(max_ray_cast_info_life_time_in_seconds_)
{}


sensory_controller_ray_cast_sight::ray_cast_info::ray_cast_info(
        vector3 const&  ray_origin_,
        vector3 const&  ray_unit_direction_,
        float_32_bit const  parameter_to_coid_,
        angeo::collision_object_id const  coid_
        )
    : ray_origin(ray_origin_)
    , ray_unit_direction(ray_unit_direction_)
    , parameter_to_coid(parameter_to_coid_)
    , coid(coid_)
{}


sensory_controller_ray_cast_sight::sensory_controller_ray_cast_sight(
        blackboard_weak_ptr const  blackboard_,
        sensory_controller_sight::camera_config const&  camera_config_,
        ray_cast_config const& ray_cast_config_)
    : sensory_controller_sight(blackboard_, camera_config_)
    , m_ray_cast_config(ray_cast_config_)
    , m_coids_with_times()
    , m_from_times_to_coids()
    , m_distribution()
    , m_generator()
{}


void  sensory_controller_ray_cast_sight::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    std::chrono::system_clock::time_point const  now = std::chrono::system_clock::now();
    auto const  num_seconds_till_now = [&now](std::chrono::system_clock::time_point const  time_point) {
        return std::chrono::duration<float_32_bit>(now - time_point).count();
    };

    while (!m_from_times_to_coids.empty() &&
           num_seconds_till_now(m_from_times_to_coids.begin()->first) >= m_ray_cast_config.max_coid_life_time_in_seconds)
    {
        auto const  it = m_coids_with_times.find(m_from_times_to_coids.begin()->second);
        if (it != m_coids_with_times.end() && it->second == m_from_times_to_coids.begin()->first)
            m_coids_with_times.erase(it);
        m_from_times_to_coids.erase(m_from_times_to_coids.begin());
    }

    while (!m_ray_casts_as_performed_in_time.empty() &&
           num_seconds_till_now(m_ray_casts_as_performed_in_time.begin()->first) >= m_ray_cast_config.max_ray_cast_info_life_time_in_seconds)
    {
        m_ray_casts_as_performed_in_time.erase(m_ray_casts_as_performed_in_time.begin());
    }

    sensory_controller_sight::next_round(time_step_in_seconds); // Updates the camera.
    if (get_camera() == nullptr)
        return;

    std::unordered_set<scene::collision_object_id>  ignored;
    get_blackboard()->m_scene->get_coids_under_scene_node_subtree(
            get_blackboard()->m_action_controller->get_motion_object_node_id(),
            [&ignored](scene::collision_object_id const  coid) { ignored.insert(coid); return true; }
            );

    auto const  gen_random_number_in_01 = [this]() {
        return std::min(1.0f, std::max(0.0f, (1.0f + m_distribution(m_generator) / 4.0f) / 2.0f));
    };

    matrix44  W;
    angeo::from_base_matrix(*get_camera()->coordinate_system(), W);

    natural_32_bit const  num_raycasts = std::max(1U, (natural_32_bit)(m_ray_cast_config.num_raycasts_per_second * time_step_in_seconds + 0.5));
    for (natural_32_bit raycast_idx = 0U; raycast_idx < num_raycasts; ++raycast_idx)
    {
        vector3 const  ray_direction_local{
                (get_camera()->left() + gen_random_number_in_01() * (get_camera()->right() - get_camera()->left())),
                (get_camera()->bottom() + gen_random_number_in_01() * (get_camera()->top() - get_camera()->bottom())),
                -get_camera()->near_plane()
                };

        vector3 const&  ray_origin = get_camera()->coordinate_system()->origin();
        vector3 const  ray_unit_direction = normalised(transform_vector(ray_direction_local, W));
        float_32_bit const  ray_length = get_camera()->far_plane();
        scene::collision_object_id  nearest_coid;
        float_32_bit  parameter_to_nearest_coid;
        if (!get_blackboard()->m_scene->get_collision_scene().ray_cast(ray_origin, ray_unit_direction, ray_length, true, true, &nearest_coid, &parameter_to_nearest_coid, &ignored))
            continue;

        m_coids_with_times.erase(nearest_coid);
        m_coids_with_times[nearest_coid] = now;
        m_from_times_to_coids.insert({ now, nearest_coid });

        m_ray_casts_as_performed_in_time.insert({ now, {ray_origin, ray_unit_direction, parameter_to_nearest_coid * ray_length, nearest_coid} });
    }
}


}
