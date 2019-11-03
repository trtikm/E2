#ifndef AI_CORTEX_INPUT_ENCODER_HPP_INCLUDED
#   define AI_CORTEX_INPUT_ENCODER_HPP_INCLUDED

#   include <ai/cortex_io.hpp>
#   include <ai/blackboard.hpp>

namespace ai {


struct  cortex_input_encoder
{
    cortex_input_encoder(cortex_io_ptr const  io, blackboard_weak_ptr const  blackboard_)
        : m_io(io)
        , m_blackboard(blackboard_)
    {}

    virtual ~cortex_input_encoder() {}

    virtual void  run() {}

    cortex_io_ptr  get_io() const { return m_io; }
    blackboard_const_ptr  get_blackboard() const { return m_blackboard.lock(); }

private:
    cortex_io_ptr  m_io;
    blackboard_weak_ptr  m_blackboard;
};


}

#endif
