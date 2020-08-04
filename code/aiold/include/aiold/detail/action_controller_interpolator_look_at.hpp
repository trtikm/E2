#ifndef AIOLD_DETAIL_ACTION_CONTROLLER_INTERPOLATOR_LOOK_AT_HPP_INCLUDED
#   define AIOLD_DETAIL_ACTION_CONTROLLER_INTERPOLATOR_LOOK_AT_HPP_INCLUDED

#   include <aiold/detail/action_controller_interpolator.hpp>
#   include <aiold/blackboard_agent.hpp>
#   include <aiold/skeletal_motion_templates.hpp>
#   include <aiold/sensory_controller_sight.hpp>
#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <angeo/skeleton_kinematics.hpp>
#   include <vector>

namespace aiold { namespace detail {


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

    void  set_target(skeletal_motion_templates::motion_template_cursor const&  cursor) { /* nothing to do. */ }
    void  commit() const { /* nothing to commit actually. */ }

private:
    void  update_look_at_target_in_local_space(
            vector3 const&  look_at_target_from_cortex,
            angeo::coordinate_system_explicit const&  agent_frame,
            aiold::sensory_controller_sight::camera_perspective_ptr const  camera
            );

    angeo::joint_rotation_states_of_bones  m_joint_rotations;
    vector3  m_look_at_target_in_local_space;
    bool  m_target_pose_reached;
};


}}

#endif
