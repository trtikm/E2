#include <ai/detail/action_controller_roller.hpp>
#include <ai/skeletal_motion_templates.hpp>
#include <ai/sensory_controller.hpp>
#include <angeo/utility.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai { namespace detail {


action_controller_roller::action_controller_roller(blackboard_agent_weak_ptr const  blackboard_)
    : m_blackboard(blackboard_)

    , m_config {
            0.25f,              // ROLLER_RADIUS
            0.021827f,          // ROLLER_MASS_INVERTED
            angeo::COLLISION_MATERIAL_TYPE::WOOD,           // ROLLER_MATERIAL
            angeo::COLLISION_CLASS::AGENT_MOTION_OBJECT,    // ROLLER_COLLISION_CLASS

            0.45f,              // BODY_EXCENTRICITY
            0.00589918f,        // BODY_MASS_INVERTED
            row_vectors_to_matrix(
                    vector3(0.174896f, -0.0723081f, 0.0254452f),
                    vector3(-0.0723081f, 0.133024f, -0.0191229f),
                    vector3(0.0254452f, -0.0191229f, 0.0854113f)
                    ),          // BODY_INERTIA_TENSOR_INVERTED
            angeo::COLLISION_MATERIAL_TYPE::WOOD,           // BODY_MATERIAL
            angeo::COLLISION_CLASS::AGENT_MOTION_OBJECT,    // BODT_COLLISION_CLASS

            0.1f,               // MAX_NO_CONTACT_SECONDS_FOR_DESIRE_FRAME_UPDATE

            0.825f,             // AGENT_FRAME_ORIGIN_Z_OFFSET_FROM_BOTTOM

            10000.0f,           // JUMP_MAX_FORCE_MAGNITUDE

            PI() / 9.0f,        // MAX_RUN_STRAIGHT_POSE_ANGLE
            PI() / 18.0f,       // MAX_JUMP_STRAIGHT_POSE_ANGLE
            PI() / 18.0f,       // MAX_JUMP_ANGULAR_SPEED

            2.0f,               // UPPER_JOINT_REDUCTION_SPEED
            0.2f,               // UPPER_JOINT_SLOW_DOWN_DISTANCE
            2000.0f,            // UPPER_JOINT_MAX_FORCE_MAGNITUDE

            PI() / 9.0f,        // SPIN_ANGLE_DELIMITER  (must be < PI()/2)
            PI() / 1.0f,        // SPIN_ANGULAR_SPEED 
            PI() / 18.0f,       // SPIN_SLOW_DOWN_ANGLE
            1000000.0f,         // SPIN_MAX_FORCE_MAGNITUDE
            }

    , m_roller_nid(
            get_blackboard()->m_scene->get_aux_root_node(
                    OBJECT_KIND::AGENT,
                    get_blackboard()->m_self_rid.get_node_id(),
                    "motion_object.roller"
                    )
            )
    , m_body_nid(
            get_blackboard()->m_scene->get_aux_root_node(
                    OBJECT_KIND::AGENT,
                    get_blackboard()->m_self_rid.get_node_id(),
                    "motion_object.body"
                    )
            )

    , m_ccid_lower_joint_roller_and_body{
            get_blackboard()->m_scene->acquire_fresh_custom_constraint_id(),
            get_blackboard()->m_scene->acquire_fresh_custom_constraint_id(),
            get_blackboard()->m_scene->acquire_fresh_custom_constraint_id()
            }
    , m_ccid_upper_joint_roller_and_body{
            get_blackboard()->m_scene->acquire_fresh_custom_constraint_id(),
            get_blackboard()->m_scene->acquire_fresh_custom_constraint_id(),
            get_blackboard()->m_scene->acquire_fresh_custom_constraint_id()
            }
    , m_ccid_body_spin{ get_blackboard()->m_scene->acquire_fresh_custom_constraint_id() }

    , m_roller_frame()
    , m_body_frame()
    , m_agent_frame()
    , m_desire_frame()
    , m_roller_contacts()
    , m_all_contacts()
    , m_seconds_since_last_contact(0.0f)
    , m_environment_linear_velocity(vector3_zero())
    , m_environment_angular_velocity(vector3_zero())
    , m_angle_to_straight_pose(0.0f)
    , m_prev_angle_to_straight_pose(0.0f)
    , m_linear_velocity(vector3_zero())
    , m_angular_velocity(vector3_zero())
{
    {
        angeo::coordinate_system  tmp;
        get_blackboard()->m_scene->get_frame_of_scene_node(get_blackboard()->m_self_rid.get_node_id(), false, tmp);
        m_agent_frame = tmp;
        get_blackboard()->m_scene->get_frame_of_scene_node(get_blackboard()->m_bone_nids.front(), false, tmp);
        m_agent_frame.set_origin(tmp.origin());
    }

    if (!get_blackboard()->m_scene->has_scene_node(m_roller_nid))
    {
        m_roller_frame.set_origin(
                m_agent_frame.origin() -
                (m_config.AGENT_FRAME_ORIGIN_Z_OFFSET_FROM_BOTTOM - m_config.ROLLER_RADIUS) * m_agent_frame.basis_vector_z()
                );
        m_roller_frame.set_basis_vector_x(m_agent_frame.basis_vector_x());
        m_roller_frame.set_basis_vector_y(m_agent_frame.basis_vector_y());
        m_roller_frame.set_basis_vector_z(m_agent_frame.basis_vector_z());

        angeo::coordinate_system  tmp(m_roller_frame);
        get_blackboard()->m_scene->insert_scene_node(m_roller_nid, tmp, false);

        get_blackboard()->m_scene->insert_collision_sphere_to_scene_node(
                m_roller_nid,
                m_config.ROLLER_RADIUS,
                m_config.ROLLER_MATERIAL,
                m_config.ROLLER_COLLISION_CLASS,
                1.0f,
                true
                );
        get_blackboard()->m_scene->insert_rigid_body_to_scene_node(
                m_roller_nid,
                vector3_zero(),
                vector3_zero(),
                vector3_zero(),
                vector3_zero(),
                m_config.ROLLER_MASS_INVERTED,
                matrix33_zero()
                );
        get_blackboard()->m_scene->register_to_collision_contacts_stream(m_roller_nid, get_blackboard()->m_self_id);
    }

    if (!get_blackboard()->m_scene->has_scene_node(m_body_nid))
    {
        m_body_frame.set_origin(
                m_roller_frame.origin() +
                (m_config.ROLLER_RADIUS + m_config.BODY_EXCENTRICITY) * m_roller_frame.basis_vector_z()
                );
        m_body_frame.set_basis_vector_x(m_roller_frame.basis_vector_x());
        m_body_frame.set_basis_vector_y(m_roller_frame.basis_vector_y());
        m_body_frame.set_basis_vector_z(m_roller_frame.basis_vector_z());

        angeo::coordinate_system  tmp(m_body_frame);
        get_blackboard()->m_scene->insert_scene_node(m_body_nid, tmp, false);

        get_blackboard()->m_scene->insert_collision_capsule_to_scene_node(
                m_body_nid,
                m_config.BODY_EXCENTRICITY,
                m_config.ROLLER_RADIUS, // Yes, roller, because it is the same radius used for the capsule too.
                m_config.BODY_MATERIAL,
                m_config.BODY_COLLISION_CLASS,
                1.0f,
                true
                );
        get_blackboard()->m_scene->insert_rigid_body_to_scene_node(
                m_body_nid,
                vector3_zero(),
                vector3_zero(),
                vector3_zero(),
                vector3_zero(),
                m_config.BODY_MASS_INVERTED,
                m_config.BODY_INERTIA_TENSOR_INVERTED
                );
        get_blackboard()->m_scene->register_to_collision_contacts_stream(m_body_nid, get_blackboard()->m_self_id);
    }

    get_blackboard()->m_scene->enable_colliding_colliders_of_scene_nodes(m_roller_nid, m_body_nid, false);

    m_desire_frame = m_roller_frame;
    set_desire_frame();
}


