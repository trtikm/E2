#ifndef AI_BLACKBOARD_HPP_INCLUDED
#   define AI_BLACKBOARD_HPP_INCLUDED

#   include <ai/skeletal_motion_templates.hpp>
#   include <ai/scene.hpp>
#   include <vector>
#   include <memory>

namespace ai {


/// A storage of data shared by all modules of an agent/device.
/// NOTE: This is NOT an implementation of the 'blackboard' AI approach.
///       Only the idea of a shared storage is used here.
struct blackboard
{
    virtual ~blackboard();

    skeletal_motion_templates  m_motion_templates;
    scene_ptr  m_scene;
    std::vector<scene::node_id>  m_bone_nids;
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
