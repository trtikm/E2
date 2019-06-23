#ifndef AI_BLACKBOARD_HUMAN_HPP_INCLUDED
#   define AI_BLACKBOARD_HUMAN_HPP_INCLUDED

#   include <ai/blackboard.hpp>
#   include <memory>

namespace ai {


struct  blackboard_human : public blackboard
{
    blackboard_human()
        : m_cortex_cmd_move_intensity(0.0f)
        , m_cortex_cmd_turn_intensity(0.0f)
        , m_max_forward_speed_in_meters_per_second(7.5f)
        , m_max_turn_speed_in_radians_per_second(PI() * 1.0f)
    {}

    float_32_bit  m_cortex_cmd_move_intensity;  // in range <0,1>; 0 - 0 - no move; 1 - max move forward.
    float_32_bit  m_cortex_cmd_turn_intensity;  // in range <-1,1>; -1 - max turn left; 0 - no turn; 1 - max turn right.

    // Constants:

    float_32_bit  m_max_forward_speed_in_meters_per_second;
    float_32_bit  m_max_turn_speed_in_radians_per_second;
};


using  blackboard_human_ptr = std::shared_ptr<blackboard_human>;
using  blackboard_human_const_ptr = std::shared_ptr<blackboard_human const>;


}

#endif
