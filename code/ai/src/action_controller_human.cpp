#include <ai/action_controller_human.hpp>
#include <ai/skeleton_utils.hpp>
#include <angeo/skeleton_kinematics.hpp>
#include <scene/scene_node_id.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/std_pair_hash.hpp>
#include <utility/log.hpp>
#include <limits>
#include <queue>
#include <unordered_map>
#include <functional>

namespace ai { namespace detail {


struct  rigid_body_motion
{
    rigid_body_motion()
        : velocity({vector3_zero(),vector3_zero()})
        , acceleration({vector3_zero(),vector3_zero()})
        , inverted_mass(0.0f)
        , inverted_inertia_tensor(matrix33_zero())
    {}

    rigid_body_motion(scene_ptr const  s, scene::node_id const&  motion_object_nid)
        : velocity({ s->get_linear_velocity_of_rigid_body_of_scene_node(motion_object_nid),
                     s->get_angular_velocity_of_rigid_body_of_scene_node(motion_object_nid) })
        , acceleration({ s->get_linear_acceleration_of_rigid_body_of_scene_node(motion_object_nid),
                         s->get_angular_acceleration_of_rigid_body_of_scene_node(motion_object_nid) })
        , inverted_mass(s->get_inverted_mass_of_rigid_body_of_scene_node(motion_object_nid))
        , inverted_inertia_tensor(s->get_inverted_inertia_tensor_of_rigid_body_of_scene_node(motion_object_nid))
    {}

    rigid_body_motion(
            scene_ptr const  s,
            scene::node_id const&  motion_object_nid,
            skeletal_motion_templates::meta_record_real const&  mass_distribution
            )
        : velocity({ s->get_linear_velocity_of_rigid_body_of_scene_node(motion_object_nid),
            s->get_angular_velocity_of_rigid_body_of_scene_node(motion_object_nid) })
        , acceleration({ s->get_linear_acceleration_of_rigid_body_of_scene_node(motion_object_nid),
            s->get_angular_acceleration_of_rigid_body_of_scene_node(motion_object_nid) })
        , inverted_mass()
        , inverted_inertia_tensor()
    {
        set_inverted_mass(mass_distribution);
        set_inverted_inertia_tensor(mass_distribution);
    }

    void  set_linear_acceleration(vector3 const&  linear_acceleration)
    {
        acceleration.m_linear = linear_acceleration;
    }

    void  set_inverted_mass(skeletal_motion_templates::meta_record_real const&  mass_distribution)
    {
        enum INDICES_OF_ARGUMENTS
        {
            INVERTED_MASS           = 0,
            INVERTED_INERTIA_TENSOR = 1,
        };
        inverted_mass = mass_distribution.arguments.at(INVERTED_MASS);
    }

    void  set_inverted_inertia_tensor(skeletal_motion_templates::meta_record_real const&  mass_distribution)
    {
        enum INDICES_OF_ARGUMENTS
        {
            INVERTED_MASS           = 0,
            INVERTED_INERTIA_TENSOR = 1,
        };
        for (int i = 0; i != 3; ++i)
            for (int j = 0; j != 3; ++j)
                inverted_inertia_tensor(i,j) = mass_distribution.arguments.at(INVERTED_INERTIA_TENSOR + 3*i + j);
    }

    void restore(scene_ptr const  s, scene::node_id const&  motion_object_nid)
    {
        s->set_linear_velocity_of_rigid_body_of_scene_node(motion_object_nid, velocity.m_linear);
        s->set_angular_velocity_of_rigid_body_of_scene_node(motion_object_nid, velocity.m_angular);
        s->set_linear_acceleration_of_rigid_body_of_scene_node(motion_object_nid, acceleration.m_linear);
        s->set_angular_acceleration_of_rigid_body_of_scene_node(motion_object_nid, acceleration.m_angular);
        s->set_inverted_mass_of_rigid_body_of_scene_node(motion_object_nid, inverted_mass);
        s->set_inverted_inertia_tensor_of_rigid_body_of_scene_node(motion_object_nid, inverted_inertia_tensor);
    }

    angeo::linear_and_angular_vector  velocity;
    angeo::linear_and_angular_vector  acceleration;
    float_32_bit  inverted_mass;
    matrix33  inverted_inertia_tensor;
};


void  create_collider_and_rigid_body_of_motion_scene_node(
        scene_ptr const  s,
        scene::node_id const&  motion_object_nid,
        skeletal_motion_templates::meta_record_real const&  collider_props,
        rigid_body_motion const&  rb_motion
        )
{
    if (collider_props.keyword == "capsule")
    {
        enum INDICES_OF_ARGUMENTS
        {
            LENGTH = 0,
            RADIUS = 1,
            WEIGHT = 2,
        };
        s->insert_collision_capsule_to_scene_node(
                motion_object_nid,
                collider_props.arguments.at(LENGTH),
                collider_props.arguments.at(RADIUS),
                angeo::COLLISION_MATERIAL_TYPE::NO_FRINCTION_NO_BOUNCING,
                1.0f,
                true
                );
    }
    else
    {
        NOT_IMPLEMENTED_YET();
    }
    s->insert_rigid_body_to_scene_node(
            motion_object_nid,
            rb_motion.velocity.m_linear,
            rb_motion.velocity.m_angular,
            rb_motion.acceleration.m_linear,
            rb_motion.acceleration.m_angular,
            rb_motion.inverted_mass,
            rb_motion.inverted_inertia_tensor
            );
}


void  destroy_collider_and_rigid_bofy_of_motion_scene_node(scene_ptr const  s, scene::node_id const&  motion_object_nid)
{
    s->erase_rigid_body_from_scene_node(motion_object_nid);
    s->erase_collision_object_from_scene_node(motion_object_nid);
}


scene::node_id  get_motion_object_nid(scene_ptr const  s, scene::node_id const  agent_nid)
{
    return s->get_aux_root_node_for_agent(agent_nid, "motion_object");
}


scene::node_id  create_motion_scene_node(
        scene_ptr const  s,
        scene::node_id const&  motion_object_nid,
        angeo::coordinate_system const&  frame_in_world_space,
        skeletal_motion_templates::meta_record_real const&  collider_props,
        skeletal_motion_templates::meta_record_real const&  mass_distribution
        )
{
    s->insert_scene_node(motion_object_nid, frame_in_world_space, false);
    rigid_body_motion  rb_motion;
    rb_motion.set_linear_acceleration(s->get_gravity_acceleration_at_point(frame_in_world_space.origin()));
    rb_motion.set_inverted_mass(mass_distribution);
    rb_motion.set_inverted_inertia_tensor(mass_distribution);
    create_collider_and_rigid_body_of_motion_scene_node(s, motion_object_nid, collider_props, rb_motion);
    return motion_object_nid;
}


void  destroy_motion_scene_node(scene_ptr const  s, scene::node_id const&  motion_object_nid)
{
    destroy_collider_and_rigid_bofy_of_motion_scene_node(s, motion_object_nid);
    s->erase_scene_node(motion_object_nid);
}


template<typename motion_action_data__type, typename... arg_types_for_creation>
motion_action_data__type&  copy_or_create_motion_data_and_get_reference(
        action_controller_human::motion_action_data_map&  output_motion_action_data,
        action_controller_human::motion_action_data_map const&  motion_action_data,
        std::string const&  motion_action_name,
        arg_types_for_creation...  args_for_creation
        )
{
    action_controller_human::motion_action_data_map::iterator  output_data_it;
    {
        auto  data_it = motion_action_data.find(motion_action_name);
        if (data_it == motion_action_data.end())
            output_data_it = output_motion_action_data.insert({
                    motion_action_name,
                    std::make_unique<motion_action_data__type>(args_for_creation...)
                    }).first;
        else
            output_data_it = output_motion_action_data.insert({motion_action_name, data_it->second->clone()}).first;
    }
    motion_action_data__type* const  data_ptr = dynamic_cast<motion_action_data__type*>(output_data_it->second.get());
    INVARIANT(data_ptr != nullptr);
    return *data_ptr;
}


void  clone_motion_action_data_map(
        action_controller_human::motion_action_data_map const&  src,
        action_controller_human::motion_action_data_map&  dst
        )
{
    for (auto const&  elem : src)
        dst.insert({elem.first, elem.second->clone()});
}


struct  motion_action_data__dont_move : public action_controller_human::motion_action_data
{
    explicit  motion_action_data__dont_move(vector3 const&  position_)
        : position(position_)
    {}

