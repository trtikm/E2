#include <ai/agents.hpp>
#include <ai/blackboard_human.hpp>
#include <angeo/skeleton_kinematics.hpp>
#include <ai/cortex_mock_human.hpp>
#include <ai/cortex_input_encoder_human.hpp>
#include <ai/cortex_output_decoder_human.hpp>
#include <ai/sensory_controller.hpp>
#include <ai/sensory_controller_ray_cast_sight.hpp>
#include <ai/action_controller_human.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <algorithm>

namespace ai {


agents::agents(scene_ptr const  scene_)
    : m_agents()
    , m_scene(scene_)
    , m_input_devices(std::make_shared<input_devices>())
    , m_scene_update_events()
{
    ASSUMPTION(m_scene != nullptr);
}


agent_id  agents::insert(
        scene::node_id const&  agent_nid,
        skeletal_motion_templates const  motion_templates,
        retina_ptr const  retina_or_null
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(agent_nid.valid() && !motion_templates.empty());

    agent_id  id = 0U;
    for (; id != m_agents.size(); ++id)
        if (m_agents.at(id) == nullptr)
            break;

    auto const  props = std::make_shared<agent_props>();
    props->agent_ptr = nullptr;
    props->agent_nid = agent_nid;
    props->motion_templates = motion_templates;
    props->retina_ptr = retina_or_null;

    if (id == m_agents.size())
        m_agents.resize(m_agents.size() + 1U, nullptr);
    m_agents.at(id) = props;

    return id;
}


void  agents::construct_agent(agent_id const  id, agent_props&  props)
{
    TMPROF_BLOCK();

    blackboard_ptr const  bb = std::make_shared<blackboard_human>();
    {
        bb->m_motion_templates = props.motion_templates;
        bb->m_agent_id = id;
        bb->m_retina_ptr = props.retina_ptr;
        bb->m_scene = m_scene;
        bb->m_agent_nid = props.agent_nid;
        bb->m_bone_nids.resize(props.motion_templates.pose_frames().size());
        for (natural_32_bit bone = 0U; bone != bb->m_bone_nids.size(); ++bone)
        {
            scene::node_id::path_type  path;
            for (integer_32_bit parent_bone = (integer_32_bit)bone;
                    parent_bone >= 0;
                    parent_bone = bb->m_motion_templates.hierarchy().parents().at(parent_bone))
                path.push_back(bb->m_motion_templates.names().at(parent_bone));
            std::reverse(path.begin(), path.end());
            bb->m_bone_nids.at(bone) = props.agent_nid / scene::node_id(path);
        }
        cortex_io_ptr const  io = std::make_shared<cortex_io>();
        io->input.resize(2U);
        io->num_inner_inputs = 1U;
        io->output.resize(3U);
        bb->m_cortex_primary = std::make_shared<cortex_mock_human>(io, m_input_devices);
        bb->m_cortex_secondary = std::make_shared<cortex>(io); // Not used so far.
        bb->m_cortex_input_encoder = std::make_shared<cortex_input_encoder_human>(io, bb);
        bb->m_cortex_output_decoder = std::make_shared<cortex_output_decoder_human>(io, bb);
        bb->m_sensory_controller = std::make_shared<sensory_controller>(bb, std::make_shared<sensory_controller_ray_cast_sight>(bb,
                sensory_controller_sight::camera_config(),
                sensory_controller_ray_cast_sight::ray_cast_config()
                ));
        bb->m_action_controller = std::make_shared<action_controller_human>(bb);
    }
    props.agent_ptr = std::make_unique<agent>(bb);
}


void  agents::next_round(
        float_32_bit const  time_step_in_seconds,
        input_devices::keyboard_props const&  keyboard,
        input_devices::mouse_props const&  mouse,
        input_devices::window_props const&  window
        )
{
    TMPROF_BLOCK();

    for (auto const&  event_handler_ptr : m_scene_update_events)
        (*event_handler_ptr)();
    m_scene_update_events.clear();

    m_input_devices->keyboard = keyboard;
    m_input_devices->mouse = mouse;
    m_input_devices->window = window;

    for (natural_32_bit  id = 0U; id != m_agents.size(); ++id)
    {
        auto const  props = m_agents.at(id);
        if (props != nullptr)
            if (props->agent_ptr != nullptr)
            {
                props->agent_ptr->next_round(time_step_in_seconds);
                props->agent_ptr->get_blackboard()->m_collision_contacts.clear();
            }
            else if (props->motion_templates.loaded_successfully())
                construct_agent(id, *props);
    }
}


void  agents::on_collision_contact(
        agent_id const  agent_id,
        scene::node_id const&  collider_nid,
        scene::collicion_contant_info const&  contact_info
        )
{
    m_agents.at(agent_id)->agent_ptr->get_blackboard()->m_collision_contacts.insert({collider_nid, contact_info});
}


}
