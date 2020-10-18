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
#   include <com/object_guid.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <string>
#   include <unordered_map>
#   include <vector>
#   include <memory>

namespace ai {


struct  action_execution_context
{
    action_execution_context(
            agent_state_variables_ptr  state_variables_,
            skeletal_motion_templates  motion_templates_,
            scene_binding_ptr  binding_
            );

    agent_state_variables_ptr  state_variables;
    skeletal_motion_templates  motion_templates;
    skeleton_interpolator_animation  animate;
    skeleton_interpolator_look_at  look_at;
    skeleton_interpolator_aim_at  aim_at;
    scene_binding_ptr  binding;
    float_32_bit  time_buffer;
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

    agent_state_variables&  state_variables() const { return *m_context->state_variables; }
    skeletal_motion_templates  motion_templates() const { return m_context->motion_templates; }
    scene_binding const&  binding() const { return *m_context->binding; }
    com::simulation_context const&  ctx() const { return *m_context->binding->context; }

    bool  is_guard_valid() const;
    float_32_bit  compute_desire_penalty(motion_desire_props const&  props) const;

    bool  is_cyclic() const { return IS_CYCLIC; }
    bool  is_complete() const;
    bool  use_ghost_object_for_skeleton_location() const { return USE_GHOST_OBJECT_FOR_SKELETON_LOCATION; }

    bool  interpolation_parameter() const;

    std::vector<std::string> const&  get_successor_action_names() const { return TRANSITIONS; };
    motion_object_config const&  get_motion_object_config() const { return MOTION_OBJECT_CONFIG; }

    com::object_guid  get_frame_guid_of_skeleton_location() const;

    void  apply_effects(float_32_bit const  time_step_in_seconds);
    void  update_time(float_32_bit const  time_step_in_seconds);
    void  update_ghost_object_frame();
    void  update_animation(float_32_bit const  time_step_in_seconds);

    virtual void  on_transition(agent_action* const  from_action_ptr);
    virtual void  next_round(float_32_bit const  time_step_in_seconds);

    virtual std::unordered_set<com::object_guid>  get_motion_object_collider_guids() const { return {}; }

private:
    void  load_desire(boost::property_tree::ptree const&  ptree, boost::property_tree::ptree const&  defaults);
    void  load_effects(boost::property_tree::ptree const&  ptree, boost::property_tree::ptree const&  defaults);
    void  load_motion_object_config(boost::property_tree::ptree const&  ptree, boost::property_tree::ptree const&  defaults_);

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
    std::vector<std::string>  TRANSITIONS;
    motion_object_config  MOTION_OBJECT_CONFIG;

    // MUTABLE DATA

    action_execution_context_ptr  m_context;

    float_32_bit  m_start_time;
    float_32_bit  m_end_time;
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
    void  on_transition(agent_action* const  from_action_ptr) override;
    void  next_round(float_32_bit const  time_step_in_seconds) override;
    std::unordered_set<com::object_guid>  get_motion_object_collider_guids() const override;
};


//struct  action_interpolate : public  agent_action
//{
//    explicit  action_interpolate(
//            std::string const&  name_,
//            boost::property_tree::ptree const&  ptree_,
//            boost::property_tree::ptree const&  defaults_,
//            action_execution_context_ptr const  context_
//            );
//    //void  on_transition(agent_action* const  from_action_ptr) override {}
//    void  next_round(float_32_bit const  time_step_in_seconds) override;
//    angeo::coordinate_system  target;
//};
//
//
//struct  action_roller : public  agent_action
//{
//    explicit  action_roller(
//            std::string const&  name_,
//            boost::property_tree::ptree const&  ptree_,
//            boost::property_tree::ptree const&  defaults_,
//            action_execution_context_ptr const  context_
//            );
//    //void  on_transition(agent_action* const  from_action_ptr) override {}
//    //void  next_round(float_32_bit const  time_step_in_seconds) override {}
//};


struct  action_controller
{
    action_controller(
            agent_config const  config,
            agent_state_variables_ptr const  state_variables,
            skeletal_motion_templates const  motion_templates,
            scene_binding_ptr const  binding
            );
    ~action_controller();

    void  next_round(
            float_32_bit const  time_step_in_seconds,
            motion_desire_props const&  desire
            );

    std::unordered_set<com::object_guid>  get_motion_object_collider_guids() const
    { return m_current_action->get_motion_object_collider_guids(); }

private:
    action_execution_context_ptr  m_context;
    std::shared_ptr<agent_action>  m_current_action;
    std::unordered_map<std::string, std::shared_ptr<agent_action> >  m_available_actions;
};


using  action_controller_ptr = std::shared_ptr<action_controller>;
using  action_controller_const_ptr = std::shared_ptr<action_controller const>;


}

#endif
