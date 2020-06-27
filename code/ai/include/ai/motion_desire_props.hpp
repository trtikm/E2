#ifndef AI_MOTION_DESIRE_PROPS_HPP_INCLUDED
#   define AI_MOTION_DESIRE_PROPS_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>

namespace ai {


struct DESIRE_COORD
{
    // Use these to access coords of motion_desire_props::speed
    static natural_8_bit constexpr  FORWARD     = 0U;
    static natural_8_bit constexpr  LEFT        = 1U;
    static natural_8_bit constexpr  UP          = 2U;
    static natural_8_bit constexpr  TURN_CCW    = 3U;

    // Use these to access coords of motion_desire_props::guesture_subject and guesture_sign
    static natural_8_bit constexpr  HEAD        = 0U;
    static natural_8_bit constexpr  TAIL        = 1U;
};


struct  motion_desire_props
{
    motion_desire_props();

    // Values of all fields are in cortex's LOGICAL space which lies inside agent's local space.
    // The struct DESIRE_COORD defines an interpretation of coordinates in the logical space
    // in agent space. Action controller is responsible for correct transformation of these
    // values to the agent's local space (and back).

    vector4  speed;     // xyz represent vector in the logical space (use FORWARD, LEFT, UP),
                        // and w is the rotation/turning (use TURN_CCW); all 4 coords are in <-1,1> 

    vector2  guesture_subject;  // Use HEAD and TAIL for accessing coords; all coords are in <-1,1> 
    vector2  guesture_sign;     // Use HEAD and TAIL for accessing coords; all coords are in <0,1>
    float_32_bit  guesture_intensity;   // in <0,1>

    vector3  look_at_target;
    vector3  aim_at_target;
};


}

#endif
