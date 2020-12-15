#ifndef AI_AGENT_SYSTEM_STATE_HPP_INCLUDED
#   define AI_AGENT_SYSTEM_STATE_HPP_INCLUDED

#   include <angeo/coordinate_system.hpp>

namespace ai {


struct  agent_system_state
{
    angeo::coordinate_system_explicit  motion_object_frame;
    angeo::coordinate_system_explicit  camera_frame;
};


}

#endif
