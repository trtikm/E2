#ifndef AI_CORTEX_MOCK_HPP_INCLUDED
#   define AI_CORTEX_MOCK_HPP_INCLUDED

#   include <ai/cortex.hpp>
#   include <osi/window_props.hpp>
#   include <osi/keyboard_props.hpp>
#   include <osi/mouse_props.hpp>

namespace ai {


struct  cortex_mock : public cortex
{
    cortex_mock(agent const*  myself_) : cortex(myself_) {}
    void  next_round(float_32_bit const  time_step_in_seconds, mock_input_props const&  mock_input) override;
};


struct  cortex_mock_optional : public cortex_mock
{
    cortex_mock_optional(agent const*  myself_, bool const  use_mock_)
        : cortex_mock(myself_)
        , m_use_mock(use_mock_)
    {}
    bool  use_mock() const { return m_use_mock; }
    void  next_round(float_32_bit const  time_step_in_seconds, mock_input_props const&  mock_input) override
    { if (m_use_mock) cortex_mock::next_round(time_step_in_seconds, mock_input); }
private:
   bool  m_use_mock;
};


}

#endif
