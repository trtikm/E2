#ifndef AI_CORTEX_HPP_INCLUDED
#   define AI_CORTEX_HPP_INCLUDED

#   include <ai/cortex_io.hpp>
#   include <utility/basic_numeric_types.hpp>

namespace ai {


struct  cortex
{
    cortex(cortex_io_ptr const  io)
        : m_io(io)
    {}

    virtual ~cortex() {}

    virtual void  next_round(float_32_bit const  time_step_in_seconds);

    cortex_io_ptr  get_io() const { return m_io; }

private:
    cortex_io_ptr  m_io;
};


}

#endif
