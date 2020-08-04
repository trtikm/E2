#ifndef AIOLD_ENV_SINET_MOTION_DESIRE_DECODER_HPP_INCLUDED
#   define AIOLD_ENV_SINET_MOTION_DESIRE_DECODER_HPP_INCLUDED

#   include <aiold/motion_desire_props.hpp>
#   include <aiold/env/sinet/number_decoder.hpp>
#   include <netlab/simple_network_uid.hpp>
#   include <angeo/tensor_math.hpp>
#   include <vector>

namespace aiold { namespace env { namespace sinet {


struct  motion_desire_decoder
{
    motion_desire_decoder(motion_desire_props&  props, natural_8_bit const  layer_index);
    void  next_round(float_32_bit const  time_step_in_seconds, std::vector<netlab::simple::uid> const&  output_events);

private:

    // CONSTANTS:

    natural_8_bit  LAYER_INDEX;
    std::vector<natural_16_bit>  SEPARATOR_UNIT_INDICES;

    // DATA:

    std::vector<number_decoder>  decoders;
};


}}}

#endif
