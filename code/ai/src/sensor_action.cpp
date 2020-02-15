#include <ai/sensor_action.hpp>
#include <utility/invariants.hpp>
#include <unordered_map>
#include <functional>

namespace ai {


static std::unordered_map<natural_8_bit, std::pair<std::string, std::string> > const  from_index_to_name_and_description = {
    { as_number(SENSOR_ACTION_KIND::BEGIN_OF_LIFE), { "BEGIN_OF_LIFE",
            "Imports a template scene with agent, device, or sensor into the current one."
            } },

    { as_number(SENSOR_ACTION_KIND::ENABLE_SENSOR), { "ENABLE_SENSOR",
            "Enable the sensor, i.e. the sensor will start to sense and send corresponding events to the owner."
            } },
    { as_number(SENSOR_ACTION_KIND::DISABLE_SENSOR), { "DISABLE_SENSOR",
            "Disable the sensor, i.e. the sensor will stop to sense and send corresponding events to the owner."
            } },

    { as_number(SENSOR_ACTION_KIND::SET_LINEAR_VELOCITY), { "SET_LINEAR_VELOCITY",
            "Set linear velocity of a rigid body scene node under an agent or device node."
            } },
    { as_number(SENSOR_ACTION_KIND::SET_ANGULAR_VELOCITY), { "SET_ANGULAR_VELOCITY",
            "Set angular velocity of a rigid body scene node under an agent or device node."
            } },
    { as_number(SENSOR_ACTION_KIND::SET_LINEAR_ACCELERATION), { "SET_LINEAR_ACCELERATION",
            "Set linear acceleration of a rigid body scene node under an agent or device node."
            } },
    { as_number(SENSOR_ACTION_KIND::SET_ANGULAR_ACCELERATION), { "SET_ANGULAR_ACCELERATION",
            "Set angular acceleration of a rigid body scene node under an agent or device node."
            } },

    { as_number(SENSOR_ACTION_KIND::SET_MASS_INVERTED), { "SET_MASS_INVERTED",
            "Set inverted mass of a rigid body scene node under an agent or device node."
            } },
    { as_number(SENSOR_ACTION_KIND::SET_INERTIA_TENSOR_INVERTED), { "SET_INERTIA_TENSOR_INVERTED",
            "Set inverted inertia tensor of a rigid body scene node under an agent or device node."
            } },

    { as_number(SENSOR_ACTION_KIND::END_OF_LIFE), { "END_OF_LIFE",
            "Erase self from the scene."
            } },
};

static std::unordered_map<std::string, SENSOR_ACTION_KIND> const  from_name_to_kind = []() {
    std::unordered_map<std::string, SENSOR_ACTION_KIND>  result;
    for (auto const&  elem : from_index_to_name_and_description)
        result.insert({ elem.second.first, as_sensor_action_kind(elem.first) });
    return result;
}();


std::string const&  description(SENSOR_ACTION_KIND const  kind)
{
    return from_index_to_name_and_description.at(as_number(kind)).second;
}


std::unordered_map<SENSOR_ACTION_KIND, property_map::default_config_records_map> const&  default_sensor_action_configs()
{
    static natural_32_bit  edit_order = 0U;
    static std::unordered_map<SENSOR_ACTION_KIND, property_map::default_config_records_map> const  props {
        { SENSOR_ACTION_KIND::BEGIN_OF_LIFE, {
                { "scene_id", { property_map::make_string("shared/import/TODO"), true,
                        "A unique id of scene to be imported (merged) into the current scene.\n"
                        "Typically, an id is a scene directory (relative to the data root directory).",
                        edit_order++} },
                { "parent_nid", { property_map::make_string("TODO"), true,
                        "A scene node under which the scene will be merged. If the 'parent_nid' node\n"
                        "already contains a node of the same name as the root node of the imported\n"
                        "scene, then the name of the name of the impoted root node is extended by\n"
                        "a suffix making the resulting name unique under the 'parent_nid' node.\n"
                        "     For imported agent the 'parent_nid' must be empty.\n"
                        "     For imported sensor the 'parent_nid' must reference a node under\n"
                        "     nodes tree of an agent or a device.\n"
                        "NOTE: When 'parent_nid' is empty (i.e. not valid), then the root node of the\n"
                        "      impored scene will be put at the root level of the current scene.",
                        edit_order++} },
                { "frame_nid", { property_map::make_string("TODO"), true,
                        "The root node of the imported scene will be put at such location under\n"
                        "'parent_nid' node, so that its world matrix will be the same as the one\n"
                        "of the 'frame_nid' node.",
                        edit_order++} },
                { "linear_velocity_x", { property_map::make_float(0.0f), false,
                        "x coordinate. NOTE: Used only if the root node of the impored scene has a rigid body record.",
                        edit_order++} },
                { "linear_velocity_y", { property_map::make_float(0.0f), false,
                        "y coordinate. NOTE: Used only if the root node of the impored scene has a rigid body record.",
                        edit_order++} },
                { "linear_velocity_z", { property_map::make_float(0.0f), false,
                        "z coordinate. NOTE: Used only if the root node of the impored scene has a rigid body record.",
                        edit_order++} },
                { "angular_velocity_x", { property_map::make_float(0.0f), false,
                        "x coordinate. NOTE: Used only if the root node of the impored scene has a rigid body record.",
                        edit_order++} },
                { "angular_velocity_y", { property_map::make_float(0.0f), false,
                        "y coordinate. NOTE: Used only if the root node of the impored scene has a rigid body record.",
                        edit_order++} },
                { "angular_velocity_z", { property_map::make_float(0.0f), false,
                        "z coordinate. NOTE: Used only if the root node of the impored scene has a rigid body record.",
                        edit_order++} },
                { "velocities_frame_nid", { property_map::make_string(""), false,
                        "When represents a valid scene node, then both velocity vectors above will be\n"
                        "transformed from the space of the referenced frame to the world space before\n"
                        "they are applied to the rigid body. Otherwise, the velocity vectors are used\n"
                        "not-transformed. NOTE: Used only if the root node of the impored scene has a\n"
                        "rigid body record.",
                        edit_order++} },
                { "linear_acceleration_x", { property_map::make_float(0.0f), false,
                        "x coordinate. NOTE: Used only if the root node of the impored scene has a rigid body record.",
                        edit_order++} },
                { "linear_acceleration_y", { property_map::make_float(0.0f), false,
                        "y coordinate. NOTE: Used only if the root node of the impored scene has a rigid body record.",
                        edit_order++} },
                { "linear_acceleration_z", { property_map::make_float(-9.81f), false,
                        "z coordinate. NOTE: Used only if the root node of the impored scene has a rigid body record.",
                        edit_order++} },
                { "angular_acceleration_x", { property_map::make_float(0.0f), false,
                        "x coordinate. NOTE: Used only if the root node of the impored scene has a rigid body record.",
                        edit_order++} },
                { "angular_acceleration_y", { property_map::make_float(0.0f), false,
                        "y coordinate. NOTE: Used only if the root node of the impored scene has a rigid body record.",
                        edit_order++} },
                { "angular_acceleration_z", { property_map::make_float(0.0f), false,
                        "z coordinate. NOTE: Used only if the root node of the impored scene has a rigid body record.",
                        edit_order++} },
                { "accelerations_frame_nid", { property_map::make_string(""), false,
                        "When represents a valid scene node, then both acceleration vectors above will be\n"
                        "transformed from the space of the referenced frame to the world space before\n"
                        "they are applied to the rigid body. Otherwise, the acceleration vectors are used\n"
                        "not-transformed. NOTE: Used only if the root node of the impored scene has a rigid\n"
                        "body record.",
                        edit_order++
                        } },
                } },

        { SENSOR_ACTION_KIND::ENABLE_SENSOR, {
                { "sensor_rid", { property_map::make_string("TODO"), true,
                        "Identifies a scene sensor which will be enabled (i.e. sensing\n"
                        "and sensing corresponding events to its owner).",
                        edit_order++} },
                } },
        { SENSOR_ACTION_KIND::DISABLE_SENSOR, {
                { "sensor_rid", { property_map::make_string("TODO"), true,
                        "Identifies a scene sensor which will be disabled (i.e. not sensing).",
                        edit_order++} },
                } },

        { SENSOR_ACTION_KIND::SET_LINEAR_VELOCITY, {
                { "rigid_body_nid", { property_map::make_string("TODO"), true,
                        "A scene node under an agent or device scene node. The node must contain a rigid\n"
                        "body record whose linear velocity to set.",
                        edit_order++} },
                { "x", { property_map::make_float(0.0f), true,
                        "x coordinate of the velocity vector in world space.",
                        edit_order++} },
                { "y", { property_map::make_float(0.0f), true,
                        "y coordinate of the velocity vector in world space.",
                        edit_order++} },
                { "z", { property_map::make_float(0.0f), true,
                        "z coordinate of the velocity vector in world space.",
                        edit_order++} },
                } },
        { SENSOR_ACTION_KIND::SET_ANGULAR_VELOCITY, {
                { "rigid_body_nid", { property_map::make_string("TODO"), true,
                        "A scene node under an agent or device scene. The node must contain a rigid\n"
                        "body record whose angulat velocity to set.",
                        edit_order++} },
                { "x", { property_map::make_float(0.0f), true,
                        "x coordinate of the velocity vector in world space.",
                        edit_order++} },
                { "y", { property_map::make_float(0.0f), true,
                        "y coordinate of the velocity vector in world space.",
                        edit_order++} },
                { "z", { property_map::make_float(0.0f), true,
                        "z coordinate of the velocity vector in world space.",
                        edit_order++} },
                } },
        { SENSOR_ACTION_KIND::SET_LINEAR_ACCELERATION, {
                { "rigid_body_nid", { property_map::make_string("TODO"), true,
                        "A scene node under an agent or device scene. The node must contain a rigid\n"
                        "body record whose linear acceleration to set.",
                        edit_order++} },
                { "x", { property_map::make_float(0.0f), true,
                        "x coordinate of the acceleration vector in world space.",
                        edit_order++} },
                { "y", { property_map::make_float(0.0f), true,
                        "y coordinate of the acceleration vector in world space.",
                        edit_order++} },
                { "z", { property_map::make_float(0.0f), true,
                        "z coordinate of the acceleration vector in world space.",
                        edit_order++} },
                } },
        { SENSOR_ACTION_KIND::SET_ANGULAR_ACCELERATION, {
                { "rigid_body_nid", { property_map::make_string("TODO"), true,
                        "A scene node under an agent or device scene. The node must contain a rigid\n"
                        "body record whose angular acceleration to set.",
                        edit_order++} },
                { "x", { property_map::make_float(0.0f), true,
                        "x coordinate of the acceleration vector in world space.",
                        edit_order++} },
                { "y", { property_map::make_float(0.0f), true,
                        "y coordinate of the acceleration vector in world space.",
                        edit_order++} },
                { "z", { property_map::make_float(0.0f), true,
                        "z coordinate of the acceleration vector in world space.",
                        edit_order++} },
                } },

        { SENSOR_ACTION_KIND::SET_MASS_INVERTED, {
                { "rigid_body_nid", { property_map::make_string("TODO"), true,
                        "A scene node under an agent or device scene. The node must contain a rigid\n"
                        "body record whose inverted mass to set.",
                        edit_order++} },
                { "inverted_mass", { property_map::make_float(0.0f), true,
                        "An inverted mass to be set to the rigid body. The value 0.0 would make the rigid\n"
                        "body special in the sense that no other rigid body can affect its translation motion.\n"
                        "NOTE: That implies that collision forces of two rigid bodies with zero inverted weights\n"
                        "      can NOT prevent their penetration via translation.",
                        edit_order++} },
                } },
        { SENSOR_ACTION_KIND::SET_INERTIA_TENSOR_INVERTED, {
                { "rigid_body_nid", { property_map::make_string("TODO"), true,
                        "A scene node under an agent or device scene. The node must contain a rigid\n"
                        "body record whose inverted inertia tensor to set. The zero matrix would make the rigid\n"
                        "body special in the sense that no other rigid body can affect its rotation motion.\n"
                        "NOTE: That implies that collision forces of two rigid bodies with zero inverted inertial\n"
                        "      tensors can NOT prevent their penetration via rotation.",
                        edit_order++} },
                { "0_0", { property_map::make_float(0.0f), true,
                        "Element (0,0) of the inverted inertia tensor.",
                        edit_order++} },
                { "0_1", { property_map::make_float(0.0f), true,
                        "Element (0,1) of the inverted inertia tensor.",
                        edit_order++} },
                { "0_2", { property_map::make_float(0.0f), true,
                        "Element (0,2) of the inverted inertia tensor.",
                        edit_order++} },
                { "1_0", { property_map::make_float(0.0f), true,
                        "Element (1,0) of the inverted inertia tensor.",
                        edit_order++} },
                { "1_1", { property_map::make_float(0.0f), true,
                        "Element (1,1) of the inverted inertia tensor.",
                        edit_order++} },
                { "1_2", { property_map::make_float(0.0f), true,
                        "Element (1,2) of the inverted inertia tensor.",
                        edit_order++} },
                { "2_0", { property_map::make_float(0.0f), true,
                        "Element (2,0) of the inverted inertia tensor.",
                        edit_order++} },
                { "2_1", { property_map::make_float(0.0f), true,
                        "Element (2,1) of the inverted inertia tensor.",
                        edit_order++} },
                { "2_2", { property_map::make_float(0.0f), true,
                        "Element (2,2) of the inverted inertia tensor.",
                        edit_order++} },
                } },

        { SENSOR_ACTION_KIND::END_OF_LIFE, {
                } }
    };
    return props;
}


}


