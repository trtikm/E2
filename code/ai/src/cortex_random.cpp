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


cortex_random::cortex_random(blackboard_weak_ptr const  blackboard_)
    : cortex(blackboard_)
    , m_generator()
{}


void  cortex_random::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    m_motion_desire_props.forward_unit_vector_in_local_space = detail::gen_random_unit_vector(-1.0f, 1.0f, m_generator);
    m_motion_desire_props.linear_velocity_unit_direction_in_local_space = detail::gen_random_unit_vector(-1.0f, 1.0f, m_generator);
    m_motion_desire_props.linear_speed = get_random_float_32_bit_in_range(0.0f, 10.0f, m_generator);
    m_motion_desire_props.angular_velocity_unit_axis_in_local_space = detail::gen_random_unit_vector(-1.0f, 1.0f, m_generator);
    m_motion_desire_props.angular_speed = get_random_float_32_bit_in_range(0.0f, 10.0f, m_generator);
    m_motion_desire_props.look_at_target_in_local_space = detail::gen_random_vector(-50.0f, 50.0f, m_generator);
}


}
