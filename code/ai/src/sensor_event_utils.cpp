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
        vector3 const&  angular_velocity_
        )
{
    return std::make_shared<scene::request_merge_scene const>(
            scene_id_,
            parent_nid_,
            frame_reference_nid_,
            linear_velocity_,
            angular_velocity_
            );
}


scene::request_merge_scene_ptr  create_request_merge_scene(property_map const&  props)
{
    return  create_request_merge_scene(
                props.at("scene_id").get_string(),
                ::as_scene_node_id(props.at("parent_nid").get_string()),
                ::as_scene_node_id(props.at("frame_reference_nid").get_string()),
                vector3(props.at("linear_velocity_x").get_float(),
                        props.at("linear_velocity_y").get_float(),
                        props.at("linear_velocity_z").get_float()
                        ),
                vector3(props.at("angular_velocity_x").get_float(),
                        props.at("angular_velocity_y").get_float(),
                        props.at("angular_velocity_z").get_float()
                        )
                );
}


scene::request_erase_nodes_tree_ptr  create_request_erase_nodes_tree(scene::node_id const&  root_nid_)
{
    return std::make_shared<scene::request_erase_nodes_tree const>(root_nid_);
}


}
