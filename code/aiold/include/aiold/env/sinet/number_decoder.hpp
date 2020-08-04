#ifndef AIOLD_ENV_SINET_NUMBER_DECODER_HPP_INCLUDED
#   define AIOLD_ENV_SINET_NUMBER_DECODER_HPP_INCLUDED

#   include <netlab/simple_network.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <vector>
#   include <unordered_set>

namespace aiold { namespace env { namespace sinet {


struct  number_decoder
{
    number_decoder(
            float_32_bit const  min_value,
            float_32_bit const  max_value,
            float_32_bit const  charge_change_per_spike,
            float_32_bit const  charge_decay_coef,
            float_32_bit const  charge_decay_time_step,
            float_32_bit const  charge_to_speed_coef,
            float_32_bit* const  output_value_ptr_
            );

    void  next_round(float_32_bit const  time_step_in_seconds);

    void  on_spike(float_32_bit const  sign) { charge += sign * CHARGE_CHANGE_PER_SPIKE; }

private:

    // CONSTANTS:

    float_32_bit  MIN_VALUE;
    float_32_bit  MAX_VALUE;
    float_32_bit  CHARGE_CHANGE_PER_SPIKE;
    float_32_bit  CHARGE_DECAY_COEF;
    float_32_bit  CHARGE_DECAY_TIME_STEP;
    float_32_bit  CHARGE_TO_SPEED_COEF;

    // DATA:

    float_32_bit  time_buffer;
    float_32_bit  charge;
    float_32_bit*  output_value_ptr;
};


}}}

#endif
