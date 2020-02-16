#include <ai/agent.hpp>
#include <ai/cortex.hpp>
#include <ai/cortex_mock.hpp>
#include <ai/cortex_random.hpp>
#include <ai/cortex_robot.hpp>
#include <ai/sensory_controller.hpp>
#include <ai/sensory_controller_collision_contacts.hpp>
#include <ai/sensory_controller_ray_cast_sight.hpp>
#include <ai/action_controller.hpp>
#include <ai/simulator.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


blackboard_agent_ptr  agent::create_blackboard(AGENT_KIND const  agent_kind)
{
    TMPROF_BLOCK();

    switch (agent_kind)
    {
    case AGENT_KIND::MOCK:
    case AGENT_KIND::STATIONARY:
    case AGENT_KIND::RANDOM:
    case AGENT_KIND::ROBOT:
        return std::make_shared<blackboard_agent>();
    default:
        UNREACHABLE();
    }
}


void  agent::create_modules(blackboard_agent_ptr const  bb, input_devices_ptr const  idev)
{
    TMPROF_BLOCK();

    switch (bb->m_agent_kind)
    {
    case AGENT_KIND::MOCK:
        bb->m_cortex = std::make_shared<cortex_mock>(bb, idev);
        bb->m_sensory_controller = std::make_shared<sensory_controller>(
                bb,
                std::make_shared<sensory_controller_collision_contacts>(
                        bb,
                        sensory_controller_collision_contacts::config()
                        ),
                nullptr
                );
        bb->m_action_controller = std::make_shared<action_controller>(bb);
        break;
    case AGENT_KIND::STATIONARY:
        bb->m_cortex = std::make_shared<cortex>(bb);
        bb->m_sensory_controller = std::make_shared<sensory_controller>(
                bb,
                std::make_shared<sensory_controller_collision_contacts>(
                        bb,
                        sensory_controller_collision_contacts::config()
                        ),
                nullptr
                );
        bb->m_action_controller = std::make_shared<action_controller>(bb);
        break;
    case AGENT_KIND::RANDOM:
        bb->m_cortex = std::make_shared<cortex_random>(bb);
        bb->m_sensory_controller = std::make_shared<sensory_controller>(
                bb,
                std::make_shared<sensory_controller_collision_contacts>(
                        bb,
                        sensory_controller_collision_contacts::config()
                        ),
                std::make_shared<sensory_controller_sight>(
                        bb,
                        sensory_controller_sight::camera_config()
                        )
                );
        bb->m_action_controller = std::make_shared<action_controller>(bb);
        break;
    case AGENT_KIND::ROBOT:
        bb->m_cortex = std::make_shared<cortex_robot>(bb);
        bb->m_sensory_controller = std::make_shared<sensory_controller>(
                bb,
                std::make_shared<sensory_controller_collision_contacts>(
                        bb,
                        sensory_controller_collision_contacts::config()
                        ),
                std::make_shared<sensory_controller_ray_cast_sight>(
                        bb,
                        sensory_controller_sight::camera_config(),
                        sensory_controller_ray_cast_sight::ray_cast_config()
                        )
                );
        bb->m_action_controller = std::make_shared<action_controller>(bb);
        break;
    default:
        UNREACHABLE();
    }
}


}

namespace ai {


agent::agent(blackboard_agent_ptr const  blackboard_)
    : m_blackboard(blackboard_)
{
    TMPROF_BLOCK();

    // The initialisation order of agent's modules is important and mandatory.
    get_blackboard()->m_sensory_controller->initialise();
    get_blackboard()->m_action_controller->initialise();
    get_blackboard()->m_cortex->initialise();
}


agent::~agent()
{
    TMPROF_BLOCK();

    // The release order of agent's modules is important and mandatory.
    get_blackboard()->m_cortex = nullptr;
    get_blackboard()->m_action_controller = nullptr;
    get_blackboard()->m_sensory_controller = nullptr;
}


void  agent::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    // We need to synchronise motion object (located in the action controller) with the scene
    // as it can be used by any controllers in their next_round functions.
    get_blackboard()->m_action_controller->synchronise_motion_object_motion_with_scene();

    // The update order of agent's modules is important and mandatory.
    get_blackboard()->m_sensory_controller->next_round(time_step_in_seconds);
    get_blackboard()->m_cortex->next_round(time_step_in_seconds);
    get_blackboard()->m_action_controller->next_round(time_step_in_seconds);
}


void  agent::on_sensor_event(sensor const&  s, sensor const* const  other)
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

        case SENSOR_ACTION_KIND::UPDATE_RADIAL_FORCE_FIELD:
            ASSUMPTION(other != nullptr);
            get_blackboard()->m_scene->accept(scene::create_request<scene::request_update_radial_force_field>(
                    other->get_self_rid(),
                    get_blackboard()->m_self_rid,
                    action.props
                    ));
            break;
        case SENSOR_ACTION_KIND::UPDATE_LINEAR_FORCE_FIELD:
            ASSUMPTION(other != nullptr);
            get_blackboard()->m_scene->accept(scene::create_request<scene::request_update_linear_force_field>(
                    other->get_self_rid(),
                    get_blackboard()->m_self_rid,
                    action.props
                    ));
            break;
        case SENSOR_ACTION_KIND::LEAVE_FORCE_FIELD:
            ASSUMPTION(other != nullptr);
            get_blackboard()->m_scene->accept(scene::create_request<scene::request_leave_force_field>(
                    other->get_self_rid(),
                    get_blackboard()->m_self_rid
                    ));
            break;

        case SENSOR_ACTION_KIND::END_OF_LIFE:
            get_blackboard()->m_scene->accept(scene::create_request<scene::request_erase_nodes_tree>(
                    get_blackboard()->m_self_rid.get_node_id())
                    );
            break;
        default: UNREACHABLE(); break;
        }
}


}
