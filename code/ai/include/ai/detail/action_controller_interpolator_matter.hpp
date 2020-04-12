#ifndef AI_DETAIL_ACTION_CONTROLLER_INTERPOLATOR_MATTER_HPP_INCLUDED
#   define AI_DETAIL_ACTION_CONTROLLER_INTERPOLATOR_MATTER_HPP_INCLUDED

#   include <ai/detail/action_controller_interpolator.hpp>
#   include <ai/blackboard_agent.hpp>
#   include <ai/skeletal_motion_templates.hpp>
#   include <ai/detail/rigid_body_motion.hpp>
#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <vector>

namespace ai { namespace detail {


struct  action_controller_interpolator_matter  final : public action_controller_interpolator
{
    explicit action_controller_interpolator_matter(blackboard_agent_weak_ptr const  blackboard_);
    void  next_round(float_32_bit const  time_step_in_seconds);
    void  set_target(skeletal_motion_templates::motion_template_cursor const&  cursor, float_32_bit const  interpolation_time_in_seconds);

    void  commit(detail::rigid_body_motion&  rb_motion) const;

    bool  is_matter_changed() const { return m_matter_changed; }

    skeletal_motion_templates::collider_ptr  get_current_collider() const { return m_current_collider; }
    skeletal_motion_templates::mass_distribution_ptr  get_current_mass_distribution() const { return m_current_mass_distribution; }
    vector3 const&  get_interpolated_offset() const { return m_interpolated_offset; }

private:
    skeletal_motion_templates::collider_ptr  m_src_collider;
    skeletal_motion_templates::mass_distribution_ptr  m_src_mass_distribution;
    skeletal_motion_templates::collider_ptr  m_current_collider;
    skeletal_motion_templates::mass_distribution_ptr  m_current_mass_distribution;
    skeletal_motion_templates::collider_ptr  m_dst_collider;
    skeletal_motion_templates::mass_distribution_ptr  m_dst_mass_distribution;

    bool  m_matter_changed;

    bool  m_use_inverted_collider_center_offset_interpolation;
    vector3  m_collider_center_offset_in_reference_frame;
    vector3  m_interpolated_offset;
};


}}

#endif
