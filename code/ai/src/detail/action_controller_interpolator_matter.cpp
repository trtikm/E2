#include <ai/detail/action_controller_interpolator_matter.hpp>
#include <ai/detail/collider_utils.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>

namespace ai { namespace detail {


action_controller_interpolator_matter::action_controller_interpolator_matter(blackboard_agent_weak_ptr const  blackboard_)
    : action_controller_interpolator(blackboard_)

    , m_src_collider()
    , m_src_mass_distribution()
    , m_current_collider()
    , m_current_mass_distribution()
    , m_dst_collider()
    , m_dst_mass_distribution()

    , m_matter_changed(true)
    , m_use_inverted_collider_center_offset_interpolation()
    , m_collider_center_offset_in_reference_frame()
{
    ASSUMPTION(get_blackboard() != nullptr && !get_blackboard()->m_motion_templates.empty());
    set_target({ get_blackboard()->m_motion_templates.transitions().initial_motion_name(), 0U }, 0.0f);
    m_src_collider = m_current_collider = m_dst_collider;
    m_src_mass_distribution = m_current_mass_distribution = m_dst_mass_distribution;
}


void  action_controller_interpolator_matter::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    if (done()) return;
    float_32_bit const  interpolation_param = action_controller_interpolator::next_round(time_step_in_seconds);

    float_32_bit const  weighted_interpolation_param =
            (m_src_collider->weight + m_dst_collider->weight < 0.0001f) ?
                    0.5f :
                    m_src_collider->weight / (m_src_collider->weight + m_dst_collider->weight);

    skeletal_motion_templates::collider_ptr  interpolated_collider;
    skeletal_motion_templates::mass_distribution_ptr  interpolated_mass_distribution;
    if (interpolation_param < weighted_interpolation_param)
    {
        interpolated_collider = m_src_collider;
        interpolated_mass_distribution = m_src_mass_distribution;
    }
    else
    {
        interpolated_collider = m_dst_collider;
        interpolated_mass_distribution = m_dst_mass_distribution;
    }

    m_matter_changed = *interpolated_collider != *m_current_collider ||
                       *interpolated_mass_distribution != *m_current_mass_distribution;

    if (m_matter_changed)
    {
        m_current_collider = interpolated_collider;
        m_current_mass_distribution = interpolated_mass_distribution;
        m_use_inverted_collider_center_offset_interpolation = true;
    }

    m_interpolated_offset =
            (interpolation_param - (m_use_inverted_collider_center_offset_interpolation ? 1.0f : 0.0f))
            * m_collider_center_offset_in_reference_frame
            ;
}


void  action_controller_interpolator_matter::set_target(
        skeletal_motion_templates::motion_template_cursor const&  cursor,
        float_32_bit const  interpolation_time_in_seconds
        )
{
    TMPROF_BLOCK();

    action_controller_interpolator::reset_time(interpolation_time_in_seconds);

    m_src_collider = m_current_collider;
    m_src_mass_distribution = m_current_mass_distribution;

    m_dst_collider = get_blackboard()->m_motion_templates.at(cursor.motion_name).colliders.at(cursor.keyframe_index);
    m_dst_mass_distribution = get_blackboard()->m_motion_templates.at(cursor.motion_name).mass_distributions.at(cursor.keyframe_index);

    m_use_inverted_collider_center_offset_interpolation = false;
    m_collider_center_offset_in_reference_frame =
            m_src_collider == nullptr || m_dst_collider == nullptr ?
                    vector3_zero() :
                    compute_offset_for_center_of_second_collider_to_get_surfaces_alignment_in_direction(
                            m_src_collider,
                            m_dst_collider,
                            -get_blackboard()->m_motion_templates.directions().up()
                            );
}


void  action_controller_interpolator_matter::commit(detail::rigid_body_motion&  rb_motion) const
{
    if (!is_matter_changed())
        return;

    rb_motion.inverted_mass = m_current_mass_distribution->mass_inverted;
    rb_motion.inverted_inertia_tensor = m_current_mass_distribution->inertia_tensor_inverted;
    {
        matrix44  W;
        angeo::from_base_matrix(rb_motion.frame, W);
        vector3 const  offset = transform_vector(m_collider_center_offset_in_reference_frame, W);
        angeo::translate(rb_motion.frame, offset);
    }

    get_blackboard()->m_scene->unregister_from_collision_contacts_stream(rb_motion.nid, get_blackboard()->m_self_id);
    detail::destroy_collider_and_rigid_bofy_of_motion_scene_node(get_blackboard()->m_scene, rb_motion.nid);
    detail::create_collider_and_rigid_body_of_motion_scene_node(
        get_blackboard()->m_scene,
        rb_motion.nid,
        m_current_collider,
        rb_motion.frame,
        rb_motion.velocity,
        {
            get_blackboard()->m_scene->get_initial_external_linear_acceleration_at_point(rb_motion.frame.origin()),
            get_blackboard()->m_scene->get_initial_external_angular_acceleration_at_point(rb_motion.frame.origin())
        },
        rb_motion.inverted_mass,
        rb_motion.inverted_inertia_tensor
    );
    get_blackboard()->m_scene->register_to_collision_contacts_stream(rb_motion.nid, get_blackboard()->m_self_id);
    rb_motion.commit_frame(get_blackboard()->m_scene, get_blackboard()->m_self_rid.get_node_id());
}


}}
