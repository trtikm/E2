#include <ai/devices.hpp>
#include <angeo/skeleton_kinematics.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <algorithm>

namespace ai {


devices::devices(scene_ptr const  scene_)
    : m_devices()
    , m_scene(scene_)
{
    ASSUMPTION(m_scene != nullptr);
}


device_id  devices::insert(
        scene::node_id const&  device_nid,
        skeletal_motion_templates const  motion_templates,
        DEVICE_KIND const  device_kind
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(device_nid.valid());

    device_id  id = 0U;
    for (; id != m_devices.size(); ++id)
        if (m_devices.at(id) == nullptr)
            break;

    auto const  props = std::make_shared<device_props>();
    props->device_ptr = nullptr;
    props->device_nid = device_nid;
    props->motion_templates = motion_templates;
    props->device_kind = device_kind;

    if (id == m_devices.size())
        m_devices.resize(m_devices.size() + 1U, nullptr);
    m_devices.at(id) = props;

    return id;
}


void  devices::construct_device(device_id const  id, device_props&  props)
{
    TMPROF_BLOCK();

    blackboard_device_ptr const  bb = device::create_blackboard(props.device_kind);
    {
        bb->m_motion_templates = props.motion_templates;
        bb->m_device_id = id;
        bb->m_device_kind = props.device_kind;
        bb->m_scene = m_scene;
        bb->m_device_nid = props.device_nid;
        if (!props.motion_templates.empty())
        {
            bb->m_bone_nids.resize(props.motion_templates.pose_frames().size());
            for (natural_32_bit bone = 0U; bone != bb->m_bone_nids.size(); ++bone)
            {
                scene::node_id::path_type  path;
                for (integer_32_bit parent_bone = (integer_32_bit)bone;
                        parent_bone >= 0;
                        parent_bone = bb->m_motion_templates.hierarchy().parents().at(parent_bone))
                    path.push_back(bb->m_motion_templates.names().at(parent_bone));
                std::reverse(path.begin(), path.end());
                bb->m_bone_nids.at(bone) = props.device_nid / scene::node_id(path);
            }
        }
        device::create_modules(bb);
    }
    props.device_ptr = std::make_unique<device>(bb);
}


void  devices::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    for (natural_32_bit  id = 0U; id != m_devices.size(); ++id)
    {
        auto const  props = m_devices.at(id);
        if (props != nullptr)
            if (props->device_ptr != nullptr)
                props->device_ptr->next_round(time_step_in_seconds);
            else if (props->motion_templates.loaded_successfully())
                construct_device(id, *props);
    }
}


void  devices::on_collision_contact(
        device_id const  id,
        scene::node_id const&  collider_nid,
        scene::collicion_contant_info const&  contact_info,
        object_id const&  other_id,
        scene::node_id const&  other_collider_nid
        )
{}


}
