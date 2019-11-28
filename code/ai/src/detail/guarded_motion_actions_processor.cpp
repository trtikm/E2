#include <ai/detail/guarded_motion_actions_processor.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>

namespace ai { namespace detail {


std::unique_ptr<motion_action_persistent_data>  motion_action_persistent_data::clone() const { UNREACHABLE(); }


struct  motion_action_persistent_data__dont_move : public  motion_action_persistent_data
{
    explicit  motion_action_persistent_data__dont_move(vector3 const& position_)
        : position(position_)
    {}

    std::unique_ptr<motion_action_persistent_data>  clone() const override
    { return std::make_unique<motion_action_persistent_data__dont_move>(*this); }

    vector3  position;
};


template<typename motion_action_data__type, typename... arg_types_for_creation>
motion_action_data__type& copy_or_create_motion_data_and_get_reference(
        motion_action_persistent_data_map&  output_motion_action_data,
        motion_action_persistent_data_map const&  motion_action_data,
        std::string const&  motion_action_name,
        arg_types_for_creation...  args_for_creation
        )
{
    TMPROF_BLOCK();
    motion_action_persistent_data_map::iterator  output_data_it;
    {
        auto  data_it = motion_action_data.find(motion_action_name);
        if (data_it == motion_action_data.end())
            output_data_it = output_motion_action_data.insert({
                    motion_action_name,
                    std::make_unique<motion_action_data__type>(args_for_creation...)
                }).first;
        else
            output_data_it = output_motion_action_data.insert({ motion_action_name, data_it->second->clone() }).first;
    }
    motion_action_data__type* const  data_ptr = dynamic_cast<motion_action_data__type*>(output_data_it->second.get());
    INVARIANT(data_ptr != nullptr);
    return *data_ptr;
}


void  clone_motion_action_data_map(
        motion_action_persistent_data_map const&  src,
        motion_action_persistent_data_map&  dst
        )
{
    for (auto const& elem : src)
        dst.insert({ elem.first, elem.second->clone() });
}


bool  get_satisfied_motion_guarded_actions(
        std::vector<skeletal_motion_templates::guarded_actions_ptr> const&  guarded_actions_to_check,
        blackboard::collision_contacts_map const&  collision_contacts,
        detail::rigid_body_motion const&  motion_object_motion,
        motion_desire_props const&  desire,
        vector3 const&  gravity_acceleration_in_world_space,
        std::vector<skeletal_motion_templates::guarded_actions_ptr>* const  output_satisfied_guarded_actions_ptr
        )
{
    TMPROF_BLOCK();

    matrix44 W;
    angeo::from_base_matrix(motion_object_motion.frame, W);
    auto const  check_constraint = [&collision_contacts, &motion_object_motion, &desire, &gravity_acceleration_in_world_space, &W]
        (skeletal_motion_templates::constraint_ptr const  any_constraint_ptr) -> bool {
            if (auto constraint_ptr =
                    std::dynamic_pointer_cast<skeletal_motion_templates::constraint_contact_normal_cone const>(any_constraint_ptr))
            {
                vector3 const  cone_unit_axis_vector_in_world_space = transform_vector(constraint_ptr->unit_axis, W);
                auto const  begin_and_end = collision_contacts.equal_range(motion_object_motion.nid);
                for (auto  it = begin_and_end.first; it != begin_and_end.second; ++it)
                    if (it->second.normal_force_magnitude > 0.001f &&
                        angle(it->second.unit_normal, cone_unit_axis_vector_in_world_space) < constraint_ptr->angle_in_radians)
                    {
                        return true;
                    }
            }
            else if (auto constraint_ptr =
                    std::dynamic_pointer_cast<skeletal_motion_templates::constraint_has_any_contact const>(any_constraint_ptr))
            {
                auto const  begin_and_end = collision_contacts.equal_range(motion_object_motion.nid);
                if (begin_and_end.first != begin_and_end.second)
                {
                    return true;
                }
            }
            else if (auto constraint_ptr =
                    std::dynamic_pointer_cast<skeletal_motion_templates::constraint_linear_velocity_in_falling_cone const>(any_constraint_ptr))
            {
                float_32_bit const  linear_speed = length(motion_object_motion.velocity.m_linear);
                if (linear_speed < constraint_ptr->min_linear_speed)
                    return false;
                float_32_bit const  gravity_accel = length(gravity_acceleration_in_world_space);
                if (gravity_accel < 0.001f)
                    return false;
                return angle(motion_object_motion.velocity.m_linear, gravity_acceleration_in_world_space) <= constraint_ptr->cone_angle_in_radians;
            }
            else if (auto constraint_ptr =
                    std::dynamic_pointer_cast<skeletal_motion_templates::constraint_is_falling_linear_velocity const>(any_constraint_ptr))
            {
                vector3 const  falling_velocity = project_to_vector(motion_object_motion.velocity.m_linear, gravity_acceleration_in_world_space);
                float_32_bit const  falling_speed = length(falling_velocity);
                return falling_speed >= constraint_ptr->min_falling_speed;
            }
            else if (auto constraint_ptr =
                    std::dynamic_pointer_cast<skeletal_motion_templates::constraint_desired_forward_vector_inside_cone const>(any_constraint_ptr))
            {
                return angle(constraint_ptr->unit_axis, desire.forward_unit_vector_in_local_space) <= constraint_ptr->angle_in_radians;
            }
            else if (auto constraint_ptr =
                std::dynamic_pointer_cast<skeletal_motion_templates::constraint_always const>(any_constraint_ptr))
            {
                return true;
            }
            else
                NOT_IMPLEMENTED_YET();

            return false;
        };

    for (skeletal_motion_templates::guarded_actions_ptr  guarded_actions : guarded_actions_to_check)
    {
        bool  satisfied = true;
        for (skeletal_motion_templates::constraint_ptr const  any_constraint_ptr : guarded_actions->predicates_positive)
            if (check_constraint(any_constraint_ptr) == false)
            {
                satisfied = false;
                break;
            }
        if (satisfied == false)
            continue;
        for (skeletal_motion_templates::constraint_ptr const  any_constraint_ptr : guarded_actions->predicates_negative)
            if (check_constraint(any_constraint_ptr) == true)
            {
                satisfied = false;
                break;
            }
        if (satisfied == true)
            if (output_satisfied_guarded_actions_ptr == nullptr)
                return true;
            else
                output_satisfied_guarded_actions_ptr->push_back(guarded_actions);
    }

    return output_satisfied_guarded_actions_ptr == nullptr ? false : !output_satisfied_guarded_actions_ptr->empty();
}


void  execute_satisfied_motion_guarded_actions(
        std::vector<skeletal_motion_templates::guarded_actions_ptr> const&  satisfied_guarded_actions,
        float_32_bit const  time_step_in_seconds,
        vector3 const&  ideal_linear_velocity_in_world_space,
        vector3 const&  ideal_angular_velocity_in_world_space,
        vector3 const&  gravity_acceleration_in_world_space,
        motion_desire_props const&  desire,
        motion_action_persistent_data_map const&  motion_action_data,
        rigid_body_motion&  motion_object_motion,
        motion_action_persistent_data_map& output_motion_action_data    // can alias with 'motion_action_data'
        )
{
    TMPROF_BLOCK();

    motion_action_persistent_data_map  new_motion_action_data;

    matrix44 W;
    angeo::from_base_matrix(motion_object_motion.frame, W);
    for (auto const actions_ptr : satisfied_guarded_actions)
        for (auto const action_props : actions_ptr->actions)
            if (action_props == nullptr || std::dynamic_pointer_cast<skeletal_motion_templates::action_none const>(action_props) != nullptr)
                continue;
            else if (auto const  action_ptr =
                std::dynamic_pointer_cast<skeletal_motion_templates::action_move_forward_with_ideal_speed const>(action_props))
            {
                float_32_bit const  ideal_linear_speed = length(ideal_linear_velocity_in_world_space);
                vector3  agent_linear_acceleration =
                        (ideal_linear_speed * motion_object_motion.forward - motion_object_motion.velocity.m_linear) / time_step_in_seconds;
                float_32_bit const  agent_linear_acceleration_magnitude = length(agent_linear_acceleration);
                if (agent_linear_acceleration_magnitude > action_ptr->max_linear_accel)
                    agent_linear_acceleration *= action_ptr->max_linear_accel / agent_linear_acceleration_magnitude;
                motion_object_motion.acceleration.m_linear += agent_linear_acceleration;
            }
            else if (auto const  action_ptr =
                std::dynamic_pointer_cast<skeletal_motion_templates::action_rotate_forward_vector_towards_desired_linear_velocity const>(action_props))
            {
                float_32_bit const  ideal_linear_speed = length(ideal_linear_velocity_in_world_space);
                float_32_bit const  max_anglular_speed = 
                        action_ptr->max_angular_speed * std::min(1.0f, std::max(0.0f, ideal_linear_speed / std::max(action_ptr->min_linear_speed, 0.001f)));
    
                float_32_bit const  rot_angle =
                        angeo::compute_rotation_angle(
                                motion_object_motion.up,
                                motion_object_motion.forward,
                                transform_vector(desire.linear_velocity_unit_direction_in_local_space, W)
                                );
            
                float_32_bit  desired_angular_velocity_magnitude = rot_angle / time_step_in_seconds;
                if (desired_angular_velocity_magnitude > max_anglular_speed)
                    desired_angular_velocity_magnitude = max_anglular_speed;
                else if (desired_angular_velocity_magnitude < -max_anglular_speed)
                    desired_angular_velocity_magnitude = -max_anglular_speed;
    
                vector3 const  desired_angular_velocity = desired_angular_velocity_magnitude * motion_object_motion.up;
    
                vector3  agent_angular_acceleration =
                        (desired_angular_velocity - motion_object_motion.velocity.m_angular) / time_step_in_seconds;
                float_32_bit const  agent_angular_acceleration_magnitude = length(agent_angular_acceleration);
                if (agent_angular_acceleration_magnitude > action_ptr->max_angular_accel)
                    agent_angular_acceleration *= action_ptr->max_angular_accel / agent_angular_acceleration_magnitude;
                motion_object_motion.acceleration.m_angular += agent_angular_acceleration;
            }
            else if (auto const  action_ptr =
                std::dynamic_pointer_cast<skeletal_motion_templates::action_turn_around const>(action_props))
            {
                vector3  agent_angular_acceleration =
                    (ideal_angular_velocity_in_world_space - motion_object_motion.velocity.m_angular) / time_step_in_seconds;
                float_32_bit const  agent_angular_acceleration_magnitude = length(agent_angular_acceleration);
                if (agent_angular_acceleration_magnitude > action_ptr->max_angular_accel)
                    agent_angular_acceleration *= action_ptr->max_angular_accel / agent_angular_acceleration_magnitude;
                motion_object_motion.acceleration.m_angular += agent_angular_acceleration;
            }
            else if (auto const  action_ptr =
                std::dynamic_pointer_cast<skeletal_motion_templates::action_dont_move const>(action_props))
            {
                motion_action_persistent_data__dont_move&  action_data_ref =
                        copy_or_create_motion_data_and_get_reference<motion_action_persistent_data__dont_move>(
                                new_motion_action_data,
                                motion_action_data,
                                typeid(*action_ptr).name(),
                                motion_object_motion.frame.origin()
                                );
    
                if (length(action_data_ref.position - motion_object_motion.frame.origin()) > action_ptr->radius)
                    action_data_ref.position = motion_object_motion.frame.origin();
    
                // TODO: The computaion of 'dt' on the next line looks suspicious. RE-EVALUATE!
                float_32_bit const  dt = std::max(1.0f / 25.0f, time_step_in_seconds);
                vector3 const  next_motion_object_origin =
                        motion_object_motion.frame.origin() +
                        dt * motion_object_motion.velocity.m_linear +
                        (0.5f * dt * dt) * motion_object_motion.acceleration.m_linear
                        ;
                vector3 const  position_delta = action_data_ref.position - next_motion_object_origin;
                vector3  agent_linear_acceleration = (1.0f / (dt * dt)) * position_delta;
                float_32_bit const  agent_linear_acceleration_magnitude = length(agent_linear_acceleration);
                if (agent_linear_acceleration_magnitude > action_ptr->max_linear_accel)
                    agent_linear_acceleration *= action_ptr->max_linear_accel / agent_linear_acceleration_magnitude;
                motion_object_motion.acceleration.m_linear += agent_linear_acceleration;
            }
            else if (auto const  action_ptr =
                std::dynamic_pointer_cast<skeletal_motion_templates::action_dont_rotate const>(action_props))
            {
                vector3 const  angular_velocity_to_cancel_in_world_space =
                        dot_product(motion_object_motion.up, motion_object_motion.velocity.m_angular) * motion_object_motion.up;
    
                vector3  agent_angular_acceleration = -angular_velocity_to_cancel_in_world_space / time_step_in_seconds;
                float_32_bit const  agent_angular_acceleration_magnitude = length(agent_angular_acceleration);
                if (agent_angular_acceleration_magnitude > action_ptr->max_angular_accel)
                    agent_angular_acceleration *= action_ptr->max_angular_accel / agent_angular_acceleration_magnitude;
                motion_object_motion.acceleration.m_angular += agent_angular_acceleration;
            }
            else if (auto const  action_ptr =
                std::dynamic_pointer_cast<skeletal_motion_templates::action_set_linear_velocity const>(action_props))
            {
                vector3 const  target_linear_velocity_in_world_space = 
                        action_ptr->linear_velocity(0) * cross_product(motion_object_motion.forward, motion_object_motion.up) +
                        action_ptr->linear_velocity(1) * motion_object_motion.forward +
                        action_ptr->linear_velocity(2) * motion_object_motion.up
                        ;
                vector3  agent_linear_acceleration =
                        (target_linear_velocity_in_world_space - motion_object_motion.velocity.m_linear) / time_step_in_seconds;
                float_32_bit const  agent_linear_acceleration_magnitude = length(agent_linear_acceleration);
                if (agent_linear_acceleration_magnitude > action_ptr->max_linear_accel)
                    agent_linear_acceleration *= action_ptr->max_linear_accel / agent_linear_acceleration_magnitude;
                motion_object_motion.acceleration.m_linear += agent_linear_acceleration;
            }
            else if (auto const  action_ptr =
                std::dynamic_pointer_cast<skeletal_motion_templates::action_set_angular_velocity const>(action_props))
            {
                vector3 const  target_angular_velocity_in_world_space = 
                        action_ptr->angular_velocity(0) * cross_product(motion_object_motion.forward, motion_object_motion.up) +
                        action_ptr->angular_velocity(1) * motion_object_motion.forward +
                        action_ptr->angular_velocity(2) * motion_object_motion.up
                        ;
                vector3  agent_angular_acceleration =
                        (target_angular_velocity_in_world_space - motion_object_motion.velocity.m_angular) / time_step_in_seconds;
                float_32_bit const  agent_angular_acceleration_magnitude = length(agent_angular_acceleration);
                if (agent_angular_acceleration_magnitude > action_ptr->max_angular_accel)
                    agent_angular_acceleration *= action_ptr->max_angular_accel / agent_angular_acceleration_magnitude;
                motion_object_motion.acceleration.m_angular += agent_angular_acceleration;
            }
            else if (auto const  action_ptr =
                std::dynamic_pointer_cast<skeletal_motion_templates::action_cancel_gravity_accel const>(action_props))
            {
                motion_object_motion.acceleration.m_linear -= gravity_acceleration_in_world_space;
            }
            else
                NOT_IMPLEMENTED_YET();
    
    output_motion_action_data.swap(new_motion_action_data);
}


importance_of_ideal_velocities_to_guarded_actions::importance_of_ideal_velocities_to_guarded_actions()
    : linear(0.0f)
    , angular(0.0f)
{}


void  importance_of_ideal_velocities_to_guarded_actions::normalise_sum_of_importances_to_range_01()
{
    float_32_bit const  s = sum();
    if (s > 1.0f)
    {
        linear /= s;
        angular /= s;
    }
}


float_32_bit  importance_of_ideal_velocities_to_guarded_actions::sum() const
{
    return linear + angular;
}


void  compute_importance_of_ideal_velocities_to_guarded_actions(
        std::vector<skeletal_motion_templates::guarded_actions_ptr> const&  guarded_actions_to_check,
        importance_of_ideal_velocities_to_guarded_actions&  importances
        )
{
    TMPROF_BLOCK();

    for (skeletal_motion_templates::guarded_actions_ptr  guarded_actions : guarded_actions_to_check)
        for (auto const action_props : guarded_actions->actions)
            if (action_props == nullptr || std::dynamic_pointer_cast<skeletal_motion_templates::action_none const>(action_props) != nullptr)
                continue;
            else if (auto const  action_ptr =
                std::dynamic_pointer_cast<skeletal_motion_templates::action_move_forward_with_ideal_speed const>(action_props))
            {
                importances.linear += 1.0f;
            }
            else if (auto const  action_ptr =
                std::dynamic_pointer_cast<skeletal_motion_templates::action_rotate_forward_vector_towards_desired_linear_velocity const>(action_props))
            {
                importances.linear += 1.0f;
            }
            else if (auto const  action_ptr =
                std::dynamic_pointer_cast<skeletal_motion_templates::action_turn_around const>(action_props))
            {
                importances.angular += 1.0f;
            }
            else if (auto const  action_ptr =
                std::dynamic_pointer_cast<skeletal_motion_templates::action_dont_move const>(action_props))
            {
                // No imporance here.
            }
            else if (auto const  action_ptr =
                std::dynamic_pointer_cast<skeletal_motion_templates::action_dont_rotate const>(action_props))
            {
                // No imporance here.
            }
            else if (auto const  action_ptr =
                std::dynamic_pointer_cast<skeletal_motion_templates::action_set_linear_velocity const>(action_props))
            {
                // No imporance here.
            }
            else if (auto const  action_ptr =
                std::dynamic_pointer_cast<skeletal_motion_templates::action_set_angular_velocity const>(action_props))
            {
                // No imporance here.
            }
            else if (auto const  action_ptr =
                std::dynamic_pointer_cast<skeletal_motion_templates::action_cancel_gravity_accel const>(action_props))
            {
                // No imporance here.
            }
            else
                NOT_IMPLEMENTED_YET();
}


std::unordered_set<std::string> const&  get_all_action_unique_names()
{
    static std::unordered_set<std::string> const  action_names{
        skeletal_motion_templates::action_none::unique_name,
        skeletal_motion_templates::action_move_forward_with_ideal_speed::unique_name,
        skeletal_motion_templates::action_rotate_forward_vector_towards_desired_linear_velocity::unique_name,
        skeletal_motion_templates::action_turn_around::unique_name,
        skeletal_motion_templates::action_dont_move::unique_name,
        skeletal_motion_templates::action_dont_rotate::unique_name,
        skeletal_motion_templates::action_set_linear_velocity::unique_name,
        skeletal_motion_templates::action_set_angular_velocity::unique_name,
        skeletal_motion_templates::action_cancel_gravity_accel::unique_name,
    };
    return  action_names;
}


}}
