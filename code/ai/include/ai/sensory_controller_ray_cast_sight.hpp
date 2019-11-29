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
        float_32_bit  max_ray_cast_info_life_time_in_seconds;

        ray_cast_config(
                natural_32_bit const  num_raycasts_per_second_ = 600U,
                float_32_bit const  max_ray_cast_info_life_time_in_seconds_ = 0.1f
                );
    };

    struct  ray_cast_info
    {
        vector3  ray_origin;
        vector3  ray_unit_direction;
        float_32_bit  parameter_to_coid;
        angeo::collision_object_id  coid;

        ray_cast_info(
                vector3 const&  ray_origin_,
                vector3 const&  ray_unit_direction_,
                float_32_bit const  parameter_to_coid_,
                angeo::collision_object_id const  coid_
                );
    };

    using  ray_casts_in_time = std::multimap<std::chrono::system_clock::time_point, ray_cast_info>;

    sensory_controller_ray_cast_sight(
            blackboard_weak_ptr const  blackboard_,
            sensory_controller_sight::camera_config const&  camera_config_,
            ray_cast_config const& ray_cast_config_
            );

    virtual ~sensory_controller_ray_cast_sight() {}

    virtual void  next_round(float_32_bit const  time_step_in_seconds);

    ray_cast_config const&  get_ray_cast_config() const { return m_ray_cast_config; }
    ray_casts_in_time const&  get_ray_casts_in_time() const { return m_ray_casts_in_time; }

private:
    ray_cast_config  m_ray_cast_config;
    ray_casts_in_time  m_ray_casts_in_time;
    std::normal_distribution<float_32_bit>  m_distribution;
    random_generator_for_natural_32_bit  m_generator;
    float_32_bit  m_time_buffer;
};


using  sensory_controller_ray_cast_sight_ptr = std::shared_ptr<sensory_controller_ray_cast_sight>;
using  sensory_controller_ray_cast_sight_const_ptr = std::shared_ptr<sensory_controller_ray_cast_sight const>;


}

#endif