    std::unique_ptr<motion_action_data>  clone() const override { return std::make_unique<motion_action_data__dont_move>(*this); }

    vector3  position;
};


void  compute_motion_object_acceleration_from_motion_actions(
        vector3 const&  desired_forward_unit_vector_in_world_space,
        vector3 const&  desired_linear_velocity_unit_direction_in_world_space,
        float_32_bit const  desired_linear_speed,
        //----------------------------------------------
        float_32_bit const  time_step_in_seconds,
        skeletal_motion_templates::keyframes const&  animation,
        natural_32_bit const  current_keyframe_index,
        natural_32_bit const  previous_keyframe_index,
        skeletal_motion_templates::meta_records_real const&  motion_object_action_props,
        vector3 const&  forward_direction_in_anim_space,
        vector3 const&  up_direction_in_anim_space,
        vector3 const&  motion_object_origin,
        vector3 const&  motion_object_forward_direction_in_world_space,
        vector3 const&  motion_object_up_direction_in_world_space,
        vector3 const&  motion_object_linear_velocity_in_world_space,
        vector3 const&  motion_object_angular_velocity_in_world_space,
        vector3 const&  gravity_accel,
        action_controller_human::motion_action_data_map const&  motion_action_data,
        vector3&  output_motion_object_linear_acceleration,
        vector3&  output_motion_object_angular_acceleration,
        action_controller_human::motion_action_data_map&  output_motion_action_data,    // can alias with 'motion_action_data'
        float_32_bit*  output_linear_motion_error_wrt_ideal = nullptr,
        float_32_bit*  output_angular_motion_error_wrt_ideal = nullptr      // can alias with 'output_linear_motion_error_wrt_ideal'
        )
{
    TMPROF_BLOCK();

    float_32_bit const  motion_object_linear_speed = length(motion_object_linear_velocity_in_world_space);
    action_controller_human::motion_action_data_map  new_motion_action_data;
    for (auto const&  action_props : motion_object_action_props.m_records)
        if (action_props.keyword == "none")
            continue;
        else if (action_props.keyword == "accelerate_towards_clipped_desired_linear_velocity")
        {
            enum INDICES_OF_ARGUMENTS
            {
                ANGLE               = 0,
                MAX_LINEAR_ACCEL    = 1,
                MIN_LINEAR_SPEED    = 2,
            };

            float_32_bit  ideal_linear_speed;
            {
                vector3 const  position_delta =
                        animation.get_meta_data().m_reference_frames.at(current_keyframe_index).origin() -
                        animation.get_meta_data().m_reference_frames.at(previous_keyframe_index).origin();
                float_32_bit const  time_delta =
                        animation.keyframe_at(current_keyframe_index).get_time_point() -
                        animation.keyframe_at(previous_keyframe_index).get_time_point() ;
                ideal_linear_speed = length(position_delta) / std::max(time_delta, 0.0001f);
            }

            vector3  clipped_desired_linear_velocity_in_world_space;
            {
                float_32_bit  rot_angle;
                {
                    float_32_bit const  max_rot_angle = 
                            action_props.arguments.at(ANGLE) *
                            std::min(1.0f, std::max(0.0f,
                                motion_object_linear_speed * motion_object_linear_speed / action_props.arguments.at(MIN_LINEAR_SPEED)
                                ))
                            ;

                    float_32_bit const  full_rot_angle =
                            angeo::compute_rotation_angle(
                                    motion_object_up_direction_in_world_space,
                                    motion_object_forward_direction_in_world_space,
                                    desired_linear_velocity_unit_direction_in_world_space
                                    );
                    if (full_rot_angle > max_rot_angle)
                        rot_angle = max_rot_angle;
                    else if (full_rot_angle < -max_rot_angle)
                        rot_angle = -max_rot_angle;
                    else
                        rot_angle = full_rot_angle;
                }
                clipped_desired_linear_velocity_in_world_space =
                        quaternion_to_rotation_matrix(angle_axis_to_quaternion(rot_angle, motion_object_up_direction_in_world_space))
                        * motion_object_forward_direction_in_world_space;

                clipped_desired_linear_velocity_in_world_space *=
                        ideal_linear_speed / length(clipped_desired_linear_velocity_in_world_space);
            }
            vector3  agent_linear_acceleration =
                    (clipped_desired_linear_velocity_in_world_space - motion_object_linear_velocity_in_world_space) / time_step_in_seconds;
            float_32_bit const  agent_linear_acceleration_magnitude = length(agent_linear_acceleration);
            if (agent_linear_acceleration_magnitude > action_props.arguments.at(MAX_LINEAR_ACCEL))
                agent_linear_acceleration *= action_props.arguments.at(MAX_LINEAR_ACCEL) / agent_linear_acceleration_magnitude;
            output_motion_object_linear_acceleration += agent_linear_acceleration;

            if (output_linear_motion_error_wrt_ideal != nullptr)
            {
                float_32_bit const  speed_ratio =
                        ((length(motion_object_linear_velocity_in_world_space) + 0.001f) / (ideal_linear_speed + 0.001f)) - 1.0f;
                if (std::fabs(*output_linear_motion_error_wrt_ideal) < fabs(speed_ratio))
                    *output_linear_motion_error_wrt_ideal = speed_ratio;
            }
        }
        else if (action_props.keyword == "chase_linear_velocity_by_forward_vector")
        {
            enum INDICES_OF_ARGUMENTS
            {
                MAX_ANGULAR_SPEED   = 0,
                MAX_ANGULAR_ACCEL   = 1,
                MIN_LINEAR_SPEED    = 2,
            };

            float_32_bit const  max_anglular_speed = 
                    action_props.arguments.at(MAX_ANGULAR_SPEED) *
                    std::min(1.0f, std::max(0.0f,
                        motion_object_linear_speed * motion_object_linear_speed / action_props.arguments.at(MIN_LINEAR_SPEED)
                        ))
                    ;

            float_32_bit const  rot_angle =
                    angeo::compute_rotation_angle(
                            motion_object_up_direction_in_world_space,
                            motion_object_forward_direction_in_world_space,
                            desired_linear_velocity_unit_direction_in_world_space
                            );
        
            float_32_bit  desired_angular_velocity_magnitude = rot_angle / time_step_in_seconds;
            if (desired_angular_velocity_magnitude > max_anglular_speed)
                desired_angular_velocity_magnitude = max_anglular_speed;
            else if (desired_angular_velocity_magnitude < -max_anglular_speed)
                desired_angular_velocity_magnitude = -max_anglular_speed;

            vector3 const  desired_angular_velocity = desired_angular_velocity_magnitude * motion_object_up_direction_in_world_space;

            vector3  agent_angular_acceleration =
                    (desired_angular_velocity - motion_object_angular_velocity_in_world_space) / time_step_in_seconds;
            float_32_bit const  agent_angular_acceleration_magnitude = length(agent_angular_acceleration);
            if (agent_angular_acceleration_magnitude > action_props.arguments.at(MAX_ANGULAR_ACCEL))
                agent_angular_acceleration *= action_props.arguments.at(MAX_ANGULAR_ACCEL) / agent_angular_acceleration_magnitude;
            output_motion_object_angular_acceleration += agent_angular_acceleration;

            if (output_angular_motion_error_wrt_ideal != nullptr)
            {
                // This action does not have any motion error wrt an ideal one (beacause the is no ideal motion to compare with).
            }
        }
        else if (action_props.keyword == "turn_around")
        {
            enum INDICES_OF_ARGUMENTS
            {
                MAX_ANGULAR_ACCEL = 0,
            };

            float_32_bit  ideal_angular_speed;
            vector3  ideal_angular_velocity;
            {
                matrix44  M;

                angeo::from_base_matrix(animation.get_meta_data().m_reference_frames.at(current_keyframe_index), M);
                vector3 const  current_keyframe_forward_direction_in_world_space = transform_vector(forward_direction_in_anim_space, M);

                angeo::from_base_matrix(animation.get_meta_data().m_reference_frames.at(previous_keyframe_index), M);
                vector3 const  previous_keyframe_forward_direction_in_world_space = transform_vector(forward_direction_in_anim_space, M);

                float_32_bit const  rot_angle =
                        angeo::compute_rotation_angle(
                                motion_object_up_direction_in_world_space,
                                previous_keyframe_forward_direction_in_world_space,
                                current_keyframe_forward_direction_in_world_space
                                );

                float_32_bit const  time_delta =
                        animation.keyframe_at(current_keyframe_index).get_time_point() -
                        animation.keyframe_at(previous_keyframe_index).get_time_point() ;

                ideal_angular_speed = rot_angle / std::max(time_delta, 0.0001f);
                ideal_angular_velocity = ideal_angular_speed * motion_object_up_direction_in_world_space;
            }

            vector3  agent_angular_acceleration =
                (ideal_angular_velocity - motion_object_angular_velocity_in_world_space) / time_step_in_seconds;
            float_32_bit const  agent_angular_acceleration_magnitude = length(agent_angular_acceleration);
            if (agent_angular_acceleration_magnitude > action_props.arguments.at(MAX_ANGULAR_ACCEL))
                agent_angular_acceleration *= action_props.arguments.at(MAX_ANGULAR_ACCEL) / agent_angular_acceleration_magnitude;
            output_motion_object_angular_acceleration += agent_angular_acceleration;

            if (output_angular_motion_error_wrt_ideal != nullptr)
            {
                // TODO!!

                //float_32_bit const  rot_angle =
                //        angeo::compute_rotation_angle(
                //                motion_object_up_direction_in_world_space,
                //                motion_object_forward_direction_in_world_space,
                //                desired_linear_velocity_unit_direction_in_world_space
                //                );
                //float_32_bit const  desired_angular_speed = rot_angle / time_step_in_seconds;
                //vector3 const  desired_angular_velocity = desired_angular_speed * motion_object_up_direction_in_world_space;
            }
        }
        else if (action_props.keyword == "dont_move")
        {
            enum INDICES_OF_ARGUMENTS
            {
                MAX_LINEAR_ACCEL = 0,
            };

            detail::motion_action_data__dont_move&  action_data_ref =
                    copy_or_create_motion_data_and_get_reference<motion_action_data__dont_move>(
                            new_motion_action_data,
                            motion_action_data,
                            action_props.keyword,
                            motion_object_origin
                            );

            vector3 const  sliding_prevention_accel =
                    0.5f * (2.0f * (action_data_ref.position - motion_object_origin) / (time_step_in_seconds * time_step_in_seconds));
            vector3  agent_linear_acceleration =
                    -motion_object_linear_velocity_in_world_space / time_step_in_seconds + sliding_prevention_accel;
            float_32_bit const  agent_linear_acceleration_magnitude = length(agent_linear_acceleration);
            if (agent_linear_acceleration_magnitude > action_props.arguments.at(MAX_LINEAR_ACCEL))
                agent_linear_acceleration *= action_props.arguments.at(MAX_LINEAR_ACCEL) / agent_linear_acceleration_magnitude;
            output_motion_object_linear_acceleration += agent_linear_acceleration;

            if (output_linear_motion_error_wrt_ideal != nullptr)
            {
                float_32_bit const  speed_ratio = length(motion_object_linear_velocity_in_world_space);
                if (std::fabs(*output_linear_motion_error_wrt_ideal) < speed_ratio)
                    *output_linear_motion_error_wrt_ideal = speed_ratio;
            }
        }
        else if (action_props.keyword == "dont_rotate")
        {
            enum INDICES_OF_ARGUMENTS
            {
                MAX_ANGULAR_ACCEL   = 0,
            };

            vector3 const  angular_velocity_to_cancel_in_world_space =
                    dot_product(motion_object_up_direction_in_world_space, motion_object_angular_velocity_in_world_space)
                    * motion_object_up_direction_in_world_space
                    ;

            vector3  agent_angular_acceleration = -angular_velocity_to_cancel_in_world_space / time_step_in_seconds;
            float_32_bit const  agent_angular_acceleration_magnitude = length(agent_angular_acceleration);
            if (agent_angular_acceleration_magnitude > action_props.arguments.at(MAX_ANGULAR_ACCEL))
                agent_angular_acceleration *= action_props.arguments.at(MAX_ANGULAR_ACCEL) / agent_angular_acceleration_magnitude;
            output_motion_object_angular_acceleration += agent_angular_acceleration;

            if (output_angular_motion_error_wrt_ideal != nullptr)
            {
                float_32_bit const  speed_ratio = length(angular_velocity_to_cancel_in_world_space);
                if (std::fabs(*output_angular_motion_error_wrt_ideal) < speed_ratio)
                    *output_angular_motion_error_wrt_ideal = speed_ratio;
            }
        }
        else
            NOT_IMPLEMENTED_YET();

    output_motion_action_data.swap(new_motion_action_data);
}


struct  find_best_keyframe_constants
{
    find_best_keyframe_constants(
            vector3 const&  desired_forward_unit_vector_in_world_space_,
            vector3 const&  desired_linear_velocity_unit_direction_in_world_space_,
            float_32_bit const  desired_linear_speed_,
            //---------------------------------
            skeletal_motion_templates_const_ptr const  motion_templates_,
            skeleton_composition_const_ptr const  skeleton_composition_,
            float_32_bit const  time_to_consume_in_seconds_,
            float_32_bit const  search_time_horizon_in_seconds_,
            float_32_bit const  time_step_for_motion_actions_in_seconds_,
            vector3 const&  gravity_acceleration_in_world_space_,
            vector3 const&  motion_object_origin_in_world_space_
            )
        : desired_forward_unit_vector_in_world_space(desired_forward_unit_vector_in_world_space_)
        , desired_linear_velocity_unit_direction_in_world_space(desired_linear_velocity_unit_direction_in_world_space_)
        , desired_linear_speed(desired_linear_speed_)

