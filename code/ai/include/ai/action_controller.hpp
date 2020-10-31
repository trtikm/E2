#ifndef AI_ACTION_CONTROLLER_HPP_INCLUDED
#   define AI_ACTION_CONTROLLER_HPP_INCLUDED

#   include <ai/agent_config.hpp>
#   include <ai/skeleton_interpolators.hpp>
#   include <ai/agent_state_variables.hpp>
#   include <ai/skeletal_motion_templates.hpp>
#   include <ai/scene_binding.hpp>
#   include <ai/motion_desire_props.hpp>
#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <angeo/collision_shape_id.hpp>
#   include <angeo/collision_material.hpp>
#   include <angeo/collision_class.hpp>
#   include <angeo/custom_constraint_id.hpp>
#   include <com/object_guid.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <string>
#   include <array>
#   include <unordered_map>
#   include <vector>
#   include <memory>

namespace ai {


struct  agent;


struct  action_execution_context
{
    struct  scene_object_relative_path
    {
        com::object_guid  base_folder_guid;
        std::string  relative_path;
    };

    explicit  action_execution_context(agent* const  myself_);

    motion_desire_props const&  desire() const;
    agent_state_variables&  state_variables() const;
    skeletal_motion_templates  motion_templates() const;
    scene_binding const&  binding() const;
    com::simulation_context const&  ctx() const;

    agent*  myself;
    skeleton_interpolator_animation  animate;
    skeleton_interpolator_look_at  look_at;
    skeleton_interpolator_aim_at  aim_at;
    float_32_bit  time_buffer;
    std::vector<scene_object_relative_path>  disabled_colliding_with_our_motion_object;
};


using  action_execution_context_ptr = std::shared_ptr<action_execution_context>;


struct  agent_action
{
    struct  desire_config
    {
        motion_desire_props  ideal;
        motion_desire_props  weights;
        float_32_bit  treashold;
    };

    struct  effect_config
    {
        // Represent an experssion of this form:
        //      coefinient * (variable.empty() ? 1.0f : read(variable)) * std::pow(dt, dt_exponent)
        // where dt is the current simulation time step and read(...) function reads the current
        // value of the variable.
        struct  derivative_props
        {
            std::string  variable;      // When empty, then no variable is used in the expression.
            float_32_bit  coeficient;
            float_32_bit  dt_exponent;
        };
        std::string  variable;  // Name of the output variable which will be modified in this action effect.
                                // Must always be a valid variable name, in particular can NOT be empty.
        std::vector<derivative_props>  derivative;  // Time derivative of the output variable represented in
                                                    // the form of a sum of derivatives of all variables
                                                    // involved in the overall derivative.
    };

    struct  motion_object_config
    {
        bool  operator==(motion_object_config const&  other) const;
        bool  operator!=(motion_object_config const&  other) const { return !(*this == other); }

        angeo::COLLISION_SHAPE_TYPE  shape_type;
        vector3  aabb_half_size;
        angeo::COLLISION_MATERIAL_TYPE  collision_material;
        float_32_bit  mass_inverted;
        matrix33  inertia_tensor_inverted;
        bool  is_moveable;
    };

    struct  sensor_config
    {
        std::string  name;
        std::string  under_folder;
        angeo::COLLISION_SHAPE_TYPE  shape_type;
        angeo::COLLISION_CLASS  collision_class;
        vector3  aabb_half_size;
        angeo::coordinate_system  frame;
    };

    struct  transition_config
    {
        enum struct AABB_ALIGNMENT : natural_8_bit { CENTER, X_LO, X_HI, Y_LO, Y_HI, Z_LO, Z_HI };
        enum struct PERCEPTION_KIND : natural_8_bit { SIGHT, TOUCH };

        struct  location_constraint_config
        {
            std::string  frame_folder;
            angeo::COLLISION_SHAPE_TYPE  shape_type; // Only BOX, CAPSULE, or SPHERE!
            vector3  origin;
            vector3  aabb_half_size;
        };

