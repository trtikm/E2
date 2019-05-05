#ifndef AI_ACTION_CONTROLLER_HUMAN_HPP_INCLUDED
#   define AI_ACTION_CONTROLLER_HUMAN_HPP_INCLUDED

#   include <ai/action_controller.hpp>
#   include <ai/blackboard_human.hpp>
#   include <scene/scene_node_id.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <string>

namespace ai {


struct  action_controller_human : public action_controller
{
    explicit action_controller_human(blackboard_ptr const  blackboard_);
    ~action_controller_human();

    blackboard_human_ptr  get_blackboard() const { return as<blackboard_human>(action_controller::get_blackboard()); }

    void  next_round(float_32_bit  time_step_in_seconds) override;

private:
    skeletal_motion_templates::template_motion_info  m_template_motion_info;
    vector3  m_desired_linear_velocity_in_world_space;
    float_32_bit  m_desired_angular_speed_in_world_space;   // Only the magnitude, because the axis is the z-axis of
                                                            // 'm_reference_frame_in_world_space'.
    scene::node_id  m_motion_object_nid;
    skeletal_motion_templates::keyframes::meta_data::record  m_motion_object_collider_props;
    skeletal_motion_templates::keyframes::meta_data::record  m_motion_object_constraint_props;
};


}

#endif
