#ifndef AI_ACTION_CONTROLLER_HUMAN_HPP_INCLUDED
#   define AI_ACTION_CONTROLLER_HUMAN_HPP_INCLUDED

#   include <ai/action_controller.hpp>
#   include <ai/blackboard_human.hpp>
#   include <scene/scene_node_id.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <angeo/tensor_math.hpp>
#   include <utility/invariants.hpp>
#   include <string>
#   include <memory>
#   include <unordered_map>

namespace ai {


struct  action_controller_human : public action_controller
{
    struct  motion_action_data
    { 
        virtual ~motion_action_data() {}
        virtual std::unique_ptr<motion_action_data>  clone() const { UNREACHABLE(); }
    };
    using  motion_action_data_map = std::unordered_map<std::string, std::unique_ptr<motion_action_data> >;

    struct  desired_props
    {
        desired_props()
            : forward_unit_vector_in_world_space(vector3_unit_x())
            , linear_velocity_unit_direction_in_world_space(vector3_unit_x())
            , linear_speed(0.0f)
        {}

        vector3  forward_unit_vector_in_world_space;
        vector3  linear_velocity_unit_direction_in_world_space;
        float_32_bit  linear_speed;
    };

    explicit action_controller_human(blackboard_ptr const  blackboard_);
    ~action_controller_human();

    blackboard_human_ptr  get_blackboard() const { return as<blackboard_human>(action_controller::get_blackboard()); }

    void  next_round(float_32_bit  time_step_in_seconds) override;

    desired_props const&  get_desired_props() const { return m_desire; }
    scene::node_id const&  get_motion_object_node_id() const { return m_motion_object_nid; }
    skeletal_motion_templates::template_motion_info const&  get_template_motion_info() const { return m_template_motion_info; }

private:
    desired_props  m_desire;

    skeletal_motion_templates::template_motion_info  m_template_motion_info;
    skeletal_motion_templates::motion_template_cursor  m_current_motion_template_cursor;
    scene::node_id  m_motion_object_nid;
    motion_action_data_map  m_motion_action_data;
};


}

#endif
