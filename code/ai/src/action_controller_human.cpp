#include <ai/action_controller_human.hpp>
#include <ai/skeleton_utils.hpp>
#include <angeo/skeleton_kinematics.hpp>
#include <scene/scene_node_id.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
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


struct  find_best_keyframe_constants
{
    skeletal_motion_templates_const_ptr  motion_templates;
    float_32_bit  time_to_consume_in_seconds;
    float_32_bit  search_time_horizon_in_seconds;
    vector3  desired_linear_velocity_in_world_space;
};


struct  find_best_keyframe_queue_record
{
    explicit find_best_keyframe_queue_record(skeletal_motion_templates::motion_template_cursor const&  start);

    find_best_keyframe_queue_record(
            find_best_keyframe_queue_record const&  predecessor,
            skeletal_motion_templates::motion_template_cursor const&  cursor_override,
            find_best_keyframe_constants const&  constants
            );

    skeletal_motion_templates::motion_template_cursor  cursor;
    float_32_bit  distance_travelled_in_meters;
    float_32_bit  time_taken_in_seconds;
    float_32_bit  cost;
    integer_32_bit  start_record_index;
};


find_best_keyframe_queue_record::find_best_keyframe_queue_record(skeletal_motion_templates::motion_template_cursor const&  start)
    : cursor(start)
    , distance_travelled_in_meters(0.0f)
    , time_taken_in_seconds(0.0f)
    , cost(std::numeric_limits<float_32_bit>::max())
    , start_record_index(-1)
{}


find_best_keyframe_queue_record::find_best_keyframe_queue_record(
        find_best_keyframe_queue_record const&  predecessor,
        skeletal_motion_templates::motion_template_cursor const&  cursor_override,
        find_best_keyframe_constants const&  constants
        )
    : cursor{ cursor_override.motion_name, cursor_override.keyframe_index + 1U }
    , distance_travelled_in_meters(predecessor.distance_travelled_in_meters)
    , time_taken_in_seconds(predecessor.time_taken_in_seconds)
    , cost()
    , start_record_index(predecessor.start_record_index)
{
    skeletal_motion_templates::keyframes const&  animation = constants.motion_templates->motions_map.at(cursor.motion_name);

    float_32_bit const  time_delta_in_seconds =
            animation.keyframe_at(cursor.keyframe_index).get_time_point() -
            animation.keyframe_at(cursor.keyframe_index - 1U).get_time_point();
    INVARIANT(time_delta_in_seconds > 0.0001f);

    time_taken_in_seconds += time_delta_in_seconds;

    vector3 const  position_delta_in_anim_space =
            animation.get_meta_data().m_reference_frames.at(cursor.keyframe_index).origin() -
            animation.get_meta_data().m_reference_frames.at(cursor.keyframe_index - 1).origin();

    distance_travelled_in_meters += length(position_delta_in_anim_space);

    float_32_bit const  ideal_distance = length(constants.desired_linear_velocity_in_world_space) * time_taken_in_seconds;

    cost = std::fabs(ideal_distance - distance_travelled_in_meters);
}


