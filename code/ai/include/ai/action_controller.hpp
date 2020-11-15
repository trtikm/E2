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
#   include <angeo/linear_segment_curve.hpp>
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

    enum struct AABB_ALIGNMENT : natural_8_bit { CENTER, X_LO, X_HI, Y_LO, Y_HI, Z_LO, Z_HI };
    static  AABB_ALIGNMENT  aabb_alignment_from_string(std::string const&  alignment_name);

    struct  motion_object_config
    {
        bool  operator==(motion_object_config const&  other) const;
        bool  operator!=(motion_object_config const&  other) const { return !(*this == other); }

        angeo::COLLISION_SHAPE_TYPE  shape_type;
        vector3  aabb_half_size;
        AABB_ALIGNMENT  aabb_alignment; // Alignment inside AABB 'agent_action::AABB_HALF_SIZE'.
        angeo::COLLISION_MATERIAL_TYPE  collision_material;
        float_32_bit  mass_inverted;
        matrix33  inertia_tensor_inverted;
        bool  is_moveable;
        vector3  offset_from_center_of_agent_aabb;
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
        enum struct PERCEPTION_KIND : natural_8_bit { SIGHT, TOUCH };

        struct  location_constraint_config
        {
            std::string  frame_folder;
            angeo::COLLISION_SHAPE_TYPE  shape_type; // Only BOX, CAPSULE, or SPHERE!
            angeo::coordinate_system  frame;
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

        struct  motion_object_location_config
        {
            bool  is_self_frame;
            std::string  frame_folder;
            angeo::coordinate_system  frame;
            std::vector<std::string>  relative_paths_to_colliders_for_disable_colliding;
        };

        std::shared_ptr<perception_guard_config>  perception_guard;
        std::shared_ptr<motion_object_location_config>  motion_object_location;
        AABB_ALIGNMENT  aabb_alignment;
        bool  is_available_only_in_action_end;
    };

    struct  transition_info
    {
        com::object_guid  other_entiry_folder_guid;
        angeo::coordinate_system  motion_object_relocation_frame;
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

    float_32_bit  interpolation_parameter() const;

    std::unordered_map<std::string, transition_config> const&  get_transitions() const { return TRANSITIONS; };
    motion_object_config const&  get_motion_object_config() const { return MOTION_OBJECT_CONFIG; }

    void  apply_effects(float_32_bit const  time_step_in_seconds);
    void  update_time(float_32_bit const  time_step_in_seconds);
    void  update_skeleton_sync();
    void  update_look_at(float_32_bit const  time_step_in_seconds, transition_info const* const  info_ptr = nullptr);
    void  update_aim_at(float_32_bit const  time_step_in_seconds, transition_info const* const  info_ptr = nullptr);
    void  update_animation(float_32_bit const  time_step_in_seconds);

    bool  is_guard_valid() const;

    bool  collect_transition_info(agent_action const* const  from_action_ptr, transition_info&  info) const;
    virtual void  get_colliders_to_be_ignored_in_empty_space_check(std::unordered_set<com::object_guid>&  ignored_collider_guids) const;

    virtual void  on_transition(agent_action* const  from_action_ptr, transition_info const* const  info_ptr);
    virtual void  next_round(float_32_bit const  time_step_in_seconds);

    using  on_custom_folder_erase_func = std::function<void()>;
    virtual void  get_custom_folders(std::unordered_map<com::object_guid, on_custom_folder_erase_func>&  folders) {}

protected:
    bool  collect_other_entiry_folder_guid(agent_action const* const  from_action_ptr, transition_info&  info) const;
    void  collect_motion_object_relocation_frame(agent_action const* const  from_action_ptr, transition_info&  info) const;
    bool  is_empty_space(
            angeo::coordinate_system const&  frame_in_world_space,
            vector3 const&  aabb_half_size,
            angeo::COLLISION_SHAPE_TYPE const  shape_type,
            std::unordered_set<com::object_guid> const&  ignored_collider_guids
            ) const;

    // CONSTANTS

    std::string  NAME;
    desire_config  DESIRE;
    std::vector<effect_config>  EFFECTS;
    std::string  MOTION_TEMPLATE_NAME;
    bool  ONLY_INTERPOLATE_TO_MOTION_TEMPLATE;
    bool  USE_MOTION_TEMPLATE_FOR_LOCATION_INTERPOLATION;
    bool  IS_CYCLIC;
    bool  IS_LOOK_AT_ENABLED;
    bool  IS_AIM_AT_ENABLED;
    bool  DEFINE_SKELETON_SYNC_SOURCE_IN_WORLD_SPACE;
    std::unordered_map<std::string, sensor_config>  SENSORS;
    std::unordered_map<std::string, transition_config>  TRANSITIONS;
    motion_object_config  MOTION_OBJECT_CONFIG; // INVARIANT: for each i, MOTION_OBJECT_CONFIG.aabb_half_size(i) <= AABB_HALF_SIZE(i)
    vector3  AABB_HALF_SIZE; // AABB of the entire agent; it is used for motion object aligment on transition between actions.

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
    float_32_bit  m_end_interpolation_time;
    float_32_bit  m_current_time;

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
    void  on_transition(agent_action* const  from_action_ptr, transition_info const* const  info_ptr) override;
    void  next_round(float_32_bit const  time_step_in_seconds) override;
};


struct  action_roller : public  agent_action
{
    struct  roller_object_config
    {
        bool  operator==(roller_object_config const&  other) const;
        bool  operator!=(roller_object_config const&  other) const { return !(*this == other); }

        float_32_bit  roller_radius;
        float_32_bit  roller_mass_inverted;
    };

    enum struct  ANIMATION_SPEED_SUBJECT : natural_8_bit { ROLLER, MOTION_OBJECT };

    explicit  action_roller(
            std::string const&  name_,
            boost::property_tree::ptree const&  ptree_,
            boost::property_tree::ptree const&  defaults_,
            action_execution_context_ptr const  context_
            );
    void  get_colliders_to_be_ignored_in_empty_space_check(std::unordered_set<com::object_guid>&  ignored_collider_guids) const override;
    void  on_transition(agent_action* const  from_action_ptr, transition_info const* const  info_ptr) override;
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
    angeo::linear_segment_curve  m_desire_move_forward_to_linear_speed;
    angeo::linear_segment_curve  m_desire_move_left_to_linear_speed;
    angeo::linear_segment_curve  m_desire_move_turn_ccw_to_angular_speed;
    angeo::linear_segment_curve  m_angular_speed_to_animation_speed;
    ANIMATION_SPEED_SUBJECT  m_animation_speed_subject;
};


struct  action_controller
{
    action_controller(agent_config const  config, agent*  const  myself);
    ~action_controller();

    void  initialise();

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
