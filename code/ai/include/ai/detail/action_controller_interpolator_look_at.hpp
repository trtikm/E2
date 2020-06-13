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
    using  look_at_infos = std::vector<skeletal_motion_templates::look_at_info_ptr>;

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

    void  commit() const { /* nothing to commit actually. */ }

    look_at_infos const&  get_current_bones() const { return m_current_infos; }

private:
    void  update_look_at_target_in_local_space(
            vector3 const&  look_at_target_from_cortex,
            angeo::coordinate_system_explicit const&  agent_frame,
            ai::sensory_controller_sight::camera_perspective_ptr const  camera
            );

    look_at_infos  m_src_infos;
    look_at_infos  m_current_infos;
    look_at_infos  m_dst_infos;

    vector3  m_look_at_target_in_local_space;
    bool  m_target_pose_reached;
};


}}

#endif
