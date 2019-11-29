#ifndef AI_CORTEX_HPP_INCLUDED
#   define AI_CORTEX_HPP_INCLUDED

#   include <ai/blackboard.hpp>
#   include <ai/skeletal_motion_templates.hpp>
#   include <ai/motion_desire_props.hpp>
#   include <angeo/tensor_math.hpp>
#   include <vector>
#   include <string>
#   include <unordered_map>

namespace ai {


struct  cortex
{
    explicit cortex(blackboard_weak_ptr const  blackboard_);
    virtual ~cortex() {}

    blackboard_ptr  get_blackboard() const { return m_blackboard.lock(); }

    // For initialisation steps which cannot be performed/completed in a constructor.
    virtual void  initialise();

    // Any overriden method is required to update the following field in each call:
    //      cortex::m_expression_evaluation_context
    virtual void  next_round(float_32_bit const  time_step_in_seconds);

    // Services for the action controller:

    struct  context
    {
        float_32_bit  time_step_in_seconds;
    };

    motion_desire_props const&  get_motion_desire_props() const { return m_motion_desire_props; }
    natural_32_bit  choose_next_motion_action(std::vector<skeletal_motion_templates::transition_info> const&  possibilities, context const&  ctx) const;

    // Checks of loaded data

    static void  __check_loaded_guard__(skeletal_motion_templates::property_tree const&  guard, std::string const&  message_prefix);
    static void  __check_loaded_desire__(skeletal_motion_templates::property_tree const&  desire, std::string const&  message_prefix);
    static void  __check_loaded_expression__(skeletal_motion_templates::property_tree const&  expr, std::string const&  message_prefix);

protected:
    void  set_stationary_desire();

    static bool  is_desire_scalar_variable(std::string const&  var_name);
    static bool  is_desire_vector_variable(std::string const&  var_name);
    static bool  vector_variable_has_to_be_normalised(std::string const&  var_name);

    float_32_bit*  get_address_of_desire_scalar_variable(std::string const&  var_name);
    vector3*  get_address_of_desire_vector_variable(std::string const&  var_name);

    float_32_bit  evaluate_scalar_expression(skeletal_motion_templates::property_tree const&  expr, context const&  ctx) const;
    vector3  evaluate_vector_expression(skeletal_motion_templates::property_tree const&  expr, context const&  ctx) const;

    // This filed have to be updated in each call to the method 'next_round', including any of its overrides:
    motion_desire_props  m_motion_desire_props;

private:
    blackboard_weak_ptr  m_blackboard;
};


}

#endif
