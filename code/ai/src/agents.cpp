#include <ai/agents.hpp>
#include <angeo/skeleton_kinematics.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


agents::agents(
        environment_models::collision_scene_ptr const  collisions,
        environment_models::rigid_body_simulator_ptr const  physics,
        environment_models::create_scene_object_callback const&  create_scene_object_handler,
        environment_models::destroy_scene_object_callback const&  destroy_scene_object_handler
        )
    : m_agents()
    , m_environment_models()
    , m_input_devices(std::make_shared<input_devices>())
    , m_scene_update_events()
    , m_create_scene_object_handler(create_scene_object_handler)
    , m_destroy_scene_object_handler(destroy_scene_object_handler)
{
    m_environment_models.swap(std::make_shared<environment_models>(
            collisions,
            physics,
            [this](agent_id const  id, environment_models::scene_action_name const&  name) -> void {
                    this->m_scene_update_events.push_back(std::make_unique<std::function<void()> >(
                        std::bind(this->m_create_scene_object_handler, id, name))
                        );
                     },
            [this](angeo::rigid_body_id const  id) -> void {
                    this->m_scene_update_events.push_back(std::make_unique<std::function<void()> >(
                        std::bind(this->m_destroy_scene_object_handler, id))
                        );
                    }
            ));
}


agent_id  agents::insert(
        std::vector<angeo::coordinate_system> const&  current_frames,
        skeleton_composition_const_ptr const  skeleton,
        skeletal_motion_templates_const_ptr const  motion_templates
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(skeleton != nullptr);
    ASSUMPTION(
        !skeleton->pose_frames.empty() &&
        skeleton->pose_frames.size() == current_frames.size() &&
        skeleton->pose_frames.size() == skeleton->names.size() &&
        skeleton->pose_frames.size() == skeleton->parents.size() &&
        (skeleton->children.empty() || skeleton->pose_frames.size() == skeleton->children.size())
        );
    ASSUMPTION(skeleton->parents.at(0U) == -1);
    ASSUMPTION(motion_templates != nullptr && motion_templates->is_ready());

    if (skeleton->children.empty())
        angeo::skeleton_compute_child_bones(skeleton->parents, std::const_pointer_cast<skeleton_composition>(skeleton)->children);

    blackboard_ptr const  bb = std::make_shared<blackboard>();
    bb->m_frames = current_frames;
    bb->m_environment_models = m_environment_models;
    bb->m_skeleton_composition = skeleton;
    bb->m_motion_templates = motion_templates;

    auto  agent_ptr = std::make_shared<agent>(bb, m_input_devices);

    agent_id  id = 0U;
    for (; id != m_agents.size(); ++id)
        if (m_agents.at(id) == nullptr)
            break;
    if (id == m_agents.size())
        m_agents.resize(m_agents.size() + 1U, nullptr);

    m_agents.at(id) = agent_ptr;
    
    return id;    
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

    for (auto&  agent_ptr : m_agents)
        if (agent_ptr != nullptr)
            agent_ptr->next_round(time_step_in_seconds);
}


}
