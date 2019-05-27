#ifndef AI_ACTION_CONTROLLER_HUMAN_HPP_INCLUDED
#   define AI_ACTION_CONTROLLER_HUMAN_HPP_INCLUDED

#   include <ai/action_controller.hpp>
#   include <ai/blackboard_human.hpp>
#   include <scene/scene_node_id.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <utility/basic_numeric_types.hpp>
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

    explicit action_controller_human(blackboard_ptr const  blackboard_);
    ~action_controller_human();

    blackboard_human_ptr  get_blackboard() const { return as<blackboard_human>(action_controller::get_blackboard()); }

    void  next_round(float_32_bit  time_step_in_seconds) override;

private:
    vector3  m_desired_forward_unit_vector_in_world_space;
    vector3  m_desired_linear_velocity_unit_direction_in_world_space;
    float_32_bit  m_desired_linear_speed;

    skeletal_motion_templates::template_motion_info  m_template_motion_info;
    scene::node_id  m_motion_object_nid;
    skeletal_motion_templates::meta_records_real  m_motion_object_collider_props;
    skeletal_motion_templates::meta_records_real  m_motion_object_mass_distribution_props;
    skeletal_motion_templates::meta_records_real  m_motion_object_constraint_props;
    skeletal_motion_templates::meta_records_real  m_motion_object_action_props;
    motion_action_data_map  m_motion_action_data;
};


}

#endif
