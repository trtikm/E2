#ifndef AI_ACTION_CONTROLLER_HPP_INCLUDED
#   define AI_ACTION_CONTROLLER_HPP_INCLUDED

#   include <ai/blackboard.hpp>
#   include <ai/scene.hpp>
#   include <ai/detail/motion_desire_props.hpp>
#   include <ai/detail/rigid_body_motion.hpp>
#   include <ai/detail/guarded_motion_actions_processor.hpp>
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

    explicit action_controller(blackboard_weak_ptr const  blackboard_);
    virtual ~action_controller();

    virtual void  initialise() {}
    virtual void  next_round(float_32_bit const  time_step_in_seconds);

    blackboard_ptr  get_blackboard() const { return m_blackboard.lock(); }

    scene::node_id const& get_motion_object_node_id() const { return m_motion_object_motion.nid; }
    detail::rigid_body_motion const&  get_motion_object_motion() const { return m_motion_object_motion; }
    vector3 const&  get_gravity_acceleration() const { return m_gravity_acceleration; }
    float_32_bit  get_total_time_till_destination_cursor_in_seconds() const { return m_total_interpolation_time_in_seconds; }
    float_32_bit  get_consumed_time_till_destination_cursor_in_seconds() const { return m_consumed_time_in_seconds; }
    skeletal_motion_templates::motion_template_cursor const&  get_destination_cursor() const { return m_dst_cursor; }
    vector3 const&  get_ideal_linear_velocity_in_world_space() const { return m_ideal_linear_velocity_in_world_space; }
    vector3 const&  get_ideal_angular_velocity_in_world_space() const { return m_ideal_angular_velocity_in_world_space; }
    skeletal_motion_templates::free_bones_for_look_at_ptr  get_free_bones_for_look_at() const { return m_current_intepolation_state.free_bones_look_at; }

protected:

    detail::rigid_body_motion  m_motion_object_motion;
    vector3  m_gravity_acceleration;

private:

    float_32_bit  compute_interpolation_speed() const;
    void  interpolate(float_32_bit const  interpolation_param);
    void  look_at_target(float_32_bit const  time_step_in_seconds, float_32_bit const  interpolation_param);

    blackboard_weak_ptr  m_blackboard;

    float_32_bit  m_total_interpolation_time_in_seconds;
    float_32_bit  m_consumed_time_in_seconds;

    intepolation_state  m_src_intepolation_state;
    intepolation_state  m_current_intepolation_state;
    bool  m_use_inverted_collider_center_offset_interpolation;

    skeletal_motion_templates::motion_template_cursor  m_dst_cursor;
    std::vector<angeo::coordinate_system>  m_dst_frames;
    vector3  m_ideal_linear_velocity_in_world_space;
    vector3  m_ideal_angular_velocity_in_world_space;
    vector3  m_collider_center_offset_in_reference_frame;

    detail::motion_action_persistent_data_map  m_motion_action_data;
};


using  action_controller_ptr = std::shared_ptr<action_controller>;
using  action_controller_const_ptr = std::shared_ptr<action_controller const>;


}

#endif
