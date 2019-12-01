#include <ai/detail/expression_evaluator.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai { namespace detail { namespace eval {


bool  is_desire_scalar_variable(std::string const&  var_name)
{
    return var_name == "linear_speed" || var_name == "angular_speed";
}


bool  is_desire_vector_variable(std::string const&  var_name)
{
    return var_name == "forward_unit_vector" ||
           var_name == "linear_velocity_unit_direction" ||
           var_name == "angular_velocity_unit_axis" ||
           var_name == "look_at_target";
}


bool  vector_variable_has_to_be_normalised(std::string const&  var_name)
{
    return var_name == "look_at_target";
}


float_32_bit*  get_address_of_desire_scalar_variable(std::string const&  var_name, motion_desire_props&  desire_props)
{
    if (var_name == "linear_speed")
        return &desire_props.linear_speed;
    if (var_name == "angular_speed")
        return &desire_props.angular_speed;
    UNREACHABLE();
}


vector3*  get_address_of_desire_vector_variable(std::string const&  var_name, motion_desire_props&  desire_props)
{
    if (var_name == "forward_unit_vector")
        return &desire_props.forward_unit_vector_in_local_space;
    if (var_name == "linear_velocity_unit_direction")
        return &desire_props.linear_velocity_unit_direction_in_local_space;
    if (var_name == "angular_velocity_unit_axis")
        return &desire_props.angular_velocity_unit_axis_in_local_space;
    if (var_name == "look_at_target")
        return &desire_props.look_at_target_in_local_space;
    UNREACHABLE();
}


float_32_bit  evaluate_scalar_expression(skeletal_motion_templates::property_tree const&  expr, context const&  ctx)
{
    if (expr.empty())
    {
        ASSUMPTION(!expr.get_value<std::string>().empty());
        if (expr.get_value<std::string>() == "linear_speed")
            return ctx.desire_props_ptr->linear_speed;
        if (expr.get_value<std::string>() == "angular_speed")
            return ctx.desire_props_ptr->angular_speed;
        if (expr.get_value<std::string>() == "dt")
            return ctx.time_step_in_seconds;
        if (expr.get_value<std::string>() == "pi")
            return PI();
        if (expr.get_value<std::string>() == "pi_2")
            return PI() / 2.0f;
        if (expr.get_value<std::string>() == "pi_4")
            return PI() / 4.0f;
        return expr.get_value<float_32_bit>();
    }
    else
    {
        std::string const  op = expr.begin()->second.get_value<std::string>();
        std::vector<skeletal_motion_templates::property_tree const*>  args;
        for (auto  it = std::next(expr.begin()); it != expr.end(); ++it)
            args.push_back(&it->second);
        if (op == "+")
        {
            if (args.size() == 2UL)
                return evaluate_scalar_expression(*args.front(), ctx) + evaluate_scalar_expression(*args.back(), ctx);
            ASSUMPTION(args.size() == 1UL);
            return evaluate_scalar_expression(*args.front(), ctx);
        }
        if (op == "-")
        {
            if (args.size() == 2UL)
                return evaluate_scalar_expression(*args.front(), ctx) - evaluate_scalar_expression(*args.back(), ctx);
            ASSUMPTION(args.size() == 1UL);
            return -evaluate_scalar_expression(*args.front(), ctx);
        }
        if (op == "*")
        {
            ASSUMPTION(args.size() == 2UL);
            return evaluate_scalar_expression(*args.front(), ctx) * evaluate_scalar_expression(*args.back(), ctx);
        }
        if (op == "/")
        {
            ASSUMPTION(args.size() == 2UL);
            float_32_bit const  denom = evaluate_scalar_expression(*args.back(), ctx);
            ASSUMPTION(std::fabs(denom) > 1e-5f);
            return evaluate_scalar_expression(*args.front(), ctx) / denom;
        }
        if (op == "sign")
        {
            ASSUMPTION(args.size() == 1UL);
            return evaluate_scalar_expression(*args.front(), ctx) >= 0.0f ? 1.0f : -1.0f;
        }
        if (op == "dot")
        {
            ASSUMPTION(args.size() == 2UL);
            return dot_product(evaluate_vector_expression(*args.front(), ctx), evaluate_vector_expression(*args.back(), ctx));
        }
        if (op == "length")
        {
            ASSUMPTION(args.size() == 1UL);
            return length(evaluate_vector_expression(*args.front(), ctx));
        }
        if (op == "angle")
        {
            ASSUMPTION(args.size() == 2UL);
            return angle(evaluate_vector_expression(*args.front(), ctx), evaluate_vector_expression(*args.back(), ctx));
        }
        if (op == "angle01")
        {
            ASSUMPTION(args.size() == 2UL);
            return angle(evaluate_vector_expression(*args.front(), ctx), evaluate_vector_expression(*args.back(), ctx)) / PI();
        }
        UNREACHABLE();
    }
}


