#ifndef AI_AGENT_HPP_INCLUDED
#   define AI_AGENT_HPP_INCLUDED

#   include <ai/cortex.hpp>
#   include <ai/cortex_mock.hpp>
#   include <ai/cortex_input_encoder.hpp>
#   include <ai/cortex_output_decoder.hpp>
#   include <ai/sensory_controller.hpp>
#   include <ai/action_controller.hpp>
#   include <ai/blackboard.hpp>
#   include <vector>
#   include <memory>

namespace ai {


struct agent
{
    agent() {}

    agent(blackboard_ptr const  blackboard_,
          input_devices_const_ptr const  input_devices_,
          skeletal_motion_templates::motion_template_cursor const&  start_pose);

    void  next_round(float_32_bit const  time_step_in_seconds);
    
    void  set_use_cortex_mock(bool const  state) { if (state != uses_cortex_mock()) m_cortex_primary.swap(m_cortex_secondary); }
    bool  uses_cortex_mock() const { return dynamic_cast<cortex_mock*>(m_cortex_primary.get()) != nullptr; }

    blackboard_ptr  get_blackboard() { return m_blackboard; }

private:
    std::unique_ptr<cortex>  m_cortex_primary;
    std::unique_ptr<cortex>  m_cortex_secondary;
    std::unique_ptr<cortex_input_encoder>  m_cortex_input_encoder;
    std::unique_ptr<cortex_output_decoder>  m_cortex_output_decoder;
    std::unique_ptr<sensory_controller>  m_sensory_controller;
    std::unique_ptr<action_controller>  m_action_controller;
    blackboard_ptr  m_blackboard;
};


}

#endif