        struct  perception_guard_config
        {
            PERCEPTION_KIND  perception_kind;
            std::string  sensor_folder_name;
            com::OBJECT_KIND  sensor_owner_kind; // Only AGENT or SENSOR.
            bool  sensor_owner_can_be_myself; // Only used when 'sensor_owner_kind==AGENT'; for SENSOR is always false.
            location_constraint_config  location_constraint;

        };

        struct  motion_object_position_config
        {
            bool  is_self_frame;
            std::string  frame_folder;
            vector3  origin;
            std::string  disable_colliding;
        };

        std::shared_ptr<perception_guard_config>  perception_guard;
        std::shared_ptr<motion_object_position_config>  motion_object_position;
        AABB_ALIGNMENT  aabb_alignment;
    };

    struct  transition_info
    {
        com::object_guid  other_entiry_folder_guid;
    };

    struct  skeletal_animation_info
    {
        float_32_bit  last_keyframe_completion_time;
        natural_32_bit  target_keyframe_index;
        natural_32_bit  end_keyframe_index;
    };

    explicit  agent_action(
            std::string const&  name_,
            boost::property_tree::ptree const&  ptree,
            boost::property_tree::ptree const&  defaults_,
            action_execution_context_ptr const  context_
            );
    virtual  ~agent_action() {}

    agent&  myself() const { return *m_context->myself; }
    motion_desire_props const&  desire() const { return m_context->desire(); }
    agent_state_variables&  state_variables() const { return m_context->state_variables(); }
    skeletal_motion_templates  agent_action::motion_templates() const { return m_context->motion_templates(); }
    scene_binding const&  agent_action::binding() const { return m_context->binding(); }
    com::simulation_context const&  agent_action::ctx() const { return m_context->ctx(); }

    float_32_bit  compute_desire_penalty() const;

    bool  is_cyclic() const { return IS_CYCLIC; }
    bool  is_complete() const;
    bool  is_ghost_complete() const;
    bool  use_ghost_object_for_skeleton_location() const { return USE_GHOST_OBJECT_FOR_SKELETON_LOCATION; }

    float_32_bit  interpolation_parameter() const;
    float_32_bit  interpolation_parameter_ghost() const;

    std::unordered_map<std::string, transition_config> const&  get_transitions() const { return TRANSITIONS; };
    motion_object_config const&  get_motion_object_config() const { return MOTION_OBJECT_CONFIG; }

    com::object_guid  get_frame_guid_of_skeleton_location() const;

    void  apply_effects(float_32_bit const  time_step_in_seconds);
    void  update_time(float_32_bit const  time_step_in_seconds);
    void  update_ghost();
    void  update_look_at(float_32_bit const  time_step_in_seconds);
    void  update_aim_at(float_32_bit const  time_step_in_seconds);
    void  update_animation(float_32_bit const  time_step_in_seconds);

    bool  is_guard_valid() const;

    bool  collect_transition_info(agent_action const&  from_action, transition_info&  info) const;

    virtual void  on_transition(agent_action* const  from_action_ptr, transition_info const&  info);
    virtual void  next_round(float_32_bit const  time_step_in_seconds);

    using  on_custom_folder_erase_func = std::function<void()>;
    virtual void  get_custom_folders(std::unordered_map<com::object_guid, on_custom_folder_erase_func>&  folders) {}

protected:
    void  get_motion_object_relocation_frame(
            angeo::coordinate_system&  frame,
            agent_action* const  from_action_ptr,
            transition_info const&  info
            ) const;

    // CONSTANTS