        , motion_templates(motion_templates_)
        , skeleton_composition(skeleton_composition_)
        , time_to_consume_in_seconds(time_to_consume_in_seconds_)
        , search_time_horizon_in_seconds(search_time_horizon_in_seconds_)
        , time_step_for_motion_actions_in_seconds(time_step_for_motion_actions_in_seconds_)
        , gravity_acceleration_in_world_space(gravity_acceleration_in_world_space_)
        , motion_object_origin_in_world_space(motion_object_origin_in_world_space_)
    {}

    vector3  desired_forward_unit_vector_in_world_space;
    vector3  desired_linear_velocity_unit_direction_in_world_space;
    float_32_bit  desired_linear_speed;

    skeletal_motion_templates_const_ptr  motion_templates;
    skeleton_composition_const_ptr  skeleton_composition;
    float_32_bit  time_to_consume_in_seconds;
    float_32_bit  search_time_horizon_in_seconds;
    float_32_bit  time_step_for_motion_actions_in_seconds;
    vector3  gravity_acceleration_in_world_space;
    vector3  motion_object_origin_in_world_space;
};


struct  find_best_keyframe_queue_record
{
    struct  pivot_props
    {
        skeletal_motion_templates::motion_template_cursor  cursor;
        float_32_bit  time_delta_in_seconds;
        float_32_bit  motion_error_wrt_ideal;
    };