action_controller_roller::~action_controller_roller()
{
    for (scene::custom_constraint_id const  ccid : m_ccid_lower_joint_roller_and_body)
        get_blackboard()->m_scene->release_generated_custom_constraint_id(ccid);
    for (scene::custom_constraint_id const  ccid : m_ccid_upper_joint_roller_and_body)
        get_blackboard()->m_scene->release_generated_custom_constraint_id(ccid);
    get_blackboard()->m_scene->release_generated_custom_constraint_id(m_ccid_body_spin);

    get_blackboard()->m_scene->unregister_from_collision_contacts_stream(m_roller_nid, get_blackboard()->m_self_id);
    get_blackboard()->m_scene->unregister_from_collision_contacts_stream(m_body_nid, get_blackboard()->m_self_id);

    get_blackboard()->m_scene->erase_rigid_body_from_scene_node(m_roller_nid);
    get_blackboard()->m_scene->erase_rigid_body_from_scene_node(m_body_nid);

    get_blackboard()->m_scene->erase_collision_object_from_scene_node(m_roller_nid);
    get_blackboard()->m_scene->erase_collision_object_from_scene_node(m_body_nid);

    get_blackboard()->m_scene->erase_scene_node(m_roller_nid);
    get_blackboard()->m_scene->erase_scene_node(m_body_nid);
}