    std::string  NAME;
    desire_config  DESIRE;
    std::vector<effect_config>  EFFECTS;
    std::string  MOTION_TEMPLATE_NAME;
    bool  ONLY_INTERPOLATE_TO_MOTION_TEMPLATE;
    bool  IS_CYCLIC;
    bool  IS_LOOK_AT_ENABLED;
    bool  IS_AIM_AT_ENABLED;
    bool  USE_GHOST_OBJECT_FOR_SKELETON_LOCATION;
    std::unordered_map<std::string, sensor_config>  SENSORS;
    std::unordered_map<std::string, transition_config>  TRANSITIONS;
    motion_object_config  MOTION_OBJECT_CONFIG;
    vector3  MOTION_OBJECT_RELOCATION_OFFSET;

private:
    void  load_desire(boost::property_tree::ptree const&  ptree, boost::property_tree::ptree const&  defaults);
    void  load_effects(boost::property_tree::ptree const&  ptree, boost::property_tree::ptree const&  defaults);
    void  load_motion_object_config(boost::property_tree::ptree const&  ptree, boost::property_tree::ptree const&  defaults_);
    void  load_sensors(boost::property_tree::ptree const&  ptree);
    void  load_transitions(boost::property_tree::ptree const&  ptree, boost::property_tree::ptree const&  defaults);

    // MUTABLE DATA

    action_execution_context_ptr  m_context;

    float_32_bit  m_start_time;
    float_32_bit  m_end_time;
    float_32_bit  m_end_ghost_time;
    float_32_bit  m_current_time;

    angeo::coordinate_system  m_ghost_object_start_coord_system;

    skeletal_animation_info  m_animation;
};


struct  action_guesture : public  agent_action
{
    explicit  action_guesture(
            std::string const&  name_,
            boost::property_tree::ptree const&  ptree_,
            boost::property_tree::ptree const&  defaults_,
            action_execution_context_ptr const  context_
            );
    void  on_transition(agent_action* const  from_action_ptr, transition_info const&  info) override;
    void  next_round(float_32_bit const  time_step_in_seconds) override;
};


//struct  action_interpolate : public  agent_action
//{
//    explicit  action_interpolate(
//            std::string const&  name_,
//            boost::property_tree::ptree const&  ptree_,
//            boost::property_tree::ptree const&  defaults_,
//            action_execution_context_ptr const  context_
//            );
//    //void  on_transition(agent_action* const  from_action_ptr, transition_info const&  info) override {}
//    void  next_round(float_32_bit const  time_step_in_seconds) override;
//    angeo::coordinate_system  target;
//};


struct  action_roller : public  agent_action
{
    struct  roller_object_config
    {
        bool  operator==(roller_object_config const&  other) const;
        bool  operator!=(roller_object_config const&  other) const { return !(*this == other); }

        float_32_bit  roller_radius;
        float_32_bit  roller_mass_inverted;
    };

    explicit  action_roller(
            std::string const&  name_,
            boost::property_tree::ptree const&  ptree_,
            boost::property_tree::ptree const&  defaults_,
            action_execution_context_ptr const  context_
            );
    void  on_transition(agent_action* const  from_action_ptr, transition_info const&  info) override;
    void  next_round(float_32_bit const  time_step_in_seconds) override;
    void  get_custom_folders(std::unordered_map<com::object_guid, on_custom_folder_erase_func>&  folders) override;

private:
    void  create_custom_constraint_ids();
    void  release_custom_constraint_ids();
    void  insert_joint_between_roller_and_motion_object() const;

    // CONSTANTS

    roller_object_config  ROLLER_CONFIG;

    // MUTABLE DATA

    com::object_guid  m_roller_folder_guid;
    com::object_guid  m_roller_frame_guid;
    std::array<angeo::custom_constraint_id, 3>  m_roller_joint_ccids;
};


struct  action_controller
{
    action_controller(agent_config const  config, agent*  const  myself);
    ~action_controller();

    void  next_round(float_32_bit const  time_step_in_seconds);

    agent_action const&  get_current_action() const { return *m_current_action; }

private:
    void  process_action_transitions();

    action_execution_context_ptr  m_context;
    std::shared_ptr<agent_action>  m_current_action;
    std::unordered_map<std::string, std::shared_ptr<agent_action> >  m_available_actions;
};


using  action_controller_ptr = std::shared_ptr<action_controller>;
using  action_controller_const_ptr = std::shared_ptr<action_controller const>;


}

#endif
