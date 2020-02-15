#include <ai/device.hpp>
#include <ai/simulator.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


blackboard_device_ptr  device::create_blackboard(DEVICE_KIND const  device_kind)
{
    TMPROF_BLOCK();

    switch (device_kind)
    {
    case DEVICE_KIND::DEFAULT:
        return std::make_shared<blackboard_device>();
    default:
        UNREACHABLE();
    }
}


void  device::create_modules(blackboard_device_ptr const  bb)
{
    TMPROF_BLOCK();

    switch (bb->m_device_kind)
    {
    case DEVICE_KIND::DEFAULT:
        break;
    default:
        UNREACHABLE();
    }
}


}

namespace ai {


device::device(blackboard_device_ptr const  blackboard_)
    : m_blackboard(blackboard_)
{
    TMPROF_BLOCK();

    // The initialisation order of device's modules is important and mandatory.
}


device::~device()
{
    TMPROF_BLOCK();

    // The release order of device's modules is important and mandatory.
}


void  device::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    // The update order of device's modules is important and mandatory.
}


void  device::on_sensor_event(sensor const&  s)
{
    auto const  actions_it = get_blackboard()->m_sensor_actions->find(s.get_self_rid());
    ASSUMPTION(actions_it != get_blackboard()->m_sensor_actions->end());
    for (sensor_action&  action : actions_it->second)
        switch (action.kind)
        {
        case SENSOR_ACTION_KIND::BEGIN_OF_LIFE:
            get_blackboard()->m_scene->accept(scene::create_request<scene::request_merge_scene>(action.props));
            break;

        case SENSOR_ACTION_KIND::ENABLE_SENSOR:
            get_blackboard()->m_simulator_ptr->set_sensor_enabled(action.props.get_scene_record_id("sensor_rid"), true);
            break;
        case SENSOR_ACTION_KIND::DISABLE_SENSOR:
            get_blackboard()->m_simulator_ptr->set_sensor_enabled(action.props.get_scene_record_id("sensor_rid"), false);
            break;

        case SENSOR_ACTION_KIND::SET_LINEAR_VELOCITY:
            get_blackboard()->m_scene->set_linear_velocity_of_rigid_body_of_scene_node(
                    action.props.get_scene_node_id("rigid_body_nid"),
                    action.props.get_vector3()
                    );
            break;
        case SENSOR_ACTION_KIND::SET_ANGULAR_VELOCITY:
            get_blackboard()->m_scene->set_angular_velocity_of_rigid_body_of_scene_node(
                    action.props.get_scene_node_id("rigid_body_nid"),
                    action.props.get_vector3()
                    );
            break;
        case SENSOR_ACTION_KIND::SET_LINEAR_ACCELERATION:
            get_blackboard()->m_scene->set_linear_acceleration_of_rigid_body_of_scene_node(
                    action.props.get_scene_node_id("rigid_body_nid"),
                    action.props.get_vector3()
                    );
            break;
        case SENSOR_ACTION_KIND::SET_ANGULAR_ACCELERATION:
            get_blackboard()->m_scene->set_angular_acceleration_of_rigid_body_of_scene_node(
                    action.props.get_scene_node_id("rigid_body_nid"),
                    action.props.get_vector3()
                    );
            break;

        case SENSOR_ACTION_KIND::SET_MASS_INVERTED:
            get_blackboard()->m_scene->set_inverted_mass_of_rigid_body_of_scene_node(
                    action.props.get_scene_node_id("rigid_body_nid"),
                    action.props.get_float("inverted_mass")
                    );
            break;
        case SENSOR_ACTION_KIND::SET_INERTIA_TENSOR_INVERTED:
            get_blackboard()->m_scene->set_inverted_inertia_tensor_of_rigid_body_of_scene_node(
                    action.props.get_scene_node_id("rigid_body_nid"),
                    action.props.get_matrix33()
                    );
            break;

        case SENSOR_ACTION_KIND::END_OF_LIFE:
            get_blackboard()->m_scene->accept(scene::create_request<scene::request_erase_nodes_tree>(get_blackboard()->m_self_nid));
            break;
        default: UNREACHABLE(); break;
        }
}


}