float_32_bit  find_best_keyframe(
        skeletal_motion_templates::motion_template_cursor const&  src_cursor,
        find_best_keyframe_constants const&  constants,
        skeletal_motion_templates::motion_template_cursor&  best_cursor
        )
{
    TMPROF_BLOCK();
    ASSUMPTION(constants.time_to_consume_in_seconds >= 0.0f);
    ASSUMPTION(constants.search_time_horizon_in_seconds > 0.0001f);
    ASSUMPTION(constants.time_to_consume_in_seconds < constants.search_time_horizon_in_seconds);

    using start_record = std::pair<skeletal_motion_templates::motion_template_cursor, float_32_bit>;
    std::vector<start_record>  start_records;

    std::priority_queue<
            find_best_keyframe_queue_record,
            std::vector<find_best_keyframe_queue_record>,
            std::function<bool(find_best_keyframe_queue_record const&, find_best_keyframe_queue_record const&)>
            >
        queue([](find_best_keyframe_queue_record const&  left, find_best_keyframe_queue_record const&  right) -> bool {
                return left.cost > right.cost; // We need inverse order: obtain lower costs first.
                });
    queue.push(find_best_keyframe_queue_record(src_cursor));

    while (true)
    {
        find_best_keyframe_queue_record const  current = queue.top();
        queue.pop();

        if (current.time_taken_in_seconds > constants.search_time_horizon_in_seconds)
        {
            INVARIANT(current.start_record_index >= 0);
            auto const&  record = start_records.at(current.start_record_index);
            best_cursor = record.first;
            return record.second;
        }

        std::vector<skeletal_motion_templates::motion_template_cursor>  cursors{ current.cursor };
        {
            skeletal_motion_templates::keyframes const&  animation = constants.motion_templates->motions_map.at(current.cursor.motion_name);
            for (auto const& equivalence_info : animation.get_meta_data().m_keyframe_equivalences.at(current.cursor.keyframe_index).m_records)
                if (!equivalence_info.arguments.empty())
                    cursors.push_back({ equivalence_info.keyword, equivalence_info.arguments.front() });
        }

        bool  was_at_least_one_successor_processed = false;
        for (auto const&  cursor : cursors)
        {
            auto const&  keyframes = constants.motion_templates->motions_map.at(cursor.motion_name).get_keyframes();
            if (cursor.keyframe_index + 1U < keyframes.size())
            {
                was_at_least_one_successor_processed = true;

                find_best_keyframe_queue_record  successor(current, cursor, constants);

                if (successor.start_record_index < 0 && successor.time_taken_in_seconds >= constants.time_to_consume_in_seconds)
                {
                    successor.start_record_index = (integer_32_bit)start_records.size();
                    start_records.push_back({ successor.cursor, successor.time_taken_in_seconds });
                }

                queue.push(successor);
            }
        }
        INVARIANT(was_at_least_one_successor_processed == true);
    }
}


struct  motion_action_data__dont_move : public action_controller_human::motion_action_data
{
    explicit  motion_action_data__dont_move(vector3 const&  position_)
        : position(position_)
    {}
    vector3  position;
};



}}

namespace ai {


action_controller_human::action_controller_human(blackboard_ptr const  blackboard_)
    : action_controller(blackboard_)
    , m_template_motion_info({
            {"", 0U},       // src_pose
            {"", 0U},       // dst_pose
            0.0f,           // total_interpolation_time_in_seconds
            0.0f            // consumed_time_in_seconds
            })
    , m_desired_linear_velocity_unit_direction_in_world_space(vector3_unit_x())
    , m_desired_linear_velocity_in_world_space(vector3_zero())
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

    angeo::coordinate_system  motion_object_frame;
    get_blackboard()->m_scene->get_frame_of_scene_node(m_motion_object_nid, false, motion_object_frame);
    matrix44  motion_object_from_base_matrix;
    angeo::from_base_matrix(motion_object_frame, motion_object_from_base_matrix);
    vector3 const  motion_object_forward_direction_in_world_space =
            transform_vector(get_blackboard()->m_skeleton_composition->forward_direction_in_anim_space, motion_object_from_base_matrix);
    vector3 const  motion_object_up_direction_in_world_space =
            transform_vector(get_blackboard()->m_skeleton_composition->up_direction_in_anim_space, motion_object_from_base_matrix);

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
        m_desired_linear_velocity_in_world_space =
                (get_blackboard()->m_cortex_cmd_move_intensity * get_blackboard()->m_max_forward_speed_in_meters_per_second)
                * m_desired_linear_velocity_unit_direction_in_world_space;
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
                    quaternion_to_rotation_matrix(motion_object_frame.orientation()) * cone_unit_axis_vector_in_anim_space;
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
        vector3 const  gravity_accel = get_blackboard()->m_scene->get_gravity_acceleration_at_point(motion_object_frame.origin());

