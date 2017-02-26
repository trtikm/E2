#ifndef E2_TOOL_GFXTUNER_SIMULATOR_NOTIFICATIONS_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_SIMULATOR_NOTIFICATIONS_HPP_INCLUDED

#   include <string>

namespace simulator_notifications {


inline std::string  camera_position_updated() { return "CAMERA_POSITION_UPDATED"; }
inline std::string  camera_orientation_updated() { return "CAMERA_ORIENTATION_UPDATED"; }
inline std::string  paused() { return "PAUSED"; }


}


#endif
