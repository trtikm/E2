#ifndef AI_CORTEX_MOCK_HPP_INCLUDED
#   define AI_CORTEX_MOCK_HPP_INCLUDED

#   include <ai/cortex.hpp>
#   include <ai/input_devices.hpp>

namespace ai {


struct  cortex_mock : public cortex
{
    cortex_mock(blackboard_weak_ptr const  blackboard_, input_devices_const_ptr const  input_devices_)
        : cortex(blackboard_)
        , m_input_devices(input_devices_)
    {}

    input_devices_const_ptr  get_input_devices() const { return m_input_devices; }

private:
    input_devices_const_ptr  m_input_devices;
};


}

#endif
