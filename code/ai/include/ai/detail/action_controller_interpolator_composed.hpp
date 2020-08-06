#ifndef AI_DETAIL_ACTION_CONTROLLER_INTERPOLATOR_COMPOSED_HPP_INCLUDED
#   define AI_DETAIL_ACTION_CONTROLLER_INTERPOLATOR_COMPOSED_HPP_INCLUDED

#   include <ai/detail/action_controller_interpolator.hpp>
#   include <ai/detail/action_controller_interpolator_animation.hpp>
#   include <ai/detail/action_controller_interpolator_look_at.hpp>
#   include <ai/detail/action_controller_interpolator_aim_at.hpp>
#   include <ai/skeletal_motion_templates.hpp>
#   include <ai/scene_binding.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <com/object_guid.hpp>
#   include <vector>

namespace ai { namespace detail {


struct  action_controller_interpolator_composed : public  action_controller_interpolator
{
    action_controller_interpolator_composed(
            skeletal_motion_templates const  motion_templates,
            scene_binding_ptr const  binding,
            skeletal_motion_templates::motion_template_cursor const&  initial_template_cursor,
            float_32_bit const  reference_offset
            );
    ~action_controller_interpolator_composed() {}

    void  next_round(
            float_32_bit const  time_step_in_seconds,
            vector3 const&  look_at_target_in_agent_space,
            vector3 const&  aim_at_target_in_agent_space
            );
    void  set_target(skeletal_motion_templates::motion_template_cursor const&  cursor);
    void  commit() const;

    action_controller_interpolator_animation const&  get_animation_iterpolator() const { return m_animation; }
    action_controller_interpolator_look_at const&  get_look_at_iterpolator() const { return m_look_at; }
    action_controller_interpolator_aim_at const&  get_aim_at_iterpolator() const { return m_aim_at; }

private:
    action_controller_interpolator_animation  m_animation;
    action_controller_interpolator_look_at  m_look_at;
    action_controller_interpolator_aim_at  m_aim_at;
};


}}

#endif
