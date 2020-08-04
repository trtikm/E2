#ifndef AIOLD_DETAIL_ACTION_CONTROLLER_INTERPOLATOR_AIM_AT_HPP_INCLUDED
#   define AIOLD_DETAIL_ACTION_CONTROLLER_INTERPOLATOR_AIM_AT_HPP_INCLUDED

#   include <aiold/detail/action_controller_interpolator.hpp>
#   include <aiold/blackboard_agent.hpp>
#   include <aiold/skeletal_motion_templates.hpp>
#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <vector>

namespace aiold { namespace detail {


struct  action_controller_interpolator_aim_at  final : public action_controller_interpolator_shared
{
    using  aim_at_infos = std::vector<skeletal_motion_templates::aim_at_info_ptr>;
    
    explicit action_controller_interpolator_aim_at(
            action_controller_interpolator const* const  interpolator_,
            skeletal_motion_templates::motion_template_cursor const&  initial_template_cursor
            );
    void  interpolate(
            float_32_bit const  time_step_in_seconds,
            float_32_bit const  interpolation_param,
            vector3 const&  aim_at_target_in_agent_space,
            angeo::coordinate_system_explicit const&  agent_frame,
            std::vector<angeo::coordinate_system>&  frames_to_update
            );
    void  set_target(skeletal_motion_templates::motion_template_cursor const&  cursor);

    void  commit() const { /* nothing to commit acctualy. */ }

    aim_at_infos const&  get_current_aim_at_infos() const { return m_current_infos; }

private:
    aim_at_infos  m_src_infos;
    aim_at_infos  m_current_infos;
    aim_at_infos  m_dst_infos;
};


}}

#endif