void  action_controller_roller::synchronise_with_scene()
{
    TMPROF_BLOCK();

    angeo::coordinate_system  tmp;

    get_blackboard()->m_scene->get_frame_of_scene_node(m_roller_nid, false, tmp);
    m_roller_frame = tmp;

    get_blackboard()->m_scene->get_frame_of_scene_node(m_body_nid, false, tmp);
    m_body_frame = tmp;

    m_agent_frame.set_origin(
            m_roller_frame.origin() +
            (m_config.AGENT_FRAME_ORIGIN_Z_OFFSET_FROM_BOTTOM - m_config.ROLLER_RADIUS) * m_body_frame.basis_vector_z()
            );
    m_agent_frame.set_basis_vector_x(m_body_frame.basis_vector_x());
    m_agent_frame.set_basis_vector_y(m_body_frame.basis_vector_y());
    m_agent_frame.set_basis_vector_z(m_body_frame.basis_vector_z());
    get_blackboard()->m_scene->set_frame_of_scene_node(get_blackboard()->m_self_rid.get_node_id(), false, m_agent_frame);

    m_roller_contacts.clear();
    filter_contacts(m_roller_nid, m_roller_contacts);
    m_all_contacts = m_roller_contacts;
    filter_contacts(m_body_nid, m_all_contacts);

    if (!m_all_contacts.empty())
    {
        m_seconds_since_last_contact = 0.0f;
        m_environment_linear_velocity = compute_environment_linear_velocity();
        m_environment_angular_velocity = compute_environment_angular_velocity();
    }
    m_external_linear_acceleration =
            0.5f * (get_blackboard()->m_scene->get_external_linear_acceleration_of_rigid_body_of_scene_node(m_roller_nid) +
                    get_blackboard()->m_scene->get_external_linear_acceleration_of_rigid_body_of_scene_node(m_body_nid) );

    set_desire_frame();

    m_prev_angle_to_straight_pose = m_angle_to_straight_pose;
    m_angle_to_straight_pose = angle(m_body_frame.basis_vector_z(), m_desire_frame.basis_vector_z());

    m_linear_velocity = get_blackboard()->m_scene->get_linear_velocity_of_rigid_body_of_scene_node(m_roller_nid) -
                        get_environment_linear_velocity();
    m_angular_velocity = get_blackboard()->m_scene->get_angular_velocity_of_rigid_body_of_scene_node(m_body_nid) -
                         get_environment_angular_velocity();
}


void  action_controller_roller::apply_desire(
        float_32_bit const  time_step_in_seconds,
        float_32_bit const  desired_ccw_angular_speed,
        vector3 const&  desired_linear_velocity_in_local_space,
        vector3 const&  desired_jump_velocity_in_local_space,
        bool const  disable_upper_joint
        )
{
    TMPROF_BLOCK();

    if (get_seconds_since_last_contact() <= m_config.MAX_NO_CONTACT_SECONDS_FOR_DESIRE_FRAME_UPDATE)
        rotate_desired_frame(time_step_in_seconds, desired_ccw_angular_speed);

    set_roller_angular_velocity(
            get_angle_to_straight_pose() <= m_config.MAX_RUN_STRAIGHT_POSE_ANGLE ?
                    angeo::vector3_from_coordinate_system(desired_linear_velocity_in_local_space, m_desire_frame) :
                    vector3_zero()
            );

    if (!m_roller_contacts.empty()
            && get_angle_to_straight_pose() <= m_config.MAX_JUMP_STRAIGHT_POSE_ANGLE
            && length_squared(desired_jump_velocity_in_local_space) >= 1e-6f
            && std::fabs(m_angle_to_straight_pose - m_prev_angle_to_straight_pose) / time_step_in_seconds
                    <= m_config.MAX_JUMP_ANGULAR_SPEED
            )
        insert_jump_constraints_between_roller_body_and_floor(
                    angeo::vector3_from_coordinate_system(desired_jump_velocity_in_local_space, m_desire_frame)
                    );

    insert_lower_joint_between_roller_and_body();
    if (!disable_upper_joint && !m_all_contacts.empty())
        insert_upper_joint_between_roller_and_body();
    insert_roller_and_body_spin_motor();

    m_seconds_since_last_contact += time_step_in_seconds;
}


