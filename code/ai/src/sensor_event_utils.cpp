#include <ai/sensor_event_utils.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


scene::request_merge_scene_ptr  create_request_merge_scene(
        std::string const&  scene_id_,
        scene::node_id const&  parent_nid_,
        scene::node_id const&  frame_reference_nid_,
        vector3 const&  linear_velocity_,
        vector3 const&  angular_velocity_,
        scene::node_id const&  velocities_frame_nid_
        )
{
    return std::make_shared<scene::request_merge_scene const>(
            scene_id_,
            parent_nid_,
            frame_reference_nid_,
            linear_velocity_,
            angular_velocity_,
            velocities_frame_nid_
            );
}


scene::request_merge_scene_ptr  create_request_merge_scene(property_map const&  props)
{
    return  create_request_merge_scene(
                props.get_string("scene_id"),
                as_scene_node_id(props.get_string("parent_nid")),
                as_scene_node_id(props.get_string("frame_nid")),
                props.has_vector3("linear_velocity") ? props.get_vector3("linear_velocity") : vector3_zero(),
                props.has_vector3("angular_velocity") ? props.get_vector3("angular_velocity") : vector3_zero(),
                props.has("velocities_frame_nid") ? as_scene_node_id(props.get_string("velocities_frame_nid")) : scene::node_id()
                );
}


scene::request_erase_nodes_tree_ptr  create_request_erase_nodes_tree(scene::node_id const&  root_nid_)
{
    return std::make_shared<scene::request_erase_nodes_tree const>(root_nid_);
}


}
