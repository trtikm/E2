#ifndef AI_CORTEX_OUTPUT_DECODER_HPP_INCLUDED
#   define AI_CORTEX_OUTPUT_DECODER_HPP_INCLUDED

#   include <ai/cortex_io.hpp>
#   include <ai/blackboard.hpp>

namespace ai {


struct  cortex_output_decoder
{
    cortex_output_decoder(cortex_io_ptr const  io, blackboard_ptr const  blackboard_)
        : m_io(io)
        , m_blackboard(blackboard_)
    {}

    virtual ~cortex_output_decoder() {}

    virtual void  run() {}

    cortex_io_ptr  get_io() const { return m_io; }
    blackboard_ptr  get_blackboard() const { return m_blackboard; }

private:
    cortex_io_ptr  m_io;
    blackboard_ptr  m_blackboard;
};


}

#endif