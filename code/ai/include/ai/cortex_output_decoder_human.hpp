#ifndef AI_CORTEX_OUTPUT_DECODER_HUMAN_HPP_INCLUDED
#   define AI_CORTEX_OUTPUT_DECODER_HUMAN_HPP_INCLUDED

#   include <ai/cortex_output_decoder.hpp>
#   include <ai/cortex_io.hpp>
#   include <ai/blackboard.hpp>

namespace ai {


struct  cortex_output_decoder_human : public cortex_output_decoder 
{
    cortex_output_decoder_human(cortex_io_ptr const  io, blackboard_ptr const  blackboard_)
        : cortex_output_decoder(io, blackboard_)
    {}

    void  run() override;
};


}

#endif