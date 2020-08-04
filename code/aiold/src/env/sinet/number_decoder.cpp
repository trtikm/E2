#include <aiold/env/sinet/number_decoder.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>

namespace aiold { namespace env { namespace sinet {


number_decoder::number_decoder(
        float_32_bit const  min_value,
        float_32_bit const  max_value,
        float_32_bit const  charge_change_per_spike,
        float_32_bit const  charge_decay_coef,
        float_32_bit const  charge_decay_time_step,
        float_32_bit const  charge_to_speed_coef,
        float_32_bit* const  output_value_ptr_
        )
    : MIN_VALUE(min_value)
    , MAX_VALUE(max_value)
    , CHARGE_CHANGE_PER_SPIKE(charge_change_per_spike)
    , CHARGE_DECAY_COEF(charge_decay_coef)
    , CHARGE_DECAY_TIME_STEP(charge_decay_time_step)
    , CHARGE_TO_SPEED_COEF(charge_to_speed_coef)
    , time_buffer(0.0f)
    , charge(0.0f)
    , output_value_ptr(output_value_ptr_)
{
    ASSUMPTION(
            MIN_VALUE < MAX_VALUE &&
            CHARGE_CHANGE_PER_SPIKE > 0.0f &&
            CHARGE_DECAY_COEF > 0.0f && CHARGE_DECAY_COEF < 1.0f &&
            CHARGE_DECAY_TIME_STEP > 0.0001f &&
            CHARGE_TO_SPEED_COEF > 0.0f &&
            output_value_ptr != nullptr
            );
    *output_value_ptr = 0.5f * (min_value + max_value);
}


void  number_decoder::next_round(float_32_bit const  time_step_in_seconds)
{
    for (time_buffer += time_step_in_seconds; time_buffer >= CHARGE_DECAY_TIME_STEP; time_buffer -= CHARGE_DECAY_TIME_STEP)
    {
        *output_value_ptr += charge * CHARGE_TO_SPEED_COEF;
        if (*output_value_ptr < MIN_VALUE)
            *output_value_ptr = MIN_VALUE;
        else if (*output_value_ptr > MAX_VALUE)
            *output_value_ptr = MAX_VALUE;
        charge *= CHARGE_DECAY_COEF;
    }
}


}}}
