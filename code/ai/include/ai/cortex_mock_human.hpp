#ifndef AI_CORTEX_MOCK_HUMAN_HPP_INCLUDED
#   define AI_CORTEX_MOCK_HUMAN_HPP_INCLUDED

#   include <ai/cortex_mock.hpp>
#   include <ai/input_devices.hpp>
#   include <utility/basic_numeric_types.hpp>

namespace ai {


struct cortex_mock_human : public cortex_mock
{
    cortex_mock_human(blackboard_weak_ptr const  blackboard_, input_devices_const_ptr const  input_devices_);

    void  next_round(float_32_bit const  time_step_in_seconds) override;

    // Services for the action controller:

    skeletal_motion_templates::cursor_and_transition_time  choose_next_motion_action(
            std::vector<skeletal_motion_templates::cursor_and_transition_time> const& possibilities
            ) const override;

private:

    void  update_motion_intensities(float_32_bit const  time_step_in_seconds);
    void  update_motion_desire_props();
    void  update_look_at_target_in_local_space();


    float_32_bit  m_move_intensity;
    float_32_bit  m_turn_intensity;
    float_32_bit  m_elevation_intensity;

    // Constants:

    float_32_bit  m_max_forward_speed_in_meters_per_second;
    float_32_bit  m_max_turn_speed_in_radians_per_second;
};


using  cortex_mock_human_ptr = std::shared_ptr<cortex_mock_human>;
using  cortex_mock_human_const_ptr = std::shared_ptr<cortex_mock_human const>;


}

#endif