    find_best_keyframe_queue_record(
            skeletal_motion_templates::motion_template_cursor const&  start_cursor,
            vector3 const&  start_motion_object_origin_in_world_space,
            vector3 const&  start_motion_object_forward_direction_in_world_space,
            vector3 const&  start_motion_object_up_direction_in_world_space,
            vector3 const&  start_motion_object_linear_velocity_in_world_space,
            vector3 const&  start_motion_object_angular_velocity_in_world_space
            );

    find_best_keyframe_queue_record(
            find_best_keyframe_queue_record const&  predecessor,
            skeletal_motion_templates::motion_template_cursor const&  cursor_override,
            find_best_keyframe_constants const&  constants
            );

    float_32_bit  cost;
    pivot_props  pivot;
    skeletal_motion_templates::motion_template_cursor  cursor;
    float_32_bit  time_taken_in_seconds;
    float_32_bit  motion_error_wrt_ideal;
    // Next follow motion object data (they represent basis for the computation of the cost)
    vector3  motion_object_origin_in_world_space;
    vector3  motion_object_forward_direction_in_world_space;
    vector3  motion_object_up_direction_in_world_space;
    vector3  motion_object_linear_velocity_in_world_space;
    vector3  motion_object_angular_velocity_in_world_space;

    // The code below is not important (allows to use this data type inside priority queue).

    static void _assign(find_best_keyframe_queue_record&  self, find_best_keyframe_queue_record const&  other);
    find_best_keyframe_queue_record() {}
    find_best_keyframe_queue_record(find_best_keyframe_queue_record const&  other) { _assign(*this, other); }
    find_best_keyframe_queue_record(find_best_keyframe_queue_record&&  other) { _assign(*this, other); }
    find_best_keyframe_queue_record& operator=(find_best_keyframe_queue_record const&  other) { _assign(*this, other); return *this; }
    find_best_keyframe_queue_record& operator=(find_best_keyframe_queue_record&&  other) { _assign(*this, other); return *this; }
};

void  find_best_keyframe_queue_record::_assign(
        find_best_keyframe_queue_record&  self,
        find_best_keyframe_queue_record const&  other
        )
{
    TMPROF_BLOCK();

    self.cost = other.cost;
    self.pivot = other.pivot;
    self.cursor = other.cursor;
    self.time_taken_in_seconds = other.time_taken_in_seconds;
    self.motion_error_wrt_ideal = other.motion_error_wrt_ideal;

    self.motion_object_origin_in_world_space = other.motion_object_origin_in_world_space;
    self.motion_object_forward_direction_in_world_space = other.motion_object_forward_direction_in_world_space;
    self.motion_object_up_direction_in_world_space = other.motion_object_up_direction_in_world_space;
    self.motion_object_linear_velocity_in_world_space = other.motion_object_linear_velocity_in_world_space;
    self.motion_object_angular_velocity_in_world_space = other.motion_object_angular_velocity_in_world_space;
}

find_best_keyframe_queue_record::find_best_keyframe_queue_record(
        skeletal_motion_templates::motion_template_cursor const&  start_cursor,
        vector3 const&  start_motion_object_origin_in_world_space,
        vector3 const&  start_motion_object_forward_direction_in_world_space,
        vector3 const&  start_motion_object_up_direction_in_world_space,
        vector3 const&  start_motion_object_linear_velocity_in_world_space,
        vector3 const&  start_motion_object_angular_velocity_in_world_space
        )
    : cost(0.0f)
    , pivot()
    , cursor(start_cursor)
    , time_taken_in_seconds(0.0f)
    , motion_error_wrt_ideal(0.0f)

