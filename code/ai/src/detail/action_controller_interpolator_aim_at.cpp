#include <ai/detail/action_controller_interpolator_aim_at.hpp>
#include <ai/skeleton_utils.hpp>
#include <angeo/skeleton_kinematics.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>

namespace ai { namespace detail {


action_controller_interpolator_aim_at::action_controller_interpolator_aim_at(
        action_controller_interpolator const* const  interpolator_,
        skeletal_motion_templates::motion_template_cursor const&  initial_template_cursor
        )
    : action_controller_interpolator_shared(interpolator_)

    , m_src_bones()
    , m_current_bones()
    , m_dst_bones()
{
    set_target(initial_template_cursor);
    m_src_bones = m_current_bones = m_dst_bones;
}


void  action_controller_interpolator_aim_at::interpolate(
        float_32_bit const  time_step_in_seconds,
        float_32_bit const  interpolation_param,
        vector3 const&  aim_at_target_in_agent_space,
        angeo::coordinate_system_explicit const&  agent_frame,
        std::vector<angeo::coordinate_system>&  frames_to_update
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(frames_to_update.size() == get_blackboard()->m_motion_templates.names().size());
    
    m_current_bones = (interpolation_param < 0.5f) ? m_src_bones : m_dst_bones;
    if (m_current_bones == nullptr || m_current_bones->all_bones.empty() || m_current_bones->end_effector_bones.empty())
        return;

    std::unordered_set<integer_32_bit>  bones_to_consider;
    for (auto bone : m_current_bones->all_bones)
        bones_to_consider.insert(bone);

    auto const&  parents = get_blackboard()->m_motion_templates.parents();

    vector3 const  target = angeo::point3_from_coordinate_system(aim_at_target_in_agent_space, agent_frame);
    std::vector<angeo::aim_at_goal>  goals;
    for (natural_32_bit bone : m_current_bones->end_effector_bones)
        goals.push_back({ bone, { angeo::aim_at_target::ORIGIN, target }});

    std::vector<angeo::coordinate_system>  target_frames;
    std::unordered_map<integer_32_bit, std::vector<natural_32_bit> >  bones_to_rotate;
    angeo::skeleton_aim_at(
            target_frames,
            goals,
            get_blackboard()->m_motion_templates.pose_frames().get_coord_systems(),
            parents,
            get_blackboard()->m_motion_templates.joints(),
            bones_to_consider,
            &bones_to_rotate
            );

    for (auto const& bone_and_anges : bones_to_rotate)
        get_blackboard()->m_scene->get_frame_of_scene_node(
            get_blackboard()->m_bone_nids.at(bone_and_anges.first),
            true,
            frames_to_update.at(bone_and_anges.first)
            );

    angeo::skeleton_rotate_bones_towards_target_pose(
            frames_to_update,
            target_frames,
            get_blackboard()->m_motion_templates.joints(),
            bones_to_rotate,
            time_step_in_seconds
            );
}


void  action_controller_interpolator_aim_at::set_target(
        skeletal_motion_templates::motion_template_cursor const&  cursor
        )
{
    m_src_bones = m_current_bones;
    m_dst_bones = get_blackboard()->m_motion_templates.motions_map().at(cursor.motion_name).aim_at.at(cursor.keyframe_index);
}


}}
