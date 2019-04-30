#ifndef AI_AGENTS_HPP_INCLUDED
#   define AI_AGENTS_HPP_INCLUDED

#   include <ai/agent.hpp>
#   include <ai/agent_id.hpp>
#   include <ai/input_devices.hpp>
#   include <ai/scene.hpp>
#   include <ai/skeleton_composition.hpp>
#   include <ai/skeletal_motion_templates.hpp>
#   include <ai/action_controller.hpp>
#   include <memory>
#   include <vector>
#   include <functional>

namespace ai {


struct agents
{
    explicit agents(scene_ptr const  scene_);

    agent_id  insert(
            scene::node_id const&  agent_nid,
            skeletal_motion_templates::motion_template_cursor const&  start_pose,   // Defines the start pose (animation name and keyframe
                                                                                    // index) in which the agent should start the simulation. 
            skeleton_composition_const_ptr const  skeleton, // INVARIANT(skeleton->parents.at(0) == -1)
                                                            // INVARIANT(skeleton->pose_frames.size() == current_frames.size())
            skeletal_motion_templates_const_ptr const  motion_templates
            );
    void  erase(agent_id const  id) { m_agents.at(id) = nullptr; }

    void  clear() { m_agents.clear(); }

    agent&  at(agent_id const  id) { return *m_agents.at(id); }
    agent const&  at(agent_id const  id) const { return *m_agents.at(id); }

    scene_ptr  get_scene_ptr() const { return m_scene; }

    void  next_round(
            float_32_bit const  time_step_in_seconds,
            input_devices::keyboard_props const&  keyboard,
            input_devices::mouse_props const&  mouse,
            input_devices::window_props const&  window
            );

    void  on_collision_contact(
            agent_id const  agent_id,
            scene::node_id const&  collider_nid,
            vector3 const&  contact_point,
            vector3 const&  unit_normal
            );

private:
    std::vector<std::shared_ptr<agent> >  m_agents; // Should be 'std::vector<std::unique_ptr<agent> >', but does not compile :-(
    scene_ptr  m_scene;
    input_devices_ptr  m_input_devices;
    std::vector<std::unique_ptr<std::function<void()> > >  m_scene_update_events;
};


}

#endif
