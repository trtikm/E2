#ifndef AI_CORTEX_MOCK_HUMAN_HPP_INCLUDED
#   define AI_CORTEX_MOCK_HUMAN_HPP_INCLUDED

#   include <ai/cortex_mock.hpp>
#   include <ai/cortex_io.hpp>
#   include <ai/input_devices.hpp>
#   include <utility/basic_numeric_types.hpp>

namespace ai {


struct cortex_mock_human : public cortex_mock
{
    cortex_mock_human(cortex_io_ptr const  io, input_devices_const_ptr const  input_devices_)
        : cortex_mock(io, input_devices_)
    {}

    void  next_round(float_32_bit const  time_step_in_seconds) override;
};


}

#endif
