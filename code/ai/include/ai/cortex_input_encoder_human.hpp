#ifndef AI_CORTEX_INPUT_ENCODER_HUMAN_HPP_INCLUDED
#   define AI_CORTEX_INPUT_ENCODER_HUMAN_HPP_INCLUDED

#   include <ai/cortex_input_encoder.hpp>
#   include <ai/cortex_io.hpp>
#   include <ai/blackboard_human.hpp>

namespace ai {


struct  cortex_input_encoder_human : public cortex_input_encoder
{
    cortex_input_encoder_human(cortex_io_ptr const  io, blackboard_const_ptr const  blackboard_)
        : cortex_input_encoder(io, blackboard_)
    {}

    blackboard_human_const_ptr  get_blackboard() const { return as<blackboard_human const>(cortex_input_encoder::get_blackboard()); }

    void  run() override;
};


}

#endif
