#ifndef AI_ACTION_CONTROLLER_HPP_INCLUDED
#   define AI_ACTION_CONTROLLER_HPP_INCLUDED

#   include <ai/blackboard_agent.hpp>
#   include <ai/skeletal_motion_templates.hpp>
#   include <ai/detail/action_controller_roller.hpp>
#   include <ai/detail/action_controller_interpolator_composed.hpp>
#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <memory>

namespace ai {


struct  action_controller
{
    explicit action_controller(blackboard_agent_weak_ptr const  blackboard_);
    virtual ~action_controller() {}

    virtual void  initialise() {}

    virtual void  synchronise_with_scene() { m_roller.synchronise_with_scene(); }
    virtual void  next_round(float_32_bit const  time_step_in_seconds);

    blackboard_agent_ptr  get_blackboard() const { return m_blackboard.lock(); }

    scene::node_id const&  get_roller_nid() const { return m_roller.get_roller_nid(); }
    scene::node_id const&  get_body_nid() const { return m_roller.get_body_nid(); }
    angeo::coordinate_system_explicit const&  get_agent_frame() const { return m_roller.get_agent_frame(); }
    std::vector<skeletal_motion_templates::look_at_info_ptr> const&  get_free_bones_for_look_at() const
    { return m_interpolator.get_look_at_iterpolator().get_current_bones(); }

private:

    blackboard_agent_weak_ptr  m_blackboard;

    detail::action_controller_roller  m_roller;

    skeletal_motion_templates::motion_template_cursor  m_template_cursor;
    detail::action_controller_interpolator_composed  m_interpolator;
};


using  action_controller_ptr = std::shared_ptr<action_controller>;
using  action_controller_const_ptr = std::shared_ptr<action_controller const>;


}

#endif