        for (auto const&  action_props : m_motion_object_action_props.m_records)
            if (action_props.keyword == "none")
                continue;
            else if (action_props.keyword == "accelerate_towards_clipped_desired_linear_velocity")
            {
                enum INDICES_OF_ARGUMENTS
                {
                    VEC_FWD             = 0,
                    ANGLE               = 3,
                    MAX_LINEAR_ACCEL    = 4,
                };

                vector3  clipped_desired_linear_velocity_in_world_space;
                {
                    vector3 const  cone_unit_axis_vector_in_anim_space(
                            action_props.arguments.at(VEC_FWD + 0),
                            action_props.arguments.at(VEC_FWD + 1),
                            action_props.arguments.at(VEC_FWD + 2)
                            );
                    vector3 const  cone_unit_axis_vector_in_world_space =
                            quaternion_to_rotation_matrix(motion_object_frame.orientation()) * cone_unit_axis_vector_in_anim_space;
                    vector3  rot_axis = cross_product(cone_unit_axis_vector_in_world_space, m_desired_linear_velocity_unit_direction_in_world_space);
                    float_32_bit const  rot_axis_length = length(rot_axis);

                    float_32_bit  ideal_linear_speed;
                    {
                        skeletal_motion_templates::keyframes const&  animation =
                                get_blackboard()->m_motion_templates->motions_map.at(m_template_motion_info.dst_pose.motion_name
                                );
                        vector3 const  position_delta =
                                animation.get_meta_data().m_reference_frames.at(m_template_motion_info.dst_pose.keyframe_index).origin() -
                                animation.get_meta_data().m_reference_frames.at(m_template_motion_info.dst_pose.keyframe_index - 1U).origin();
                        float_32_bit const  time_delta =
                                animation.keyframe_at(m_template_motion_info.dst_pose.keyframe_index).get_time_point() -
                                animation.keyframe_at(m_template_motion_info.dst_pose.keyframe_index - 1U).get_time_point() ;
                        ideal_linear_speed = length(position_delta) / std::max(time_delta, 0.0001f);
                    }

                    if (rot_axis_length < 0.001f)
                        clipped_desired_linear_velocity_in_world_space =
                            (ideal_linear_speed / length(cone_unit_axis_vector_in_world_space)) * cone_unit_axis_vector_in_world_space;
                    else
                    {
                        rot_axis /= rot_axis_length;
                        float_32_bit  rot_angle;
                        {
                            float_32_bit const  full_rot_angle =
                                    angeo::compute_rotation_angle(
                                            rot_axis,
                                            cone_unit_axis_vector_in_world_space,
                                            m_desired_linear_velocity_unit_direction_in_world_space
                                            );
                            if (std::fabs(full_rot_angle) <= action_props.arguments.at(ANGLE))
                                rot_angle = full_rot_angle;
                            else
                                rot_angle = (full_rot_angle >= 0.0f ? 1.0f : -1.0f) * action_props.arguments.at(ANGLE);
                        }
                        clipped_desired_linear_velocity_in_world_space =
                                quaternion_to_rotation_matrix(angle_axis_to_quaternion(rot_angle, rot_axis))
                                * cone_unit_axis_vector_in_world_space;

                        clipped_desired_linear_velocity_in_world_space *=
                                ideal_linear_speed / length(clipped_desired_linear_velocity_in_world_space);
                    }
                }
                vector3 const  current_linear_velocity_in_world_space =
                        get_blackboard()->m_scene->get_linear_velocity_of_rigid_body_of_scene_node(m_motion_object_nid);
                vector3  agent_linear_acceleration =
                        (clipped_desired_linear_velocity_in_world_space - current_linear_velocity_in_world_space) / time_step_in_seconds;
                float_32_bit const  agent_linear_acceleration_magnitude = length(agent_linear_acceleration);
                if (agent_linear_acceleration_magnitude > action_props.arguments.at(MAX_LINEAR_ACCEL))
                    agent_linear_acceleration *= action_props.arguments.at(MAX_LINEAR_ACCEL) / agent_linear_acceleration_magnitude;
                get_blackboard()->m_scene->add_to_linear_acceleration_of_rigid_body_of_scene_node(m_motion_object_nid, agent_linear_acceleration);
            }
            else if (action_props.keyword == "chase_linear_velocity_by_forward_vector")
            {
                enum INDICES_OF_ARGUMENTS
                {
                    VEC_FWD             = 0,
                    UNIT_AXIS           = 3,
                    MAX_ANGULAR_SPEED   = 6,
                    MAX_ANGULAR_ACCEL   = 7,
                };

                vector3 const  current_linear_velocity_in_world_space =
                        get_blackboard()->m_scene->get_linear_velocity_of_rigid_body_of_scene_node(m_motion_object_nid);
                vector3 const  current_angular_velocity_in_world_space =
                        get_blackboard()->m_scene->get_angular_velocity_of_rigid_body_of_scene_node(m_motion_object_nid);

                bool const  is_current_linear_velocity_too_slow =
                        length(current_linear_velocity_in_world_space) < length(gravity_accel) * time_step_in_seconds;

                vector3 const  source_linear_velocity_direction_in_anim_space(
                        action_props.arguments.at(VEC_FWD + 0),
                        action_props.arguments.at(VEC_FWD + 1),
                        action_props.arguments.at(VEC_FWD + 2)
                        );
                vector3 const  rot_axis_in_anim_space(
                        action_props.arguments.at(UNIT_AXIS + 0),
                        action_props.arguments.at(UNIT_AXIS + 1),
                        action_props.arguments.at(UNIT_AXIS + 2)
                        );

                matrix33 const  from_motion_to_world_rot_matrix = quaternion_to_rotation_matrix(motion_object_frame.orientation());

                vector3 const  rot_axis_in_world_space = from_motion_to_world_rot_matrix * rot_axis_in_anim_space;
                vector3 const  source_linear_velocity_direction_in_world_space =
                        from_motion_to_world_rot_matrix * source_linear_velocity_direction_in_anim_space;

                float_32_bit const  rot_angle =
                        angeo::compute_rotation_angle(
                                normalised(rot_axis_in_world_space),
                                source_linear_velocity_direction_in_world_space,
                                is_current_linear_velocity_too_slow ?
                                    m_desired_linear_velocity_unit_direction_in_world_space :
                                    current_linear_velocity_in_world_space
                                );
        
                float_32_bit  desired_angular_velocity_magnitude = rot_angle / time_step_in_seconds;
                if (desired_angular_velocity_magnitude >= 0.0f && desired_angular_velocity_magnitude > action_props.arguments.at(MAX_ANGULAR_SPEED))
                    desired_angular_velocity_magnitude = action_props.arguments.at(MAX_ANGULAR_SPEED);
                if (desired_angular_velocity_magnitude < 0.0f && desired_angular_velocity_magnitude < -action_props.arguments.at(MAX_ANGULAR_SPEED))
                    desired_angular_velocity_magnitude = -action_props.arguments.at(MAX_ANGULAR_SPEED);

                vector3 const  desired_angular_velocity = desired_angular_velocity_magnitude * normalised(rot_axis_in_world_space);

                vector3  agent_angular_acceleration =
                        (desired_angular_velocity - current_angular_velocity_in_world_space) / time_step_in_seconds;
                float_32_bit const  agent_angular_acceleration_magnitude = length(agent_angular_acceleration);
                if (agent_angular_acceleration_magnitude > action_props.arguments.at(MAX_ANGULAR_ACCEL))
                    agent_angular_acceleration *= action_props.arguments.at(MAX_ANGULAR_ACCEL) / agent_angular_acceleration_magnitude;
                get_blackboard()->m_scene->add_to_angular_acceleration_of_rigid_body_of_scene_node(m_motion_object_nid, agent_angular_acceleration);
            }
            else if (action_props.keyword == "dont_move")
            {
                enum INDICES_OF_ARGUMENTS
                {
                    MAX_LINEAR_ACCEL = 0,
                };

                detail::motion_action_data__dont_move const*  data_ptr;
                {
                    auto  data_it = m_motion_action_data.find(action_props.keyword);
                    if (data_it == m_motion_action_data.end())
                        data_it = m_motion_action_data.insert({
                                action_props.keyword,
                                std::make_unique<detail::motion_action_data__dont_move>(motion_object_frame.origin())
                                }).first;
                    data_ptr = dynamic_cast<detail::motion_action_data__dont_move const*>(data_it->second.get());
                    INVARIANT(data_ptr != nullptr);
                }

                vector3 const  sliding_prevention_accel =
                        2.0f * (data_ptr->position - motion_object_frame.origin()) / (time_step_in_seconds * time_step_in_seconds);
                vector3 const  current_linear_velocity_in_world_space =
                        get_blackboard()->m_scene->get_linear_velocity_of_rigid_body_of_scene_node(m_motion_object_nid);
                vector3  agent_linear_acceleration =
                        -current_linear_velocity_in_world_space / time_step_in_seconds + sliding_prevention_accel;
                float_32_bit const  agent_linear_acceleration_magnitude = length(agent_linear_acceleration);
                if (agent_linear_acceleration_magnitude > action_props.arguments.at(MAX_LINEAR_ACCEL))
                    agent_linear_acceleration *= action_props.arguments.at(MAX_LINEAR_ACCEL) / agent_linear_acceleration_magnitude;
                get_blackboard()->m_scene->add_to_linear_acceleration_of_rigid_body_of_scene_node(m_motion_object_nid, agent_linear_acceleration);
            }
            else if (action_props.keyword == "dont_rotate")
            {
                enum INDICES_OF_ARGUMENTS
                {
                    UNIT_AXIS           = 0,
                    MAX_ANGULAR_ACCEL   = 3,
                };

                vector3 const  current_angular_velocity_in_world_space =
                        get_blackboard()->m_scene->get_angular_velocity_of_rigid_body_of_scene_node(m_motion_object_nid);

                vector3 const  rot_axis_in_anim_space(
                        action_props.arguments.at(UNIT_AXIS + 0),
                        action_props.arguments.at(UNIT_AXIS + 1),
                        action_props.arguments.at(UNIT_AXIS + 2)
                        );

                matrix33 const  from_motion_to_world_rot_matrix = quaternion_to_rotation_matrix(motion_object_frame.orientation());

                vector3 const  rot_axis_in_world_space = normalised(from_motion_to_world_rot_matrix * rot_axis_in_anim_space);

                vector3 const  angular_velocity_to_cancel_in_world_space =
                        dot_product(rot_axis_in_world_space, current_angular_velocity_in_world_space) * rot_axis_in_world_space;

                vector3  agent_angular_acceleration = -angular_velocity_to_cancel_in_world_space / time_step_in_seconds;
                float_32_bit const  agent_angular_acceleration_magnitude = length(agent_angular_acceleration);
                if (agent_angular_acceleration_magnitude > action_props.arguments.at(MAX_ANGULAR_ACCEL))
                    agent_angular_acceleration *= action_props.arguments.at(MAX_ANGULAR_ACCEL) / agent_angular_acceleration_magnitude;
                get_blackboard()->m_scene->add_to_angular_acceleration_of_rigid_body_of_scene_node(m_motion_object_nid, agent_angular_acceleration);
            }
            else
                NOT_IMPLEMENTED_YET();
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