// Sub-routines colled from  synchronise_with_scene()


void  action_controller_roller::filter_contacts(scene::node_id const&  nid, std::vector<scene::collicion_contant_info_ptr>&  contacts) const
{
    auto const begin_and_end = 
            get_blackboard()->m_sensory_controller->get_collision_contacts()->get_collision_contacts_map().equal_range(nid);
    for (auto it = begin_and_end.first; it != begin_and_end.second; ++it)
        contacts.push_back(it->second.data);
}


vector3  action_controller_roller::compute_environment_linear_velocity() const
{
    vector3  environment_linear_velocity = vector3_zero();
    for (scene::collicion_contant_info_ptr  info : m_all_contacts)
        environment_linear_velocity +=
                get_blackboard()->m_scene->get_linear_velocity_of_rigid_body_at_point(
                        get_blackboard()->m_scene->get_scene_node_of_rigid_body_associated_with_collider(info->other_coid),
                        m_roller_frame.origin()
                        );
    if (!m_all_contacts.empty())
        environment_linear_velocity /= (float_32_bit)m_all_contacts.size();
    return  environment_linear_velocity;
}


vector3  action_controller_roller::compute_environment_angular_velocity() const
{
    vector3  environment_angular_velocity = vector3_zero();
    for (scene::collicion_contant_info_ptr  info : m_all_contacts)
        environment_angular_velocity +=
                get_blackboard()->m_scene->get_angular_velocity_of_rigid_body_of_scene_node(
                        get_blackboard()->m_scene->get_scene_node_of_rigid_body_associated_with_collider(info->other_coid)
                        );
    if (!m_all_contacts.empty())
        environment_angular_velocity /= (float_32_bit)m_all_contacts.size();
    return  environment_angular_velocity;
}


void  action_controller_roller::set_desire_frame()
{
    float_32_bit const  external_accel_magnitude = length(m_external_linear_acceleration);

    m_desire_frame.set_basis_vector_z(
            external_accel_magnitude < 0.0001f ?
                    m_desire_frame.basis_vector_z() :
                    (-1.0f / external_accel_magnitude) * m_external_linear_acceleration
            );
    m_desire_frame.set_basis_vector_x(normalised(cross_product(m_desire_frame.basis_vector_y(), m_desire_frame.basis_vector_z())));
    m_desire_frame.set_basis_vector_y(normalised(cross_product(m_desire_frame.basis_vector_z(), m_desire_frame.basis_vector_x())));
    m_desire_frame.set_origin(
            m_roller_frame.origin() +
            (2.0f * (m_config.ROLLER_RADIUS + m_config.BODY_EXCENTRICITY) + 0.05f) * m_desire_frame.basis_vector_z()
            );
}


// Sub-routines colled from  apply_desire()


void  action_controller_roller::rotate_desired_frame(float_32_bit const  time_step_in_seconds, float_32_bit const  desired_ccw_angular_speed)
{
    float_32_bit const  environment_angular_velocity_magnitude = length(m_environment_angular_velocity);
    matrix33 const  environment_rotation =
            environment_angular_velocity_magnitude > 0.00001f ?
                    angle_axis_to_rotation_matrix(
                            environment_angular_velocity_magnitude * time_step_in_seconds,
                            (1.0f / environment_angular_velocity_magnitude) * m_environment_angular_velocity
                            ) :
                    matrix33_identity();
    matrix33 const  desired_rotation =
            angle_axis_to_rotation_matrix(desired_ccw_angular_speed * time_step_in_seconds, m_desire_frame.basis_vector_z());

    vector3 const  rotated_y_axis = environment_rotation * desired_rotation * m_desire_frame.basis_vector_y();

    m_desire_frame.set_basis_vector_x(normalised(cross_product(rotated_y_axis, m_desire_frame.basis_vector_z())));
    m_desire_frame.set_basis_vector_y(normalised(cross_product(m_desire_frame.basis_vector_z(), m_desire_frame.basis_vector_x())));
}


