#ifndef AI_SENSOR_EVENT_UTILS_HPP_INCLUDED
#   define AI_SENSOR_EVENT_UTILS_HPP_INCLUDED

#   include <ai/property_map.hpp>
#   include <ai/sensor_action.hpp>
#   include <ai/scene.hpp>
#   include <angeo/tensor_math.hpp>

namespace ai {


scene::request_merge_scene_ptr  create_request_merge_scene(
        std::string const&  scene_id_,
        scene::node_id const&  parent_nid_,
        scene::node_id const&  frame_reference_nid_,
        vector3 const&  linear_velocity_,
        vector3 const&  angular_velocity_,
        scene::node_id const&  velocities_frame_nid_
        );


scene::request_merge_scene_ptr  create_request_merge_scene(property_map const&  props);


scene::request_erase_nodes_tree_ptr  create_request_erase_nodes_tree(scene::node_id const&  root_nid_);


}

#endif
