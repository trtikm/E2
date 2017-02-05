#ifndef E2_TOOL_NETVIEWER_SIMULATOR_NOTIFICATIONS_HPP_INCLUDED
#   define E2_TOOL_NETVIEWER_SIMULATOR_NOTIFICATIONS_HPP_INCLUDED

#   include <string>

namespace simulator_notifications {


inline std::string  camera_position_updated() { return "CAMERA_POSITION_UPDATED"; }
inline std::string  camera_orientation_updated() { return "CAMERA_ORIENTATION_UPDATED"; }
inline std::string  paused() { return "PAUSED"; }
inline std::string  on_wrong_index_of_layer_to_render() { return "ON_WRONG_INDEX_OF_LAYER_TO_RENDER"; }
//inline std::string  selection_changed() { return "SELECTION_CHANGED"; }


}


#endif
