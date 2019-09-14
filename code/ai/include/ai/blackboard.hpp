#ifndef AI_BLACKBOARD_HPP_INCLUDED
#   define AI_BLACKBOARD_HPP_INCLUDED

#   include <ai/agent_id.hpp>
#   include <ai/scene.hpp>
#   include <ai/retina.hpp>
#   include <ai/skeletal_motion_templates.hpp>
#   include <unordered_map>
#   include <memory>

namespace ai {


/// A storage of data shared by all modules of an agent.
/// NOTE: This is NOT an implementation of the 'blackboard' AI approach.
///       Only the idea of a shared storage is used here.
struct blackboard
{
    virtual ~blackboard() {}

    using  collision_contacts_map = std::unordered_multimap<scene::node_id, scene::collicion_contant_info>;

    skeletal_motion_templates  m_motion_templates;

    agent_id  m_agent_id;

    retina_ptr  m_retina_ptr;

    scene_ptr  m_scene;
    scene::node_id  m_agent_nid;
    std::vector<scene::node_id>  m_bone_nids;
    collision_contacts_map  m_collision_contacts;
};


using  blackboard_ptr = std::shared_ptr<blackboard>;
using  blackboard_const_ptr = std::shared_ptr<blackboard const>;


template<typename T>
inline std::shared_ptr<T>  as(blackboard_ptr const  ptr) { return std::dynamic_pointer_cast<T>(ptr); }

template<typename T>
inline std::shared_ptr<T const>  as(blackboard_const_ptr const  ptr) { return std::dynamic_pointer_cast<T const>(ptr); }


}

#endif
