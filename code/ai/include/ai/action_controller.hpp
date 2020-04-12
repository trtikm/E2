#ifndef AI_ACTION_CONTROLLER_HPP_INCLUDED
#   define AI_ACTION_CONTROLLER_HPP_INCLUDED

#   include <ai/blackboard_agent.hpp>
#   include <ai/scene.hpp>
#   include <ai/motion_desire_props.hpp>
#   include <ai/detail/rigid_body_motion.hpp>
#   include <ai/detail/guarded_motion_actions_processor.hpp>
#   include <ai/detail/action_controller_interpolator_animation.hpp>
#   include <ai/detail/action_controller_interpolator_look_at.hpp>
#   include <ai/detail/action_controller_interpolator_matter.hpp>
#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <string>
#   include <memory>
#   include <vector>
#   include <unordered_map>

namespace ai {


struct  action_controller
{
    struct  intepolation_state
    {
        intepolation_state();

        std::vector<angeo::coordinate_system>  frames;
        skeletal_motion_templates::free_bones_for_look_at_ptr  free_bones_look_at;
        skeletal_motion_templates::collider_ptr  collider;
        skeletal_motion_templates::mass_distribution_ptr  mass_distribution;
        skeletal_motion_templates::disjunction_of_guarded_actions  disjunction_of_guarded_actions;
    };

    explicit action_controller(blackboard_agent_weak_ptr const  blackboard_);
    virtual ~action_controller();

    virtual void  initialise();
    virtual void  next_round(float_32_bit const  time_step_in_seconds);

    void  synchronise_motion_object_motion_with_scene();

    blackboard_agent_ptr  get_blackboard() const { return m_blackboard.lock(); }

    scene::node_id const& get_motion_object_node_id() const { return m_motion_object_motion.nid; }
    detail::rigid_body_motion const&  get_motion_object_motion() const { return m_motion_object_motion; }
    vector3 const&  get_environment_linear_velocity() const { return m_environment_linear_velocity; }
    vector3 const&  get_environment_angular_velocity() const { return m_environment_angular_velocity; }
    float_32_bit  get_environment_acceleration_coef() const { return m_environment_acceleration_coef; }
    vector3 const&  get_external_linear_acceleration() const { return m_external_linear_acceleration; }
    vector3 const&  get_external_angular_acceleration() const { return m_external_angular_acceleration; }
    float_32_bit  get_interpolation_parameter() const { return m_interpolator_animation.get_interpolation_parameter(); }
    skeletal_motion_templates::motion_template_cursor const&  get_destination_cursor() const { return m_dst_cursor; }
    vector3 const&  get_ideal_linear_velocity_in_world_space() const { return m_ideal_linear_velocity_in_world_space; }
    vector3 const&  get_ideal_angular_velocity_in_world_space() const { return m_ideal_angular_velocity_in_world_space; }
    skeletal_motion_templates::free_bones_for_look_at_ptr  get_free_bones_for_look_at() const { return m_interpolator_look_at.get_current_bones(); }

protected:

    // The motion of the agent in world space. The velocities and
    // accelerations should be computed with consideration of
    // velocity of the environment and external acceleration (see fields below).
    detail::rigid_body_motion  m_motion_object_motion;
    // Linear and angular velocity of agent's reference frame attached
    // to the environment. For example, when a humanoid agent stays on
    // an escalator, then these represent the velocity of the stairs;
    // when the agent in on a boat, airplain, or a car, then the velocities
    // are those of the vehicles; when the agent is a fish in a river, then
    // the velocities represent the flow of the water in its close proximity.
    vector3  m_environment_linear_velocity;     
    vector3  m_environment_angular_velocity;
    // in range <0,1>; 0 means no fraction of agent's desired accel
    // is used to affect agent's motion; the value 1 means that whole
    // agent's desired accel affects its motion in the enviroment.
    float_32_bit  m_environment_acceleration_coef;
    // Totsal external forces acting on the agent from force/accel fields.
    // For example: gravity force.
    vector3  m_external_linear_acceleration;
    vector3  m_external_angular_acceleration;

private:

    void  compute_environment_motion(std::vector<scene::collicion_contant_info_ptr> const& contacts_in_normal_cone);
    void  clear_environment_motion();

    blackboard_agent_weak_ptr  m_blackboard;

    detail::action_controller_interpolator_animation  m_interpolator_animation;
    detail::action_controller_interpolator_look_at  m_interpolator_look_at;
    detail::action_controller_interpolator_matter  m_interpolator_matter;

    skeletal_motion_templates::motion_template_cursor  m_dst_cursor;

    vector3  m_ideal_linear_velocity_in_world_space;
    vector3  m_ideal_angular_velocity_in_world_space;
    detail::motion_action_persistent_data_map  m_motion_action_data;
};


using  action_controller_ptr = std::shared_ptr<action_controller>;
using  action_controller_const_ptr = std::shared_ptr<action_controller const>;


}

#endif
