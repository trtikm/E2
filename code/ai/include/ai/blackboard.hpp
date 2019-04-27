#ifndef AI_BLACKBOARD_HPP_INCLUDED
#   define AI_BLACKBOARD_HPP_INCLUDED

#   include <ai/agent_id.hpp>
#   include <ai/scene.hpp>
#   include <ai/skeleton_composition.hpp>
#   include <ai/skeletal_motion_templates.hpp>
#   include <memory>

namespace ai {


/// A storage of data shared by all modules of an agent.
/// NOTE: This is NOT an implementation of the 'blackboard' AI approach.
///       Only the idea of a shared storage is used here.
struct blackboard
{
    virtual ~blackboard() {}

    agent_id  m_agent_id;

    scene_ptr  m_scene;
    scene::node_id  m_agent_nid;

    std::vector<angeo::coordinate_system>  m_frames;    // For each bone a coord. system in the local space of the parent bone if any,
                                                        // otherwise in the world space. 'm_frames.at(0)' is ALWAYS in the world space,
                                                        // because 'INVARIANT(m_skeleton_composition->parents.at(0) == -1)'.
    skeleton_composition_const_ptr  m_skeleton_composition;
    skeletal_motion_templates_const_ptr  m_motion_templates;
};


using  blackboard_ptr = std::shared_ptr<blackboard>;
using  blackboard_const_ptr = std::shared_ptr<blackboard const>;


template<typename T>
inline std::shared_ptr<T>  as(blackboard_ptr const  ptr) { return std::dynamic_pointer_cast<T>(ptr); }

template<typename T>
inline std::shared_ptr<T const>  as(blackboard_const_ptr const  ptr) { return std::dynamic_pointer_cast<T const>(ptr); }


}

#endif
