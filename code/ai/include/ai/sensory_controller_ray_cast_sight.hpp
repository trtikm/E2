#ifndef AI_SENSORY_CONTROLLER_RAY_CAST_SIGHT_HPP_INCLUDED
#   define AI_SENSORY_CONTROLLER_RAY_CAST_SIGHT_HPP_INCLUDED

#   include <ai/blackboard.hpp>
#   include <ai/sensory_controller_sight.hpp>
#   include <angeo/collision_object_id.hpp>
#   include <angeo/tensor_math.hpp>
#   include <utility/random.hpp>
#   include <unordered_map>
#   include <map>
#   include <chrono>

namespace ai {


struct  sensory_controller_ray_cast_sight : public sensory_controller_sight
{
    struct  ray_cast_config
    {
        natural_32_bit  num_raycasts_per_second;
        float_32_bit  max_coid_life_time_in_seconds;

        ray_cast_config(
                natural_32_bit const  num_raycasts_per_second_ = 100U,
                float_32_bit const  max_coid_life_time_in_seconds_ = 10.0f
                );
    };

    using  coids_with_last_seen_times = std::unordered_map<angeo::collision_object_id, std::chrono::system_clock::time_point>;

    sensory_controller_ray_cast_sight(
            blackboard_weak_ptr const  blackboard_,
            sensory_controller_sight::camera_config const&  camera_config_,
            ray_cast_config const& ray_cast_config_
            );

    virtual ~sensory_controller_ray_cast_sight() {}

    virtual void  next_round(float_32_bit const  time_step_in_seconds);

    ray_cast_config const&  get_ray_cast_config() const { return m_ray_cast_config; }
    coids_with_last_seen_times const&  get_coids_with_last_seen_times() const { return m_coids_with_times; }

private:
    ray_cast_config  m_ray_cast_config;
    coids_with_last_seen_times  m_coids_with_times;
    std::multimap<std::chrono::system_clock::time_point, angeo::collision_object_id>  m_from_times_to_coids;
    std::normal_distribution<float_32_bit>  m_distribution;
    random_generator_for_natural_32_bit  m_generator;
};


using  sensory_controller_ray_cast_sight_ptr = std::shared_ptr<sensory_controller_ray_cast_sight>;


}

#endif