        vector3 const  current_linear_velocity_in_world_space =
                get_blackboard()->m_scene->get_linear_velocity_of_rigid_body_of_scene_node(m_motion_object_nid);

        vector3  target_linear_velocity_in_world_space;
        {
            if (m_template_motion_info.dst_pose.keyframe_index == 0U)
                target_linear_velocity_in_world_space = current_linear_velocity_in_world_space;
            else
            {
                skeletal_motion_templates::keyframes const&  animation =
                        get_blackboard()->m_motion_templates->motions_map.at(m_template_motion_info.dst_pose.motion_name
                        );
                float_32_bit const  time_delta =
                        animation.keyframe_at(m_template_motion_info.dst_pose.keyframe_index).get_time_point() -
                        animation.keyframe_at(m_template_motion_info.dst_pose.keyframe_index - 1U).get_time_point();
                INVARIANT(time_delta > 0.0f);
                vector3 const  position_delta =
                        animation.get_meta_data().m_reference_frames.at(m_template_motion_info.dst_pose.keyframe_index).origin() -
                        animation.get_meta_data().m_reference_frames.at(m_template_motion_info.dst_pose.keyframe_index - 1U).origin();

                vector3 const  average_linear_velocity_in_anim_space = position_delta / std::max(time_delta, 0.0001f);

                float_32_bit const  speed_distance =
                        std::fabs(length(current_linear_velocity_in_world_space) - length(average_linear_velocity_in_anim_space));

                float_32_bit const  max_speed_distance = std::max(0.25f, 0.5f * length(average_linear_velocity_in_anim_space));

                if (speed_distance <= max_speed_distance)
                    target_linear_velocity_in_world_space = m_desired_linear_velocity_in_world_space;
                else
                    target_linear_velocity_in_world_space = current_linear_velocity_in_world_space;
            }
        }

