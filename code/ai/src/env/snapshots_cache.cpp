#include <ai/env/snapshots_cache.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <algorithm>

namespace ai { namespace env {


snapshots_cache::config::config(
        natural_32_bit const  num_snapshots_per_second_,
        float_32_bit const  max_snapshot_life_time_in_seconds_
        )
    : num_snapshots_per_second(num_snapshots_per_second_)
    , max_snapshot_life_time_in_seconds(max_snapshot_life_time_in_seconds_)
{
    ASSUMPTION(num_snapshots_per_second > 0U);
}


snapshots_cache::snapshots_cache(blackboard_weak_const_ptr const  blackboard_ptr, config const& cfg)
    : m_blackboard(blackboard_ptr)
    , m_config(cfg)
    , m_cache()
    , m_time_buffer(0.0f)
{}


void  snapshots_cache::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    std::chrono::system_clock::time_point const  now = std::chrono::system_clock::now();
    auto const  num_seconds_till_now = [&now](std::chrono::system_clock::time_point const  time_point) {
        return std::chrono::duration<float_32_bit>(now - time_point).count();
    };

    while (!m_cache.empty() && num_seconds_till_now(m_cache.begin()->first) >= m_config.max_snapshot_life_time_in_seconds)
        m_cache.erase(m_cache.begin());

    float_32_bit const  snapshot_duration = 1.0f / (float_32_bit)m_config.num_snapshots_per_second;
    for (m_time_buffer += time_step_in_seconds; m_time_buffer >= snapshot_duration; m_time_buffer -= snapshot_duration)
        m_cache.insert({ now, create_snapshot(m_blackboard) });
}


snapshots_cache_ptr  create_snapshots_cache(blackboard_weak_const_ptr const  blackboard_ptr, snapshots_cache::config const& cfg)
{
    return std::make_shared<snapshots_cache>(blackboard_ptr, cfg);
}


}}
