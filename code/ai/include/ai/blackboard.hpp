#ifndef AI_BLACKBOARD_HPP_INCLUDED
#   define AI_BLACKBOARD_HPP_INCLUDED

#   include <ai/agent_id.hpp>
#   include <ai/scene.hpp>
#   include <ai/skeletal_motion_templates.hpp>
#   include <unordered_map>
#   include <memory>

namespace ai {


struct  cortex;
struct  cortex_input_encoder;
struct  cortex_output_decoder;
struct  sensory_controller;
struct  action_controller;
struct  retina;


/// A storage of data shared by all modules of an agent.
/// NOTE: This is NOT an implementation of the 'blackboard' AI approach.
///       Only the idea of a shared storage is used here.
struct blackboard
{
    using  collision_contacts_map = std::unordered_multimap<scene::node_id, scene::collicion_contant_info>;

    virtual ~blackboard();

    skeletal_motion_templates  m_motion_templates;

    agent_id  m_agent_id;
    scene_ptr  m_scene;
    scene::node_id  m_agent_nid;
    std::vector<scene::node_id>  m_bone_nids;
    collision_contacts_map  m_collision_contacts;

    std::shared_ptr<retina>  m_retina_ptr; // This should be moved somewhere else, e.g. to sensory_controller_sight_retina?

    // Modules defining the agent:

    std::shared_ptr<cortex>  m_cortex_primary;
    std::shared_ptr<cortex>  m_cortex_secondary;
    std::shared_ptr<cortex_input_encoder>  m_cortex_input_encoder;
    std::shared_ptr<cortex_output_decoder>  m_cortex_output_decoder;
    std::shared_ptr<sensory_controller>  m_sensory_controller;
    std::shared_ptr<action_controller>  m_action_controller;

    virtual void  release_modules(); // Whenever you add a new module, include its release to this function too.
};


using  blackboard_ptr = std::shared_ptr<blackboard>;
using  blackboard_const_ptr = std::shared_ptr<blackboard const>;

using  blackboard_weak_ptr = std::weak_ptr<blackboard>;

template<typename T>
inline std::shared_ptr<T>  as(blackboard_ptr const  ptr) { return std::dynamic_pointer_cast<T>(ptr); }

template<typename T>
inline std::shared_ptr<T const>  as(blackboard_const_ptr const  ptr) { return std::dynamic_pointer_cast<T const>(ptr); }


}

#endif