        m_template_motion_info.src_pose = m_template_motion_info.dst_pose;
        m_template_motion_info.consumed_time_in_seconds = 0.0f;
        m_template_motion_info.total_interpolation_time_in_seconds =
                detail::find_best_keyframe(
                        m_template_motion_info.src_pose,
                        detail::find_best_keyframe_constants{
                                get_blackboard()->m_motion_templates,       // motion_templates
                                time_step_in_seconds,                       // time_to_consume
                                1.0f,                                       // search_time_horizon
                                target_linear_velocity_in_world_space       // desired_linear_velocity
                                },
                        m_template_motion_info.dst_pose
                        );

        float_32_bit  animation_speed_coef;
        {
            skeletal_motion_templates::keyframes const&  animation =
                    get_blackboard()->m_motion_templates->motions_map.at(m_template_motion_info.dst_pose.motion_name
                    );
            float_32_bit const  time_delta_in_seconds =
                    animation.keyframe_at(m_template_motion_info.dst_pose.keyframe_index).get_time_point() -
                    animation.keyframe_at(m_template_motion_info.dst_pose.keyframe_index - 1U).get_time_point();
            INVARIANT(time_delta_in_seconds > 0.0f);
            vector3 const  position_delta_in_anim_space =
                    animation.get_meta_data().m_reference_frames.at(m_template_motion_info.dst_pose.keyframe_index).origin() -
                    animation.get_meta_data().m_reference_frames.at(m_template_motion_info.dst_pose.keyframe_index - 1U).origin();
            vector3 const  average_linear_velocity_in_anim_space =
                    position_delta_in_anim_space / std::max(time_delta_in_seconds, 0.0001f);
            float_32_bit const  linear_acceleration_ratio =
                    (length(current_linear_velocity_in_world_space) + 0.001f) / (length(average_linear_velocity_in_anim_space) + 0.001f);
            
            animation_speed_coef = std::max(0.5f, std::min(2.0f, linear_acceleration_ratio));
        }

        m_template_motion_info.total_interpolation_time_in_seconds /= animation_speed_coef;

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

            matrix44  W, Ainv, M;
            angeo::from_base_matrix(motion_object_frame, W);
            angeo::to_base_matrix(reference_frame_in_animation_space, Ainv);
            M = W * Ainv;
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