void  action_controller_roller::set_roller_angular_velocity(vector3 const&  desired_linear_velocity_in_world_space) const
{
    vector3  angular_velocity;
    {
        angular_velocity = cross_product(m_desire_frame.basis_vector_z(), desired_linear_velocity_in_world_space);
        float_32_bit const  len = length(angular_velocity);
        if (len < 0.0001f)
            angular_velocity = vector3_zero();
        else
        {
            float_32_bit const  angular_speed = length(desired_linear_velocity_in_world_space) / m_config.ROLLER_RADIUS;
            angular_velocity *= angular_speed / len;
        }
    }
    get_blackboard()->m_scene->set_angular_velocity_of_rigid_body_of_scene_node(
            m_roller_nid,
            angular_velocity + m_environment_angular_velocity
            );
}


void  action_controller_roller::insert_jump_constraints_between_roller_body_and_floor(
        vector3 const&  desired_jump_velocity_in_world_space
        ) const
{
    std::unordered_set<scene::node_id>  floor_nids;
    for (scene::collicion_contant_info_ptr  info : m_roller_contacts)
        if (dot_product(info->unit_normal_in_world_space, desired_jump_velocity_in_world_space) > 0.0f)
            floor_nids.insert(get_blackboard()->m_scene->get_scene_node_of_rigid_body_associated_with_collider(info->other_coid));

    float_32_bit const  bias = length(desired_jump_velocity_in_world_space);
    vector3  unit_constraint_vector = (1.0f / bias) * desired_jump_velocity_in_world_space;
    natural_32_bit  ccid_idx = 0U;
    for (scene::node_id const&  floor_nid : floor_nids)
    {
        vector3 const  floor_mass_centre = get_blackboard()->m_scene->get_origin_of_scene_node(floor_nid, false);

        get_blackboard()->m_scene->insert_immediate_constraint(
                m_roller_nid,
                unit_constraint_vector,
                vector3_zero(),
                floor_nid,
                -unit_constraint_vector,
                -cross_product(m_roller_frame.origin() - floor_mass_centre, unit_constraint_vector),
                bias,
                0.0f,
                m_config.JUMP_MAX_FORCE_MAGNITUDE
                );
        get_blackboard()->m_scene->insert_immediate_constraint(
                m_body_nid,
                unit_constraint_vector,
                vector3_zero(),
                floor_nid,
                -unit_constraint_vector,
                -cross_product(m_body_frame.origin() - floor_mass_centre, unit_constraint_vector),
                bias,
                0.0f,
                m_config.JUMP_MAX_FORCE_MAGNITUDE
                );
    }
}


void  action_controller_roller::insert_lower_joint_between_roller_and_body() const
{
    vector3 const  roller_joint = m_roller_frame.origin();
    vector3 const  body_joint =
            m_body_frame.origin() - (m_config.ROLLER_RADIUS + m_config.BODY_EXCENTRICITY) * m_body_frame.basis_vector_z();
    vector3 const  joint_delta = body_joint - roller_joint;
    vector3 const  joint = roller_joint + 0.5f * joint_delta;
    float_32_bit const  joint_separation_distance = length(joint_delta);
    vector3  unit_constraint_vector[3];
    {
        if (joint_separation_distance < 0.0001f)
            unit_constraint_vector[0] = m_desire_frame.basis_vector_z();
        else
            unit_constraint_vector[0] = (1.0f / joint_separation_distance) * joint_delta;
        angeo::compute_tangent_space_of_unit_vector(
                unit_constraint_vector[0],
                unit_constraint_vector[1],
                unit_constraint_vector[2]
                );
    }
    float_32_bit const  bias[3] {
            joint_separation_distance,
            0.0f,
            0.0f
            };
    for (natural_32_bit i = 0U; i != 3U; ++i)
        get_blackboard()->m_scene->insert_custom_constraint(
                m_ccid_lower_joint_roller_and_body[i],
                m_roller_nid,
                unit_constraint_vector[i],
                cross_product(joint - m_roller_frame.origin(), unit_constraint_vector[i]),
                m_body_nid,
                -unit_constraint_vector[i],
                -cross_product(joint - m_body_frame.origin(), unit_constraint_vector[i]),
                bias[i],
                -std::numeric_limits<float_32_bit>::max(),
                std::numeric_limits<float_32_bit>::max()
                );
}


