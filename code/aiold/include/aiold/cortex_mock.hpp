#ifndef AIOLD_CORTEX_MOCK_HPP_INCLUDED
#   define AIOLD_CORTEX_MOCK_HPP_INCLUDED

#   include <aiold/cortex.hpp>
#   include <aiold/input_devices.hpp>

namespace aiold {


struct  cortex_mock : public cortex
{
    cortex_mock(blackboard_agent_weak_ptr const  blackboard_, input_devices_const_ptr const  input_devices_)
        : cortex(blackboard_)
        , m_input_devices(input_devices_)
    {}

    input_devices_const_ptr  get_input_devices() const { return m_input_devices; }

    void  next_round(float_32_bit const  time_step_in_seconds) override;

private:
    input_devices_const_ptr  m_input_devices;
};


}

#endif
