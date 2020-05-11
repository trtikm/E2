#ifndef AI_MOTION_DESIRE_PROPS_HPP_INCLUDED
#   define AI_MOTION_DESIRE_PROPS_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>

namespace ai {


struct DESIRE_COORD
{
    static natural_8_bit constexpr  FORWARD     = 0U;
    static natural_8_bit constexpr  LEFT        = 1U;
    static natural_8_bit constexpr  UP          = 2U;
    static natural_8_bit constexpr  TURN_CCW    = 3U;
};



struct  motion_desire_props
{
    motion_desire_props();

    // Values of all fields are in cortex's LOGICAL space which lies inside agent's local space.
    // The struct DESIRE_COORD defines an interpretation of coordinates in the logical space
    // in agent space. Action controller is responsible for correct transformation of these
    // values to the agent's local space (and back).

    vector4  speed;
    vector3  look_at_target;
    vector3  aim_at_target;
};


}

#endif
