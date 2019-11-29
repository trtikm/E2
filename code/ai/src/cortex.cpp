#include <ai/cortex.hpp>
#include <ai/action_controller.hpp>
#include <ai/detail/guarded_motion_actions_processor.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


cortex::cortex(blackboard_weak_ptr const  blackboard_)
    : m_blackboard(blackboard_)
    , m_motion_desire_props()
{}


void  cortex::initialise()
{
    set_stationary_desire();
}


void  cortex::next_round(float_32_bit const  time_step_in_seconds)
{
    set_stationary_desire();
}


natural_32_bit  cortex::choose_next_motion_action(std::vector<skeletal_motion_templates::transition_info> const&  possibilities, context const&  ctx) const
{
    TMPROF_BLOCK();

    float_32_bit  best_rank = 0.0f;
    natural_32_bit  best_index = (natural_32_bit)possibilities.size();
    for (natural_32_bit  i = 0U; i != possibilities.size(); ++i)
    {
        skeletal_motion_templates::transition_info const&  info = possibilities.at(i);

        float_32_bit const  guard_valid_cost_addon = info.guard->get_child("valid").get_value<float_32_bit>();
        float_32_bit const  guard_invalid_cost_addon = info.guard->get_child("invalid").get_value<float_32_bit>();

        float_32_bit  rank =
                std::fabs(guard_valid_cost_addon - guard_invalid_cost_addon) > 1e-5f &&
                detail::get_satisfied_motion_guarded_actions(
                        get_blackboard()->m_motion_templates.at(info.cursor.motion_name).actions.at(info.cursor.keyframe_index),
                        get_blackboard()->m_collision_contacts,
                        get_blackboard()->m_action_controller->get_motion_object_motion(),
                        m_motion_desire_props,
                        get_blackboard()->m_action_controller->get_gravity_acceleration(),
                        nullptr
                        )
                ? guard_valid_cost_addon : guard_invalid_cost_addon;

        float_32_bit  error = 0.0f;
        float_32_bit  max_error = 0.0f;
        for (auto const&  null_and_child : *info.desire)
        {
            float_32_bit const  expr_value = evaluate_scalar_expression(null_and_child.second.get_child("expression"), ctx);
            float_32_bit const  ideal_value = null_and_child.second.get_child("value").get_value<float_32_bit>();
            error += std::fabs(ideal_value - expr_value);
            max_error += std::max(std::fabs(1.0f - ideal_value), std::fabs(0.0f - ideal_value));
        }

        if (max_error > 0.0f)
            rank += 1.0f - error / max_error;

        if (best_index == possibilities.size() || best_rank < rank)
        {
            best_rank = rank;
            best_index = i;
        }
    }
    INVARIANT(best_index < possibilities.size());
    return best_index;
}


void  cortex::set_stationary_desire()
{
    detail::rigid_body_motion const& motion = get_blackboard()->m_action_controller->get_motion_object_motion();
    m_motion_desire_props.forward_unit_vector_in_local_space = get_blackboard()->m_motion_templates.directions().forward();
    m_motion_desire_props.linear_velocity_unit_direction_in_local_space = m_motion_desire_props.forward_unit_vector_in_local_space;
    m_motion_desire_props.linear_speed = 0.0f;
    m_motion_desire_props.angular_velocity_unit_axis_in_local_space = get_blackboard()->m_motion_templates.directions().up();
    m_motion_desire_props.angular_speed = 0.0f;
    m_motion_desire_props.look_at_target_in_local_space = 10.0f * get_blackboard()->m_motion_templates.directions().forward();
}


bool  cortex::is_desire_scalar_variable(std::string const&  var_name)
{
    return var_name == "linear_speed" || var_name == "angular_speed";
}


bool  cortex::is_desire_vector_variable(std::string const&  var_name)
{
    return var_name == "forward_unit_vector" ||
           var_name == "linear_velocity_unit_direction" ||
           var_name == "angular_velocity_unit_axis" ||
           var_name == "look_at_target";
}


bool  cortex::vector_variable_has_to_be_normalised(std::string const&  var_name)
{
    return var_name == "look_at_target";
}


float_32_bit*  cortex::get_address_of_desire_scalar_variable(std::string const&  var_name)
{
    if (var_name == "linear_speed")
        return &m_motion_desire_props.linear_speed;
    if (var_name == "angular_speed")
        return &m_motion_desire_props.angular_speed;
    UNREACHABLE();
}