vector3  evaluate_vector_expression(skeletal_motion_templates::property_tree const&  expr, context const&  ctx)
{
    if (expr.empty())
    {
        std::string const  name = expr.get_value<std::string>();
        if (name == "forward_unit_vector")
            return ctx.desire_props_ptr->forward_unit_vector_in_local_space;
        if (name == "linear_velocity_unit_direction")
            return ctx.desire_props_ptr->linear_velocity_unit_direction_in_local_space;
        if (name == "angular_velocity_unit_axis")
            return ctx.desire_props_ptr->angular_velocity_unit_axis_in_local_space;
        if (name == "look_at_target")
            return ctx.desire_props_ptr->look_at_target_in_local_space;
        if (name == "forward")
            return ctx.directions.forward();
        if (name == "up")
            return ctx.directions.up();
        if (name == "left")
            return cross_product(ctx.directions.up(), ctx.directions.forward());
        if (name == "back")
            return -ctx.directions.forward();
        if (name == "down")
            return -ctx.directions.up();
        if (name == "right")
            return -cross_product(ctx.directions.up(), ctx.directions.forward());
        if (name == "zero")
            return vector3_zero();
        UNREACHABLE();
    }
    else
    {
        std::string const  op = expr.begin()->second.get_value<std::string>();
        std::vector<skeletal_motion_templates::property_tree const*>  args;
        for (auto it = std::next(expr.begin()); it != expr.end(); ++it)
            args.push_back(&it->second);
        if (op == "vec")
        {
            ASSUMPTION(args.size() == 3UL);
            return vector3(args.at(0)->get_value<float_32_bit>(), args.at(1)->get_value<float_32_bit>(), args.at(2)->get_value<float_32_bit>());
        }
        if (op == "neg")
        {
            ASSUMPTION(args.size() == 1UL);
            return -evaluate_vector_expression(*args.front(), ctx);
        }
        if (op == "add")
        {
            ASSUMPTION(args.size() == 2UL);
            return evaluate_vector_expression(*args.front(), ctx) + evaluate_vector_expression(*args.back(), ctx);
        }
        if (op == "sub")
        {
            ASSUMPTION(args.size() == 2UL);
            return evaluate_vector_expression(*args.front(), ctx) - evaluate_vector_expression(*args.back(), ctx);
        }
        if (op == "scale")
        {
            ASSUMPTION(args.size() == 2UL);
            return evaluate_scalar_expression(*args.front(), ctx) * evaluate_vector_expression(*args.back(), ctx);
        }
        if (op == "cross")
        {
            ASSUMPTION(args.size() == 2UL);
            return cross_product(evaluate_vector_expression(*args.front(), ctx), evaluate_vector_expression(*args.back(), ctx));
        }
        if (op == "normalised")
        {
            ASSUMPTION(args.size() == 1UL);
            vector3 const  v = evaluate_vector_expression(*args.front(), ctx);
            float_32_bit const  len = length(v);
            if (len > 1e-5f)
                return vector3_unit_y();
            return (1.0f / len) * v;
        }
        if (op == "rotate")
        {
            ASSUMPTION(args.size() == 3UL);
            matrix33 const  R = quaternion_to_rotation_matrix(angle_axis_to_quaternion(evaluate_scalar_expression(*args.back(), ctx),
                                                                                       evaluate_vector_expression(*args.at(1), ctx)));
            return R * evaluate_vector_expression(*args.front(), ctx);
        }
        if (op == "orthogonal_component")
        {
            ASSUMPTION(args.size() == 2UL);
            vector3 const  u = evaluate_vector_expression(*args.front(), ctx);
            return u - project_to_vector(u, evaluate_vector_expression(*args.back(), ctx));
        }
        if (op == "rotate_towards")
        {
            ASSUMPTION(args.size() == 3UL);
            vector3 const  source = evaluate_vector_expression(*args.front(), ctx);
            vector3 const  target = evaluate_vector_expression(*args.at(1), ctx);
            float_32_bit const  max_angle = evaluate_scalar_expression(*args.back(), ctx);
            float_32_bit const  current_angle = angle(source, target);
            if (current_angle <= max_angle)
                return target;
            vector3  axis = cross_product(source, target);
            float_32_bit  len = length(axis);
            if (len < 1e-5f)
            {
                axis = orthogonal(source);
                len = length(axis);
            }
            axis = (1.0f / len) * axis;
            return quaternion_to_rotation_matrix(angle_axis_to_quaternion(max_angle, axis)) * source;
        }
        UNREACHABLE();
    }
}


void  __check_loaded_expression__(skeletal_motion_templates::property_tree const&  expr, std::string const&  message_prefix)
{
    // Nothing interresting can be checked here.
}


}}}
