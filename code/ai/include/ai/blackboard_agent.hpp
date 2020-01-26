#ifndef AI_BLACKBOARD_AGENT_HPP_INCLUDED
#   define AI_BLACKBOARD_AGENT_HPP_INCLUDED

#   include <ai/blackboard.hpp>
#   include <ai/agent_id.hpp>
#   include <ai/agent_kind.hpp>
#   include <memory>

namespace ai {


struct  cortex;
struct  sensory_controller;
struct  action_controller;
struct  retina;


/// A storage of data shared by all modules of an agent.
/// NOTE: This is NOT an implementation of the 'blackboard' AI approach.
///       Only the idea of a shared storage is used here.
struct blackboard_agent : public blackboard
{
    virtual ~blackboard_agent();
    bool are_all_modules_released() const;

    agent_id  m_agent_id;
    AGENT_KIND  m_agent_kind;
    scene::node_id  m_agent_nid;

    std::shared_ptr<retina>  m_retina_ptr; // This should be moved somewhere else, e.g. to sensory_controller_sight_retina?

    // Modules defining the agent:

    std::shared_ptr<cortex>  m_cortex;
    std::shared_ptr<sensory_controller>  m_sensory_controller;
    std::shared_ptr<action_controller>  m_action_controller;
};


using  blackboard_agent_ptr = std::shared_ptr<blackboard_agent>;
using  blackboard_agent_const_ptr = std::shared_ptr<blackboard_agent const>;

using  blackboard_agent_weak_ptr = std::weak_ptr<blackboard_agent>;
using  blackboard_agent_weak_const_ptr = std::weak_ptr<blackboard_agent const>;


}

#endif
