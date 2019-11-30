#ifndef AI_BLACKBOARD_HPP_INCLUDED
#   define AI_BLACKBOARD_HPP_INCLUDED

#   include <ai/agent_id.hpp>
#   include <ai/agent_kind.hpp>
#   include <ai/scene.hpp>
#   include <ai/skeletal_motion_templates.hpp>
#   include <memory>

namespace ai {


struct  cortex;
struct  sensory_controller;
struct  action_controller;
struct  retina;


/// A storage of data shared by all modules of an agent.
/// NOTE: This is NOT an implementation of the 'blackboard' AI approach.
///       Only the idea of a shared storage is used here.
struct blackboard
{
    virtual ~blackboard();
    bool are_all_modules_released() const;

    skeletal_motion_templates  m_motion_templates;

    agent_id  m_agent_id;
    AGENT_KIND  m_agent_kind;
    scene_ptr  m_scene;
    scene::node_id  m_agent_nid;
    std::vector<scene::node_id>  m_bone_nids;

    std::shared_ptr<retina>  m_retina_ptr; // This should be moved somewhere else, e.g. to sensory_controller_sight_retina?

    // Modules defining the agent:

    std::shared_ptr<cortex>  m_cortex;
    std::shared_ptr<sensory_controller>  m_sensory_controller;
    std::shared_ptr<action_controller>  m_action_controller;
};


using  blackboard_ptr = std::shared_ptr<blackboard>;
using  blackboard_const_ptr = std::shared_ptr<blackboard const>;

using  blackboard_weak_ptr = std::weak_ptr<blackboard>;
using  blackboard_weak_const_ptr = std::weak_ptr<blackboard const>;

template<typename T, typename Q>
inline std::shared_ptr<T>  as(std::shared_ptr<Q>  ptr) { return std::dynamic_pointer_cast<T>(ptr); }

template<typename T, typename Q>
inline std::shared_ptr<T const>  as(std::shared_ptr<Q const>  ptr) { return std::dynamic_pointer_cast<T const>(ptr); }


}

#endif
