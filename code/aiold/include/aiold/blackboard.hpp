#ifndef AIOLD_BLACKBOARD_HPP_INCLUDED
#   define AIOLD_BLACKBOARD_HPP_INCLUDED

#   include <aiold/skeletal_motion_templates.hpp>
#   include <aiold/object_id.hpp>
#   include <aiold/scene.hpp>
#   include <aiold/sensor_action.hpp>
#   include <vector>
#   include <memory>

namespace aiold {


struct  simulator;


/// A storage of data shared by all modules of an agent/device.
/// NOTE: This is NOT an implementation of the 'blackboard' AI approach.
///       Only the idea of a shared storage is used here.
struct  blackboard
{
    blackboard();
    virtual  ~blackboard();

    void  initialise_bone_nids();

    object_id  m_self_id;
    skeletal_motion_templates  m_motion_templates;
    scene_ptr  m_scene;
    scene::record_id  m_self_rid;
    std::vector<scene::node_id>  m_bone_nids;
    natural_32_bit  m_state;
    std::shared_ptr<from_sensor_record_to_sensor_action_map>  m_sensor_actions;
    simulator*  m_simulator_ptr;
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