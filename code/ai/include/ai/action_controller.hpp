#ifndef AI_ACTION_CONTROLLER_HPP_INCLUDED
#   define AI_ACTION_CONTROLLER_HPP_INCLUDED

#   include <ai/skeletal_motion_templates.hpp>
#   include <ai/scene_binding.hpp>
#   include <ai/motion_desire_props.hpp>
#   include <ai/detail/action_controller_roller.hpp>
#   include <ai/detail/action_controller_interpolator_composed.hpp>
#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <com/object_guid.hpp>
#   include <vector>
#   include <memory>

namespace com { struct  simulation_context; }

namespace ai {


struct  action_controller
{
    action_controller(
            skeletal_motion_templates const  motion_templates,
            scene_binding_ptr const  binding
            );

    void  next_round(float_32_bit const  time_step_in_seconds, motion_desire_props const&  desire);

private:
    skeletal_motion_templates  get_motion_templates() const { return m_interpolator.get_motion_templates(); }

    detail::action_controller_roller  m_roller;
    skeletal_motion_templates::motion_template_cursor  m_template_cursor;
    detail::action_controller_interpolator_composed  m_interpolator;
};


using  action_controller_ptr = std::shared_ptr<action_controller>;
using  action_controller_const_ptr = std::shared_ptr<action_controller const>;


}

#endif