vector3*  cortex::get_address_of_desire_vector_variable(std::string const&  var_name)
{
    if (var_name == "forward_unit_vector")
        return &m_motion_desire_props.forward_unit_vector_in_local_space;
    if (var_name == "linear_velocity_unit_direction")
        return &m_motion_desire_props.linear_velocity_unit_direction_in_local_space;
    if (var_name == "angular_velocity_unit_axis")
        return &m_motion_desire_props.angular_velocity_unit_axis_in_local_space;
    if (var_name == "look_at_target")
        return &m_motion_desire_props.look_at_target_in_local_space;
    UNREACHABLE();
}


float_32_bit  cortex::evaluate_scalar_expression(skeletal_motion_templates::property_tree const&  expr, context const&  ctx) const
{
    if (expr.empty())
    {
        ASSUMPTION(!expr.get_value<std::string>().empty());
        if (expr.get_value<std::string>() == "linear_speed")
            return m_motion_desire_props.linear_speed;
        if (expr.get_value<std::string>() == "angular_speed")
            return m_motion_desire_props.angular_speed;
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


vector3  cortex::evaluate_vector_expression(skeletal_motion_templates::property_tree const&  expr, context const&  ctx) const
{
    if (expr.empty())
    {
        std::string const  name = expr.get_value<std::string>();
        if (name == "forward_unit_vector")
            return m_motion_desire_props.forward_unit_vector_in_local_space;
        if (name == "linear_velocity_unit_direction")
            return m_motion_desire_props.linear_velocity_unit_direction_in_local_space;
        if (name == "angular_velocity_unit_axis")
            return m_motion_desire_props.angular_velocity_unit_axis_in_local_space;
        if (name == "look_at_target")
            return m_motion_desire_props.look_at_target_in_local_space;
        if (name == "forward")
            return get_blackboard()->m_motion_templates.directions().forward();
        if (name == "up")
            return get_blackboard()->m_motion_templates.directions().up();
        if (name == "left")
            return cross_product(get_blackboard()->m_motion_templates.directions().up(), get_blackboard()->m_motion_templates.directions().forward());
        if (name == "back")
            return -get_blackboard()->m_motion_templates.directions().forward();
        if (name == "down")
            return -get_blackboard()->m_motion_templates.directions().up();
        if (name == "right")
            return -cross_product(get_blackboard()->m_motion_templates.directions().up(), get_blackboard()->m_motion_templates.directions().forward());
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


void  cortex::__check_loaded_guard__(skeletal_motion_templates::property_tree const&  guard, std::string const&  message_prefix)
{
    std::string const  prefix = message_prefix + " [cortex/guard]: ";
    if (guard.count("valid") != 1UL)
        throw std::runtime_error(msgstream() << prefix << "The 'guard' does not have exactly 1 'valid' key.");
    if (!guard.get_child("valid").empty())
        throw std::runtime_error(msgstream() << prefix << "The 'valid' key of the 'guard' does not map to a float.");
    if (guard.count("invalid") != 1UL)
        throw std::runtime_error(msgstream() << prefix << "The 'guard' does not have exactly 1 'invalid' key.");
    if (!guard.get_child("invalid").empty())
        throw std::runtime_error(msgstream() << prefix << "The 'invalid' key of the 'guard' does not map to a float.");
    if (guard.size() != 2UL)
        throw std::runtime_error(msgstream() << prefix << "The 'guard' does not have exactly 2 keys: 'valid' and 'invalid'.");
}


void  cortex::__check_loaded_desire__(skeletal_motion_templates::property_tree const&  desire, std::string const&  message_prefix)
{
    std::string const  prefix = message_prefix + " [cortex/desire]: ";
    int i = 0;
    for (auto const&  null_and_child : desire)
    {
        ++i;
        if (null_and_child.second.count("expression") != 1UL)
            throw std::runtime_error(msgstream() << prefix << "The desire rule no. " << i << " does not have exactly 1 'expression' key.");
        __check_loaded_expression__(null_and_child.second.get_child("expression"), prefix);
        if (null_and_child.second.count("value") != 1UL)
            throw std::runtime_error(msgstream() << prefix << "The desire rule no. " << i << " does not have exactly 1 'value' key.");
        if (!null_and_child.second.get_child("value").empty())
            throw std::runtime_error(msgstream() << prefix << "The 'value' key of the desire rule no. " << i << " does not map to a float.");
        if (std::fabs(null_and_child.second.get_child("value").get_value<float_32_bit>()) > 1.0f)
            throw std::runtime_error(msgstream() << prefix << "The 'value' key of the desire rule no. " << i << " is not in the interval [0, 1].");
        if (null_and_child.second.size() != 2UL)
            throw std::runtime_error(msgstream() << prefix << "The desire rule no. " << i << " does not have exactly 2 keys: 'expression' and 'value'.");
    }
}


void  cortex::__check_loaded_expression__(skeletal_motion_templates::property_tree const&  expr, std::string const&  message_prefix)
{
    // Nothing interresting can be checked here.
}


}
