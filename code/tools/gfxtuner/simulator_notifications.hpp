#ifndef E2_TOOL_GFXTUNER_SIMULATOR_NOTIFICATIONS_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_SIMULATOR_NOTIFICATIONS_HPP_INCLUDED

#   include <string>

namespace simulator_notifications {


inline std::string  paused() { return "PAUSED"; }

inline std::string  camera_position_updated() { return "CAMERA_POSITION_UPDATED"; }
inline std::string  camera_orientation_updated() { return "CAMERA_ORIENTATION_UPDATED"; }

inline std::string  scene_node_position_updated() { return "SCENE_NODE_POSITION_UPDATED"; }
inline std::string  scene_node_orientation_updated() { return "SCENE_NODE_ORIENTATION_UPDATED"; }

inline std::string  scene_edit_mode_changed() { return "SCENE_EDIT_MODE_CHANGED"; }
inline std::string  scene_scene_selection_changed() { return "SCENE_SELECTION_CHANGED"; }


}


#endif
