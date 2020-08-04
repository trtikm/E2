#include <aiold/sensor_action_default_processor.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace aiold { namespace detail {


bool  has_contact_with_force_field_sensitive_object(std::vector<scene::collicion_contant_info_ptr>  contact_infos)
{
    for (scene::collicion_contant_info_ptr  info : contact_infos)
        switch (info->other_collision_class)
        {
        case angeo::COLLISION_CLASS::COMMON_MOVEABLE_OBJECT:
        case angeo::COLLISION_CLASS::AGENT_MOTION_OBJECT:
            return true;
        default:
            break;
        }
    return false;
}


}}

namespace aiold {


bool  process_sensor_event_using_default_procedure(
        scene::record_id const&  self_rid,
        sensor_action&  action,
        sensor::other_object_info const&  other,
        simulator* const  simulator,
        scene_ptr const  scene
        )
{
    ASSUMPTION(self_rid.valid() && simulator != nullptr && scene != nullptr);
    switch (action.kind)
    {
    case SENSOR_ACTION_KIND::BEGIN_OF_LIFE:
        scene->accept(scene::create_request<scene::request_merge_scene>(action.props), true);
        return true;

    case SENSOR_ACTION_KIND::ENABLE_SENSOR:
        simulator->set_sensor_enabled(action.props.get_scene_record_id("sensor_rid", self_rid.get_node_id()), true);
        return true;
    case SENSOR_ACTION_KIND::DISABLE_SENSOR:
        simulator->set_sensor_enabled(action.props.get_scene_record_id("sensor_rid", self_rid.get_node_id()), false);
        return true;

    case SENSOR_ACTION_KIND::SET_LINEAR_VELOCITY:
        scene->set_linear_velocity_of_rigid_body_of_scene_node(
            action.props.get_scene_node_id("rigid_body_nid", self_rid.get_node_id()),
            action.props.get_vector3()
        );
        return true;
    case SENSOR_ACTION_KIND::SET_ANGULAR_VELOCITY:
        scene->set_angular_velocity_of_rigid_body_of_scene_node(
            action.props.get_scene_node_id("rigid_body_nid", self_rid.get_node_id()),
            action.props.get_vector3()
        );
        return true;
    case SENSOR_ACTION_KIND::SET_LINEAR_ACCELERATION:
        scene->set_linear_acceleration_of_rigid_body_of_scene_node(
            action.props.get_scene_node_id("rigid_body_nid", self_rid.get_node_id()),
            action.props.get_vector3()
        );
        return true;
    case SENSOR_ACTION_KIND::SET_ANGULAR_ACCELERATION:
        scene->set_angular_acceleration_of_rigid_body_of_scene_node(
            action.props.get_scene_node_id("rigid_body_nid", self_rid.get_node_id()),
            action.props.get_vector3()
        );
        return true;

    case SENSOR_ACTION_KIND::SET_MASS_INVERTED:
        scene->set_inverted_mass_of_rigid_body_of_scene_node(
            action.props.get_scene_node_id("rigid_body_nid", self_rid.get_node_id()),
            action.props.get_float("inverted_mass")
        );
        return true;
    case SENSOR_ACTION_KIND::SET_INERTIA_TENSOR_INVERTED:
        scene->set_inverted_inertia_tensor_of_rigid_body_of_scene_node(
            action.props.get_scene_node_id("rigid_body_nid", self_rid.get_node_id()),
            action.props.get_matrix33()
        );
        return true;

    case SENSOR_ACTION_KIND::UPDATE_RADIAL_FORCE_FIELD:
        if (other.sensor == nullptr && detail::has_contact_with_force_field_sensitive_object(other.contact_infos))
            scene->accept(scene::create_request<scene::request_update_radial_force_field>(other.rigid_body_id, self_rid, action.props), false);
        return true;
    case SENSOR_ACTION_KIND::UPDATE_LINEAR_FORCE_FIELD:
        if (other.sensor == nullptr && detail::has_contact_with_force_field_sensitive_object(other.contact_infos))
            scene->accept(scene::create_request<scene::request_update_linear_force_field>(other.rigid_body_id, self_rid, action.props), false);
        return true;
    case SENSOR_ACTION_KIND::LEAVE_FORCE_FIELD:
        if (other.sensor == nullptr && detail::has_contact_with_force_field_sensitive_object(other.contact_infos))
            scene->accept(scene::create_request<scene::request_leave_force_field>(other.rigid_body_id, self_rid), false);
        return true;

    case SENSOR_ACTION_KIND::END_OF_LIFE:
        scene->accept(scene::create_request<scene::request_erase_nodes_tree>(self_rid.get_node_id()), true);
        return true;

    default: return false;
    }
}


}
