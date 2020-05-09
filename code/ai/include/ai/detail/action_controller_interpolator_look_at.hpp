#ifndef AI_DETAIL_ACTION_CONTROLLER_INTERPOLATOR_LOOK_AT_HPP_INCLUDED
#   define AI_DETAIL_ACTION_CONTROLLER_INTERPOLATOR_LOOK_AT_HPP_INCLUDED

#   include <ai/detail/action_controller_interpolator.hpp>
#   include <ai/blackboard_agent.hpp>
#   include <ai/skeletal_motion_templates.hpp>
#   include <ai/sensory_controller_sight.hpp>
#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <vector>

namespace ai { namespace detail {


struct  action_controller_interpolator_look_at  final : public action_controller_interpolator_shared
{
    explicit action_controller_interpolator_look_at(
            action_controller_interpolator const* const  interpolator_,
            skeletal_motion_templates::motion_template_cursor const&  initial_template_cursor
            );
    void  interpolate(
            float_32_bit const  time_step_in_seconds,
            float_32_bit const  interpolation_param,
            vector3 const&  look_at_target_in_agent_space,
            angeo::coordinate_system_explicit const&  agent_frame,
            std::vector<angeo::coordinate_system>&  frames_to_update
            );
    void  set_target(skeletal_motion_templates::motion_template_cursor const&  cursor);

    void  commit() const { /* nothing to commit acctualy. */ }

    skeletal_motion_templates::free_bones_for_look_at_ptr  get_current_bones() const { return m_current_bones; }

private:
    void  update_look_at_target_in_local_space(
            vector3 const&  look_at_target_from_cortex,
            angeo::coordinate_system_explicit const&  agent_frame,
            ai::sensory_controller_sight::camera_perspective_ptr const  camera
            );

    skeletal_motion_templates::free_bones_for_look_at_ptr  m_src_bones;
    skeletal_motion_templates::free_bones_for_look_at_ptr  m_current_bones;
    skeletal_motion_templates::free_bones_for_look_at_ptr  m_dst_bones;

    vector3  m_look_at_target_in_local_space;
    bool  m_target_pose_reached;
};


}}

#endif
