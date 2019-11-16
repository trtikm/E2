#ifndef AI_DETAIL_GUARDED_MOTION_ACTIONS_PROCESSOR_HPP_INCLUDED
#   define AI_DETAIL_GUARDED_MOTION_ACTIONS_PROCESSOR_HPP_INCLUDED

#   include <ai/skeletal_motion_templates.hpp>
#   include <ai/detail/rigid_body_motion.hpp>
#   include <ai/detail/motion_desire_props.hpp>
#   include <ai/blackboard.hpp>
#   include <ai/scene.hpp>
#   include <angeo/tensor_math.hpp>
#   include <string>
#   include <memory>
#   include <vector>
#   include <unordered_set>
#   include <unordered_map>

namespace ai { namespace detail {


struct  motion_action_persistent_data
{
    virtual ~motion_action_persistent_data() {}
    virtual std::unique_ptr<motion_action_persistent_data>  clone() const;
};

using  motion_action_persistent_data_map = std::unordered_map<std::string, std::unique_ptr<motion_action_persistent_data> >;


/// When 'nullptr' is passed to 'output_satisfied_guarded_actions', then the function stops on the 
/// first encounter of a satisfied guard disjunct. In all cases the function returns true if at least
/// one satisfied guard disjunct was found. Otherwise, returns false.
bool  get_satisfied_motion_guarded_actions(
        skeletal_motion_templates::disjunction_of_guarded_actions const&  guarded_actions_to_check,
        blackboard::collision_contacts_map const&  collision_contacts,
        detail::rigid_body_motion const&  motion_object_motion,
        motion_desire_props const&  desire,
        vector3 const&  gravity_acceleration_in_world_space,
        std::vector<skeletal_motion_templates::guarded_actions_ptr>* const  output_satisfied_guarded_actions_ptr
        );

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
        );


struct importance_of_ideal_velocities_to_guarded_actions
{
    importance_of_ideal_velocities_to_guarded_actions();

    void  normalise_sum_of_importances_to_range_01();
    float_32_bit  sum() const;

    // All members are greater or equal to 0.0; where 0.0 means no desire at all.
    float_32_bit  linear;
    float_32_bit  angular;
};


void  compute_importance_of_ideal_velocities_to_guarded_actions(
        std::vector<skeletal_motion_templates::guarded_actions_ptr> const&  guarded_actions_to_check,
        importance_of_ideal_velocities_to_guarded_actions&  importances
        );


std::unordered_set<std::string> const&  get_all_action_unique_names();


float_32_bit  get_stationary_rank(std::string const&  action_unique_name);
float_32_bit  get_stationary_rank(skeletal_motion_templates::action const&  action);
float_32_bit  get_stationary_rank(skeletal_motion_templates::disjunction_of_guarded_actions const&  guarded_actions);


}}

#endif
