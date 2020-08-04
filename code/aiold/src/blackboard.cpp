#include <aiold/blackboard.hpp>
#include <cassert>

namespace aiold {


blackboard::blackboard()
    : m_motion_templates()
    , m_scene()
    , m_self_rid()
    , m_bone_nids()
    , m_state(0U)
    , m_sensor_actions()
    , m_simulator_ptr(nullptr)
{}


blackboard::~blackboard()
{
}


void  blackboard::initialise_bone_nids()
{
    if (m_motion_templates.empty())
    {
        m_bone_nids.clear();
        return;
    }
    m_bone_nids.resize(m_motion_templates.pose_frames().size());
    for (natural_32_bit bone = 0U; bone != m_bone_nids.size(); ++bone)
    {
        scene::node_id::path_type  path;
        for (integer_32_bit parent_bone = (integer_32_bit)bone;
                parent_bone >= 0;
                parent_bone = m_motion_templates.parents().at(parent_bone))
            path.push_back(m_motion_templates.names().at(parent_bone));
        std::reverse(path.begin(), path.end());
        m_bone_nids.at(bone) = m_self_rid.get_node_id() / scene::node_id(path);
    }
}


}
