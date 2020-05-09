#include <ai/cortex_random.hpp>
#include <ai/action_controller.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai { namespace detail { namespace {


vector3  gen_random_vector(
        float_32_bit const  min_coord,
        float_32_bit const  max_coord,
        random_generator_for_natural_32_bit& generator
        )
{
    return { get_random_float_32_bit_in_range(min_coord, max_coord, generator),
             get_random_float_32_bit_in_range(min_coord, max_coord, generator),
             get_random_float_32_bit_in_range(min_coord, max_coord, generator) };
}

vector3  gen_random_unit_vector(
        float_32_bit const  min_coord,
        float_32_bit const  max_coord,
        random_generator_for_natural_32_bit& generator
        )
{
    if (std::fabs(max_coord - min_coord) < 1e-3f)
        return vector3_unit_y();
    while (true)
    {
        vector3 const  candidate = gen_random_vector(min_coord, max_coord, generator);
        float_32_bit const  len = length(candidate);
        if (len > 0.001f)
            return (1.0f / len) * candidate;
    }
}


}}}

namespace ai {


cortex_random::cortex_random(blackboard_agent_weak_ptr const  blackboard_)
    : cortex(blackboard_)
    , m_seconds_till_change(0.0f)
    , m_generator()
{}


void  cortex_random::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    m_seconds_till_change -= time_step_in_seconds;
    if (m_seconds_till_change > 0.0f)
        return;
    m_seconds_till_change = get_random_float_32_bit_in_range(0.0f, 2.0f, m_generator);

    m_motion_desire_props.speed(DESIRE_COORD::FORWARD) = get_random_float_32_bit_in_range(-1.0f, 1.0f, m_generator);
    m_motion_desire_props.speed(DESIRE_COORD::LEFT) = get_random_float_32_bit_in_range(-1.0f, 1.0f, m_generator);
    m_motion_desire_props.speed(DESIRE_COORD::UP) = get_random_float_32_bit_in_range(-1.0f, 1.0f, m_generator);
    m_motion_desire_props.speed(DESIRE_COORD::TURN_CCW) = get_random_float_32_bit_in_range(-1.0f, 1.0f, m_generator);
    m_motion_desire_props.look_at_target = detail::gen_random_vector(-50.0f, 50.0f, m_generator);
}


}
