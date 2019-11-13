#ifndef AI_ENV_SNAPSHOTS_CACHE_HPP_INCLUDED
#   define AI_ENV_SNAPSHOTS_CACHE_HPP_INCLUDED

#   include <ai/blackboard.hpp>
#   include <ai/env/snapshot.hpp>
#   include <angeo/tensor_math.hpp>
#   include <utility/random.hpp>
#   include <unordered_map>
#   include <map>
#   include <chrono>
#   include <memory>

namespace ai { namespace env {


struct  snapshots_cache
{
    struct  config
    {
        natural_32_bit  num_snapshots_per_second;
        float_32_bit  max_snapshot_life_time_in_seconds;

        config(
            natural_32_bit const  num_snapshots_per_second_ = 120U,
            float_32_bit const  max_snapshot_life_time_in_seconds_ = 0.1f
            );
    };

    using  snapshots_as_taken_in_time = std::multimap<std::chrono::system_clock::time_point, snapshot_const_ptr>;

    snapshots_cache(blackboard_weak_const_ptr const  blackboard_ptr, config const&  cfg);

    void  next_round(float_32_bit const  time_step_in_seconds);

    snapshots_as_taken_in_time const&  get_snapshots_as_taken_in_time() const { return m_cache; }

private:
    blackboard_weak_const_ptr  m_blackboard;
    config  m_config;
    random_generator_for_natural_32_bit  m_generator;
    snapshots_as_taken_in_time  m_cache;
    float_32_bit  m_time_buffer;
};


using  snapshots_cache_ptr = std::shared_ptr<snapshots_cache>;


snapshots_cache_ptr  create_snapshots_cache(blackboard_weak_const_ptr const  blackboard_ptr, snapshots_cache::config const& cfg);


}}

#endif
