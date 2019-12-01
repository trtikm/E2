#ifndef AI_DETAIL_EXPRESSION_EVALUATOR_HPP_INCLUDED
#   define AI_DETAIL_EXPRESSION_EVALUATOR_HPP_INCLUDED

#   include <ai/skeletal_motion_templates.hpp>
#   include <ai/motion_desire_props.hpp>
#   include <angeo/tensor_math.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <string>

namespace ai { namespace detail { namespace eval {


struct  context
{
    motion_desire_props const*  desire_props_ptr;
    skeletal_motion_templates::anim_space_directions  directions;
    float_32_bit  time_step_in_seconds;
};


bool  is_desire_scalar_variable(std::string const&  var_name);
bool  is_desire_vector_variable(std::string const&  var_name);
bool  vector_variable_has_to_be_normalised(std::string const&  var_name);

float_32_bit*  get_address_of_desire_scalar_variable(std::string const&  var_name, motion_desire_props&  desire_props);
vector3*  get_address_of_desire_vector_variable(std::string const&  var_name, motion_desire_props&  desire_props);

float_32_bit  evaluate_scalar_expression(skeletal_motion_templates::property_tree const&  expr, context const&  ctx);
vector3  evaluate_vector_expression(skeletal_motion_templates::property_tree const&  expr, context const&  ctx);


void  __check_loaded_expression__(skeletal_motion_templates::property_tree const&  expr, std::string const&  message_prefix);


}}}

#endif
