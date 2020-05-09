#include <ai/detail/action_controller_interpolator_animation.hpp>
#include <ai/skeleton_utils.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>

namespace ai { namespace detail {


action_controller_interpolator_animation::action_controller_interpolator_animation(
        action_controller_interpolator const* const  interpolator_,
        skeletal_motion_templates::motion_template_cursor const&  initial_template_cursor,
        float_32_bit const  reference_offset
        )
    : action_controller_interpolator_shared(interpolator_)
    , m_src_frames()
    , m_current_frames()
    , m_dst_frames()
    , m_reference_offset(reference_offset)
    , m_src_offset()
    , m_current_offset()
    , m_dst_offset()
{
    set_target(initial_template_cursor);
    m_src_frames = m_current_frames = m_dst_frames;
    m_src_offset = m_current_offset = m_dst_offset;
}


void  action_controller_interpolator_animation::interpolate(float_32_bit const  interpolation_param)
{
    interpolate_keyframes_spherical(m_src_frames, m_dst_frames, interpolation_param, m_current_frames);
    m_current_offset = m_src_offset + interpolation_param * (m_dst_offset - m_src_offset);
}


void  action_controller_interpolator_animation::set_target(
        skeletal_motion_templates::motion_template_cursor const&  cursor
        )
{
    m_src_frames = m_current_frames;
    transform_keyframes_to_reference_frame(
            get_blackboard()->m_motion_templates.at(cursor.motion_name).keyframes.get_keyframes().at(cursor.keyframe_index)
                                                                                                 .get_coord_systems(),
            get_blackboard()->m_motion_templates.at(cursor.motion_name).reference_frames.at(cursor.keyframe_index),
            get_blackboard()->m_motion_templates.pose_frames().get_coord_systems(),
            get_blackboard()->m_motion_templates.hierarchy().parents(),
            m_dst_frames
            );

    m_src_offset = m_current_offset;
    m_dst_offset = get_blackboard()->m_motion_templates.at(cursor.motion_name).bboxes.at(cursor.keyframe_index)(2);
}


void  action_controller_interpolator_animation::commit() const
{
    TMPROF_BLOCK();

    vector3 const  origin_offset(0.0f, 0.0f, m_current_offset - m_reference_offset);
    auto const&  parents = get_blackboard()->m_motion_templates.hierarchy().parents();
    for (natural_32_bit bone = 0; bone != m_current_frames.size(); ++bone)
        get_blackboard()->m_scene->set_frame_of_scene_node(
                get_blackboard()->m_bone_nids.at(bone),
                true,
                parents.at(bone) < 0 ?
                    angeo::coordinate_system{
                        m_current_frames.at(bone).origin() + origin_offset,
                        m_current_frames.at(bone).orientation()
                        }
                    :
                    m_current_frames.at(bone)
                );
}


}}