    , motion_object_origin_in_world_space(start_motion_object_origin_in_world_space)
    , motion_object_forward_direction_in_world_space(start_motion_object_forward_direction_in_world_space)
    , motion_object_up_direction_in_world_space(start_motion_object_up_direction_in_world_space)
    , motion_object_linear_velocity_in_world_space(start_motion_object_linear_velocity_in_world_space)
    , motion_object_angular_velocity_in_world_space(start_motion_object_angular_velocity_in_world_space)
{}


find_best_keyframe_queue_record::find_best_keyframe_queue_record(
        find_best_keyframe_queue_record const&  predecessor,
        skeletal_motion_templates::motion_template_cursor const&  cursor_override,
        find_best_keyframe_constants const&  constants
        )
    : cost()
    , pivot(predecessor.pivot)
    , cursor{ cursor_override.motion_name, cursor_override.keyframe_index + 1U }
    , time_taken_in_seconds(predecessor.time_taken_in_seconds)
    , motion_error_wrt_ideal(predecessor.motion_error_wrt_ideal)

    , motion_object_origin_in_world_space(predecessor.motion_object_origin_in_world_space)
    , motion_object_forward_direction_in_world_space(predecessor.motion_object_forward_direction_in_world_space)
    , motion_object_up_direction_in_world_space(predecessor.motion_object_up_direction_in_world_space)
    , motion_object_linear_velocity_in_world_space(predecessor.motion_object_linear_velocity_in_world_space)
    , motion_object_angular_velocity_in_world_space(predecessor.motion_object_angular_velocity_in_world_space)
{
    TMPROF_BLOCK();

    skeletal_motion_templates::keyframes const&  animation = constants.motion_templates->motions_map.at(cursor.motion_name);

    float_32_bit  time_delta_in_seconds =
            animation.keyframe_at(cursor.keyframe_index).get_time_point() -
            animation.keyframe_at(cursor.keyframe_index - 1U).get_time_point();
    if (!pivot.cursor.motion_name.empty())
        while (true)
        {
            if (cursor.keyframe_index + 1U >= animation.get_keyframes().size())
                break;

            if (time_delta_in_seconds >= constants.time_step_for_motion_actions_in_seconds)
                break;

            if (time_taken_in_seconds + time_delta_in_seconds >= constants.search_time_horizon_in_seconds)
                break;

            bool  is_branching = false;
            for (auto const& equivalence_info : animation.get_meta_data().m_keyframe_equivalences.at(cursor.keyframe_index).m_records)
                if (!equivalence_info.arguments.empty())
                {
                    is_branching = true;
                    break;
                }
            if (is_branching)
                break;

            ++cursor.keyframe_index;
            time_delta_in_seconds +=
                    animation.keyframe_at(cursor.keyframe_index).get_time_point() -
                    animation.keyframe_at(cursor.keyframe_index - 1U).get_time_point();
        }
    INVARIANT(time_delta_in_seconds > 0.0001f);

    // --- UPDATING MOTION OBJECT LOCATION AND MOTION DATA ACCORDING TO MOTION ACTIONS --------------------------------

    vector3  motion_object_linear_acceleration = vector3_zero();
    vector3  motion_object_angular_acceleration = vector3_zero();
    action_controller_human::motion_action_data_map  motion_action_data;    // Not used (but still must be passed)
    float_32_bit* const  motion_error_ptr = pivot.cursor.motion_name.empty() ? &motion_error_wrt_ideal : nullptr;
    detail::compute_motion_object_acceleration_from_motion_actions(
            constants.desired_forward_unit_vector_in_world_space,
            constants.desired_linear_velocity_unit_direction_in_world_space,
            constants.desired_linear_speed,
            time_delta_in_seconds,
            animation,
            cursor.keyframe_index,
            cursor_override.keyframe_index,
            animation.get_meta_data().m_motion_actions.at(cursor.keyframe_index),
            constants.skeleton_composition->forward_direction_in_anim_space,
            constants.skeleton_composition->up_direction_in_anim_space,
            motion_object_origin_in_world_space,
            motion_object_forward_direction_in_world_space,
            motion_object_up_direction_in_world_space,
            motion_object_linear_velocity_in_world_space,
            motion_object_angular_velocity_in_world_space,
            constants.gravity_acceleration_in_world_space,
            motion_action_data,
            motion_object_linear_acceleration,
            motion_object_angular_acceleration,
            motion_action_data,
            motion_error_ptr,
            motion_error_ptr
            );

    motion_object_linear_velocity_in_world_space += time_delta_in_seconds * motion_object_linear_acceleration;
    motion_object_angular_velocity_in_world_space += time_delta_in_seconds * motion_object_angular_acceleration;

    motion_object_origin_in_world_space += time_delta_in_seconds * motion_object_linear_velocity_in_world_space;
    float_32_bit const  angular_speed = length(motion_object_angular_velocity_in_world_space);
    if (angular_speed > 0.001f)
    {
        vector3 const  rot_axis = (1.0f / angular_speed) * motion_object_angular_velocity_in_world_space;
        matrix33 const  rot_matrix =
                quaternion_to_rotation_matrix(angle_axis_to_quaternion(angular_speed * time_delta_in_seconds, rot_axis));
        motion_object_forward_direction_in_world_space = normalised(rot_matrix * motion_object_forward_direction_in_world_space);
        motion_object_up_direction_in_world_space = normalised(rot_matrix * motion_object_up_direction_in_world_space);
    }

    time_taken_in_seconds += time_delta_in_seconds;

    // --- COMPUTATION OF THE COST --------------------------------

    vector3 const  desired_position =
            constants.motion_object_origin_in_world_space +
            (time_taken_in_seconds * constants.desired_linear_speed) * constants.desired_linear_velocity_unit_direction_in_world_space;

    float_32_bit const  position_error = length(desired_position - motion_object_origin_in_world_space);

    float_32_bit const  orientation_error =
            angle(motion_object_forward_direction_in_world_space, constants.desired_forward_unit_vector_in_world_space);

    cost = position_error + orientation_error;
}


float_32_bit  find_best_keyframe(
        find_best_keyframe_constants const&  constants,
        find_best_keyframe_queue_record const&  start_record,
        skeletal_motion_templates::motion_template_cursor&  best_cursor
        )
{
    TMPROF_BLOCK();
    ASSUMPTION(constants.time_to_consume_in_seconds >= 0.0f);
    ASSUMPTION(constants.search_time_horizon_in_seconds > 0.0001f);
    ASSUMPTION(constants.time_to_consume_in_seconds < constants.search_time_horizon_in_seconds);

    using  find_best_keyframe_queue_record_ptr = std::shared_ptr<find_best_keyframe_queue_record>;

    std::unordered_map<
            skeletal_motion_templates::motion_template_cursor,
            find_best_keyframe_queue_record_ptr,
            skeletal_motion_templates::motion_template_cursor::hasher
            >
        pivot_records;

    std::priority_queue<
            find_best_keyframe_queue_record_ptr,
            std::vector<find_best_keyframe_queue_record_ptr>,
            std::function<bool(find_best_keyframe_queue_record_ptr const&, find_best_keyframe_queue_record_ptr const&)>
            >
        queue([](find_best_keyframe_queue_record_ptr const&  left, find_best_keyframe_queue_record_ptr const&  right) -> bool {
                // We need inverse order: obtain lower costs first.
                return left->cost > right->cost;
                });
    queue.push(std::make_shared<find_best_keyframe_queue_record>(start_record));

//natural_32_bit  max_queue_size = 0U;
//natural_32_bit  num_iterations = 0U;

    do
    {
//++num_iterations;
//if (max_queue_size < queue.size())
//    max_queue_size = (natural_32_bit)queue.size();

        find_best_keyframe_queue_record_ptr const  current = queue.top();
        queue.pop();

        {
            if (current->time_taken_in_seconds >= constants.search_time_horizon_in_seconds)
            {
                INVARIANT(!current->pivot.cursor.motion_name.empty());
                auto const  pivot_it = pivot_records.find(current->pivot.cursor);
                if (pivot_it == pivot_records.end())
                    pivot_records.insert({ current->pivot.cursor, current });
                else if (current->cost < pivot_it->second->cost)
                    pivot_it->second = current;
                continue;
            }
            else if (!current->pivot.cursor.motion_name.empty() && pivot_records.find(current->pivot.cursor) != pivot_records.end())
                continue;
        }

        std::vector<skeletal_motion_templates::motion_template_cursor>  cursors{ current->cursor };
        {
            skeletal_motion_templates::keyframes const&  animation = constants.motion_templates->motions_map.at(current->cursor.motion_name);
            for (auto const& equivalence_info : animation.get_meta_data().m_keyframe_equivalences.at(current->cursor.keyframe_index).m_records)
                if (!equivalence_info.arguments.empty())
                    cursors.push_back({ equivalence_info.keyword, equivalence_info.arguments.front() });
        }

        for (auto const&  cursor : cursors)
        {
            auto const&  keyframes = constants.motion_templates->motions_map.at(cursor.motion_name).get_keyframes();
            if (cursor.keyframe_index + 1U < keyframes.size())
            {
                std::shared_ptr<find_best_keyframe_queue_record> const  successor =
                        std::make_shared<find_best_keyframe_queue_record>(*current, cursor, constants);

                if (successor->pivot.cursor.motion_name.empty() && successor->time_taken_in_seconds >= constants.time_to_consume_in_seconds)
                {
                    if (pivot_records.find(successor->cursor) != pivot_records.end())
                        continue;
                    successor->pivot = { successor->cursor, successor->time_taken_in_seconds, successor->motion_error_wrt_ideal };
                }

                queue.push(successor);
            }
        }
    }
    while (!queue.empty());

//std::cout << "|Q|=" << max_queue_size << "\tN=" << num_iterations << "\t|P|=" << pivot_records.size() << std::endl;

    INVARIANT(!pivot_records.empty());
    find_best_keyframe_queue_record_ptr  winner = nullptr;
    for (auto const&  pivot : pivot_records)
        if (winner == nullptr || pivot.second->cost < winner->cost)
            winner = pivot.second;
    best_cursor = winner->pivot.cursor;
    float_32_bit const  animation_speed_coef = std::max(0.5f, std::min(2.0f, 1.0f + winner->pivot.motion_error_wrt_ideal));
    return winner->pivot.time_delta_in_seconds / animation_speed_coef;
}


}}