std::string const&  as_string(ai::SENSOR_ACTION_KIND const  kind)
{
    return ai::from_index_to_name_and_description.at(as_number(kind)).first;
}


ai::SENSOR_ACTION_KIND  as_sensor_action_kind(std::string const&  name)
{
    return ai::from_name_to_kind.at(name);
}


boost::property_tree::ptree  as_ptree(ai::from_sensor_record_to_sensor_action_map const&  map, ai::scene::node_id const&  root)
{
    boost::property_tree::ptree  result;
    for (auto const&  elem : map)
    {
        boost::property_tree::ptree  actions;
        for (auto const& action : elem.second)
        {
            boost::property_tree::ptree  prop;
            prop.put("kind", as_string(action.kind));
            prop.put_child("props", as_ptree(action.props));
            actions.push_back({"", prop});
        }
        result.put_child(
                ::as_string({
                        elem.first.get_node_id().copy(common_prefix_size(elem.first.get_node_id(), root)),
                        elem.first.get_folder_name(),
                        elem.first.get_record_name()
                        }),
                actions);
    }
    return result;
}


ai::from_sensor_record_to_sensor_action_map  as_sensor_action_map(
        boost::property_tree::ptree const&  tree,
        ai::scene::node_id const&  root
        )
{
    ai::from_sensor_record_to_sensor_action_map  result;
    for (auto  it = tree.begin(); it != tree.end(); ++it)
    {
        ai::scene::record_id const  relative_id = as_scene_record_id(it->first);
        ai::scene::record_id const  id(root / relative_id.get_node_id(), relative_id.get_folder_name(), relative_id.get_record_name());
        std::vector<ai::sensor_action>  actions;
        for (auto  action_it = it->second.begin(); action_it != it->second.end(); ++action_it)
        {
            ai::SENSOR_ACTION_KIND action_kind = as_sensor_action_kind(action_it->second.get<std::string>("kind"));
            ai::property_map const&  loaded_props = as_property_map(action_it->second.get_child("props"));
            ai::property_map  action_props;
            for (auto const&  name_and_record : ai::default_sensor_action_configs().at(action_kind))
            {
                auto const  it = loaded_props.find(name_and_record.first);
                if (it == loaded_props.end())
                {
                    if (name_and_record.second.is_mandatory)
                        action_props.set(name_and_record.first, name_and_record.second.value);
                }
                else
                    action_props.set(*it);
            }
            actions.push_back({ action_kind, action_props });
        }
        result.insert({ id, actions });
    }
    return result;
}