void  action_controller_roller::insert_upper_joint_between_roller_and_body() const
{
    vector3 const  roller_joint =
            m_roller_frame.origin() + 2.0f * (m_config.ROLLER_RADIUS + m_config.BODY_EXCENTRICITY) * m_desire_frame.basis_vector_z();
    vector3 const  body_joint =
            m_body_frame.origin() + (m_config.ROLLER_RADIUS + m_config.BODY_EXCENTRICITY) * m_body_frame.basis_vector_z();
    vector3 const  joint_delta = body_joint - roller_joint;
    vector3 const  joint = roller_joint + 0.5f * joint_delta;
    float_32_bit const  joint_separation_distance = length(joint_delta);
    vector3  unit_constraint_vector[3];
    {
        if (joint_separation_distance < 0.0001f)
            unit_constraint_vector[0] = m_desire_frame.basis_vector_z();
        else
            unit_constraint_vector[0] = (1.0f / joint_separation_distance) * joint_delta;
        angeo::compute_tangent_space_of_unit_vector(
                unit_constraint_vector[0],
                unit_constraint_vector[1],
                unit_constraint_vector[2]
                );
    }
    float_32_bit const  bias[3] {
            m_config.UPPER_JOINT_REDUCTION_SPEED * (joint_separation_distance < m_config.UPPER_JOINT_SLOW_DOWN_DISTANCE ?
                                                            joint_separation_distance / m_config.UPPER_JOINT_SLOW_DOWN_DISTANCE :
                                                            1.0f),
            0.0f,
            0.0f
            };
    for (natural_32_bit i = 0U; i != 3U; ++i)
        get_blackboard()->m_scene->insert_custom_constraint(
                m_ccid_upper_joint_roller_and_body[i],
                m_roller_nid,
                unit_constraint_vector[i],
                vector3_zero(),
                m_body_nid,
                -unit_constraint_vector[i],
                -cross_product(joint - m_body_frame.origin(), unit_constraint_vector[i]),
                bias[i],
                -m_config.UPPER_JOINT_MAX_FORCE_MAGNITUDE,
                m_config.UPPER_JOINT_MAX_FORCE_MAGNITUDE
                );
}


void  action_controller_roller::insert_roller_and_body_spin_motor() const
{
    vector3  body_unit_rot_axis;
    vector3  desired_y_axis;
    {
        body_unit_rot_axis = m_body_frame.basis_vector_z();

        vector3  raw_desired_y_axis;
        {
            float_32_bit const  current_deimiter_angle = angle(m_desire_frame.basis_vector_z(), body_unit_rot_axis);
            if (current_deimiter_angle <= m_config.SPIN_ANGLE_DELIMITER
                    || current_deimiter_angle >= PI() - m_config.SPIN_ANGLE_DELIMITER)
                raw_desired_y_axis = m_desire_frame.basis_vector_y();
            else
                raw_desired_y_axis =
                        (dot_product(m_desire_frame.basis_vector_z(), m_body_frame.basis_vector_y()) >= 0.0f ? 1.0f : -1.0f)
                        * m_desire_frame.basis_vector_z();
        }

        vector3 const  desire_plane_normal = cross_product(body_unit_rot_axis, raw_desired_y_axis);
        desired_y_axis = normalised(cross_product(desire_plane_normal, body_unit_rot_axis));
        if (dot_product(m_body_frame.basis_vector_y(), desire_plane_normal) > 0.0f)
            body_unit_rot_axis = -body_unit_rot_axis;
    }

    float_32_bit  bias;
    {
        float_32_bit const  angle_to_reduce = angle(m_body_frame.basis_vector_y(), desired_y_axis);
        float_32_bit const  coef =
                angle_to_reduce < m_config.SPIN_SLOW_DOWN_ANGLE ? angle_to_reduce / m_config.SPIN_SLOW_DOWN_ANGLE : 1.0f;
        bias = m_config.SPIN_ANGULAR_SPEED * coef;
    }

    float_32_bit constexpr  MAX_FORCE_MAGNITUDE = 1000000.0f;

    get_blackboard()->m_scene->insert_custom_constraint(
            m_ccid_body_spin,
            m_roller_nid,
            vector3_zero(),
            m_desire_frame.basis_vector_z(),
            m_body_nid,
            vector3_zero(),
            body_unit_rot_axis,
            bias,
            -m_config.SPIN_MAX_FORCE_MAGNITUDE,
            m_config.SPIN_MAX_FORCE_MAGNITUDE
            );
}


}}