namespace ai {


action_controller_human::action_controller_human(blackboard_ptr const  blackboard_)
    : action_controller(blackboard_)
    , m_desired_forward_unit_vector_in_world_space(vector3_unit_x())
    , m_desired_linear_velocity_unit_direction_in_world_space(vector3_unit_x())
    , m_desired_linear_speed(0.0f)

    , m_template_motion_info({
            {"", 0U},       // src_pose
            {"", 0U},       // dst_pose
            0.0f,           // total_interpolation_time_in_seconds
            0.0f            // consumed_time_in_seconds
            })
    , m_motion_object_nid()
    , m_motion_object_collider_props()
    , m_motion_object_mass_distribution_props()
    , m_motion_object_constraint_props()
    , m_motion_object_action_props()
    , m_motion_action_data()
{
}


action_controller_human::~action_controller_human()
{
    if (m_motion_object_nid.valid())
    {
        get_blackboard()->m_scene->unregister_to_collision_contacts_stream(m_motion_object_nid, get_blackboard()->m_agent_id);
        detail::destroy_motion_scene_node(get_blackboard()->m_scene, m_motion_object_nid);
    }
}


void  action_controller_human::next_round(float_32_bit  time_step_in_seconds)
{
    TMPROF_BLOCK();

    // Perform initialisation of the action controller, if this is the first time step (data have not been initialised yet).
    if (m_template_motion_info.src_pose.motion_name.empty())
    {
        m_template_motion_info.src_pose.motion_name = get_blackboard()->m_motion_templates->motions_map.begin()->first;
        m_template_motion_info.dst_pose.motion_name = get_blackboard()->m_motion_templates->motions_map.begin()->first;

        angeo::coordinate_system  agent_frame;
        {
            angeo::coordinate_system  tmp;
            get_blackboard()->m_scene->get_frame_of_scene_node(get_blackboard()->m_bone_nids.front(), false, tmp);
            agent_frame.set_origin(tmp.origin());
            get_blackboard()->m_scene->get_frame_of_scene_node(get_blackboard()->m_agent_nid, false, tmp);
            agent_frame.set_orientation(tmp.orientation());
        }
        skeletal_motion_templates::keyframes const&  src_animation =
                get_blackboard()->m_motion_templates->motions_map.at(m_template_motion_info.src_pose.motion_name
                );
        m_motion_object_collider_props = src_animation.get_meta_data().m_motion_colliders.at(m_template_motion_info.src_pose.keyframe_index);
        m_motion_object_mass_distribution_props = src_animation.get_meta_data().m_mass_distributions.at(m_template_motion_info.src_pose.keyframe_index);
        m_motion_object_constraint_props = src_animation.get_meta_data().m_constraints.at(m_template_motion_info.src_pose.keyframe_index);
        m_motion_object_action_props = src_animation.get_meta_data().m_motion_actions.at(m_template_motion_info.src_pose.keyframe_index);
        m_motion_object_nid = detail::get_motion_object_nid(get_blackboard()->m_scene, get_blackboard()->m_agent_nid);
        if (!get_blackboard()->m_scene->has_scene_node(m_motion_object_nid))
            m_motion_object_nid = detail::create_motion_scene_node(
                        get_blackboard()->m_scene,
                        m_motion_object_nid,
                        agent_frame,
                        m_motion_object_collider_props.m_records.front(),
                        m_motion_object_mass_distribution_props.m_records.front()
                        );
        get_blackboard()->m_scene->register_to_collision_contacts_stream(m_motion_object_nid, get_blackboard()->m_agent_id);
    }

    // Computing basic positional and motional properties of the motion object.

    angeo::coordinate_system  motion_object_frame;
    get_blackboard()->m_scene->get_frame_of_scene_node(m_motion_object_nid, false, motion_object_frame);

    matrix44  motion_object_from_base_matrix;
    angeo::from_base_matrix(motion_object_frame, motion_object_from_base_matrix);

    vector3 const  motion_object_forward_direction_in_world_space =
            transform_vector(get_blackboard()->m_skeleton_composition->forward_direction_in_anim_space, motion_object_from_base_matrix);
    vector3 const  motion_object_up_direction_in_world_space =
            transform_vector(get_blackboard()->m_skeleton_composition->up_direction_in_anim_space, motion_object_from_base_matrix);

    vector3 const  gravity_accel = get_blackboard()->m_scene->get_gravity_acceleration_at_point(motion_object_frame.origin());

    vector3 const  motion_object_linear_velocity_in_world_space =
        get_blackboard()->m_scene->get_linear_velocity_of_rigid_body_of_scene_node(m_motion_object_nid);
    float_32_bit const  motion_object_linear_speed = length(motion_object_linear_velocity_in_world_space);
    vector3 const  motion_object_angular_velocity_in_world_space =
        get_blackboard()->m_scene->get_angular_velocity_of_rigid_body_of_scene_node(m_motion_object_nid);
    float_32_bit const  motion_object_angular_speed = length(motion_object_angular_velocity_in_world_space);


    // Clear all forces on the agent's motion object from the previous time step.
    {
        get_blackboard()->m_scene->set_linear_acceleration_of_rigid_body_of_scene_node(
            m_motion_object_nid,
            get_blackboard()->m_scene->get_gravity_acceleration_at_point(motion_object_frame.origin())
            );
        get_blackboard()->m_scene->set_angular_acceleration_of_rigid_body_of_scene_node(
            m_motion_object_nid,
            vector3_zero()
            );
    }

    // Synchronise agent's position in the world space according to its motion object in the previous time step
    get_blackboard()->m_scene->set_frame_of_scene_node(get_blackboard()->m_agent_nid, false, motion_object_frame);

    // Update desired motion of the agent, as the cortex wants it.
    {
        m_desired_linear_velocity_unit_direction_in_world_space =
                normalised(
                        quaternion_to_rotation_matrix(
                                angle_axis_to_quaternion(
                                        get_blackboard()->m_cortex_cmd_turn_intensity
                                            * get_blackboard()->m_max_turn_speed_in_radians_per_second
                                            * time_step_in_seconds,
                                        motion_object_up_direction_in_world_space
                                        )
                                )
                        * m_desired_linear_velocity_unit_direction_in_world_space
                        );
        m_desired_linear_speed = get_blackboard()->m_cortex_cmd_move_intensity * get_blackboard()->m_max_forward_speed_in_meters_per_second;

        // Currently we do not support motions where m_desired_forward_unit_vector_in_world_space and
        // m_desired_linear_velocity_unit_direction_in_world_space could be different. Also there is
        // distinguishing output from the cortex.
        m_desired_forward_unit_vector_in_world_space = m_desired_linear_velocity_unit_direction_in_world_space;
    }

    // Check whether the condition for applying forces towards the desired motion of the agent is satisfied or not.
    bool  is_motion_constraint_satisfied;
    {
        is_motion_constraint_satisfied = false;
        if (m_motion_object_constraint_props.m_records.front().keyword == "contact_normal_cone")
        {
            enum INDICES_OF_ARGUMENTS
            {
                UNIT_AXIS   = 0,
                ANGLE       = 3,
            };

            vector3 const  cone_unit_axis_vector_in_anim_space(
                    m_motion_object_constraint_props.m_records.front().arguments.at(UNIT_AXIS + 0),
                    m_motion_object_constraint_props.m_records.front().arguments.at(UNIT_AXIS + 1),
                    m_motion_object_constraint_props.m_records.front().arguments.at(UNIT_AXIS + 2)
                    );
            vector3 const  cone_unit_axis_vector_in_world_space =
                    transform_vector(cone_unit_axis_vector_in_anim_space, motion_object_from_base_matrix);
            auto const  begin_and_end = get_blackboard()->m_collision_contacts.equal_range(m_motion_object_nid);
            for (auto  it = begin_and_end.first; it != begin_and_end.second; ++it)
                if (it->second.normal_force_magnitude > 0.001f &&
                    angle(it->second.unit_normal, cone_unit_axis_vector_in_world_space)
                        < m_motion_object_constraint_props.m_records.front().arguments.at(ANGLE))
                {
                    is_motion_constraint_satisfied = true;
                    break;
                }
        }
        else
            NOT_IMPLEMENTED_YET();
    }

    // Apply forces towards the desired motion, if the condition for doing so is satified.
    if (is_motion_constraint_satisfied == true)
    {
        vector3  motion_object_linear_acceleration = vector3_zero();
        vector3  motion_object_angular_acceleration = vector3_zero();
        detail::compute_motion_object_acceleration_from_motion_actions(
                m_desired_forward_unit_vector_in_world_space,
                m_desired_linear_velocity_unit_direction_in_world_space,
                m_desired_linear_speed,
                time_step_in_seconds,
                get_blackboard()->m_motion_templates->motions_map.at(m_template_motion_info.dst_pose.motion_name),
                m_template_motion_info.dst_pose.keyframe_index,
                m_template_motion_info.dst_pose.keyframe_index - 1U,
                m_motion_object_action_props,
                get_blackboard()->m_skeleton_composition->forward_direction_in_anim_space,
                get_blackboard()->m_skeleton_composition->up_direction_in_anim_space,
                motion_object_frame.origin(),
                motion_object_forward_direction_in_world_space,
                motion_object_up_direction_in_world_space,
                motion_object_linear_velocity_in_world_space,
                motion_object_angular_velocity_in_world_space,
                gravity_accel,
                m_motion_action_data,
                motion_object_linear_acceleration,
                motion_object_angular_acceleration,
                m_motion_action_data
                );
        get_blackboard()->m_scene->add_to_linear_acceleration_of_rigid_body_of_scene_node(m_motion_object_nid, motion_object_linear_acceleration);
        get_blackboard()->m_scene->add_to_angular_acceleration_of_rigid_body_of_scene_node(m_motion_object_nid, motion_object_angular_acceleration);
    }
    else
        m_motion_action_data.clear();

    // Choose another pair of keyframes to interpolate, if we exhaused all interpolation time of the previous pair.
    if (m_template_motion_info.consumed_time_in_seconds + time_step_in_seconds > m_template_motion_info.total_interpolation_time_in_seconds)
    {
        float_32_bit const  time_till_dst_pose =
                m_template_motion_info.total_interpolation_time_in_seconds - m_template_motion_info.consumed_time_in_seconds;
        INVARIANT(time_till_dst_pose >= 0.0f);

        time_step_in_seconds -= time_till_dst_pose;
        INVARIANT(time_step_in_seconds >= 0.0f);

        m_template_motion_info.src_pose = m_template_motion_info.dst_pose;
        m_template_motion_info.consumed_time_in_seconds = 0.0f;
        m_template_motion_info.total_interpolation_time_in_seconds =
                detail::find_best_keyframe(
                        detail::find_best_keyframe_constants(
                                m_desired_forward_unit_vector_in_world_space,
                                m_desired_linear_velocity_unit_direction_in_world_space,
                                m_desired_linear_speed,
                                get_blackboard()->m_motion_templates,
                                get_blackboard()->m_skeleton_composition,
                                time_step_in_seconds,
                                1.0f,
                                0.25f,
                                gravity_accel,
                                motion_object_frame.origin()
                                ),
                        detail::find_best_keyframe_queue_record(
                                m_template_motion_info.src_pose,
                                motion_object_frame.origin(),
                                motion_object_forward_direction_in_world_space,
                                motion_object_up_direction_in_world_space,
                                motion_object_linear_velocity_in_world_space,
                                motion_object_angular_velocity_in_world_space
                                ),
                        m_template_motion_info.dst_pose
                        );

        time_step_in_seconds = std::min(time_step_in_seconds, m_template_motion_info.total_interpolation_time_in_seconds);
    }

    // Interpolate the pair of keyframes (from the source keyframe to the target one).
    float_32_bit  interpolation_param;
    {
        m_template_motion_info.consumed_time_in_seconds += time_step_in_seconds;
        INVARIANT(m_template_motion_info.consumed_time_in_seconds <= m_template_motion_info.total_interpolation_time_in_seconds);

        interpolation_param =
                m_template_motion_info.consumed_time_in_seconds / m_template_motion_info.total_interpolation_time_in_seconds;

        skeletal_motion_templates::keyframes const&  src_animation =
                get_blackboard()->m_motion_templates->motions_map.at(m_template_motion_info.src_pose.motion_name
                );
        skeletal_motion_templates::keyframes const&  dst_animation =
                get_blackboard()->m_motion_templates->motions_map.at(m_template_motion_info.dst_pose.motion_name
                );

        std::vector<angeo::coordinate_system>  interpolated_frames_in_animation_space;
        interpolate_keyframes_spherical(
                src_animation.keyframe_at(m_template_motion_info.src_pose.keyframe_index).get_coord_systems(),
                dst_animation.keyframe_at(m_template_motion_info.dst_pose.keyframe_index).get_coord_systems(),
                interpolation_param,
                interpolated_frames_in_animation_space
                );

        angeo::coordinate_system  reference_frame_in_animation_space;
        angeo::interpolate_spherical(
                src_animation.get_meta_data().m_reference_frames.at(m_template_motion_info.src_pose.keyframe_index),
                dst_animation.get_meta_data().m_reference_frames.at(m_template_motion_info.dst_pose.keyframe_index),
                interpolation_param,
                reference_frame_in_animation_space
                );

        std::vector<angeo::coordinate_system>  interpolated_frames_in_world_space;
        {
            interpolated_frames_in_world_space.reserve(interpolated_frames_in_animation_space.size());

            matrix44  Ainv, M;
            angeo::to_base_matrix(reference_frame_in_animation_space, Ainv);
            M = motion_object_from_base_matrix * Ainv;
            for (angeo::coordinate_system const&  frame : interpolated_frames_in_animation_space)
            {
                vector3  u;
                matrix33  R;
                {
                    matrix44  N;
                    angeo::from_base_matrix(frame, N);
                    decompose_matrix44(M * N, u, R);
                }
                interpolated_frames_in_world_space.push_back({ u, normalised(rotation_matrix_to_quaternion(R)) });
            }
        }

        for (natural_32_bit  bone = 0; bone != interpolated_frames_in_world_space.size(); ++bone)
            get_blackboard()->m_scene->set_frame_of_scene_node(
                    get_blackboard()->m_bone_nids.at(bone),
                    false,
                    interpolated_frames_in_world_space.at(bone)
                    );
    }
    
    // Interpolate motion object meta-data.
    {
        skeletal_motion_templates::keyframes const&  src_animation =
                get_blackboard()->m_motion_templates->motions_map.at(m_template_motion_info.src_pose.motion_name
                );
        skeletal_motion_templates::keyframes const&  dst_animation =
                get_blackboard()->m_motion_templates->motions_map.at(m_template_motion_info.dst_pose.motion_name
                );

        auto const&  src_meta_data_colliders = src_animation.get_meta_data().m_motion_colliders.at(m_template_motion_info.src_pose.keyframe_index);
        auto const&  dst_meta_data_colliders = dst_animation.get_meta_data().m_motion_colliders.at(m_template_motion_info.dst_pose.keyframe_index);

        float_32_bit const  src_weight = src_meta_data_colliders.m_records.front().arguments.back();
        float_32_bit const  dst_weight = dst_meta_data_colliders.m_records.front().arguments.back();

        float_32_bit const  motion_object_interpolation_param =
                (src_weight + dst_weight < 0.0001f) ? 0.5f : src_weight / (src_weight + dst_weight);

        if (motion_object_interpolation_param <= interpolation_param)
        {
            auto const&  dst_meta_data_mass_distributions =
                    dst_animation.get_meta_data().m_mass_distributions.at(m_template_motion_info.dst_pose.keyframe_index);

            if (m_motion_object_collider_props != dst_meta_data_colliders || m_motion_object_mass_distribution_props != dst_meta_data_mass_distributions)
            {
                m_motion_object_collider_props = dst_meta_data_colliders;
                m_motion_object_mass_distribution_props = dst_meta_data_mass_distributions;

                detail::rigid_body_motion const  rb_motion(
                        get_blackboard()->m_scene,
                        m_motion_object_nid,
                        m_motion_object_mass_distribution_props.m_records.front()
                        );

                get_blackboard()->m_scene->unregister_to_collision_contacts_stream(m_motion_object_nid, get_blackboard()->m_agent_id);
                detail::destroy_collider_and_rigid_bofy_of_motion_scene_node(get_blackboard()->m_scene, m_motion_object_nid);
                detail::create_collider_and_rigid_body_of_motion_scene_node(
                        get_blackboard()->m_scene,
                        m_motion_object_nid,
                        m_motion_object_collider_props.m_records.front(),
                        rb_motion);
                get_blackboard()->m_scene->register_to_collision_contacts_stream(m_motion_object_nid, get_blackboard()->m_agent_id);
            }

            m_motion_object_constraint_props = dst_animation.get_meta_data().m_constraints.at(m_template_motion_info.dst_pose.keyframe_index);
            m_motion_object_action_props = dst_animation.get_meta_data().m_motion_actions.at(m_template_motion_info.dst_pose.keyframe_index);
        }
    }
}


}
