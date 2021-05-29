#include <ai/detail/action_controller_roller.hpp>
#include <angeo/utility.hpp>
#include <com/simulation_context.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <unordered_map>

namespace ai { namespace detail {


action_controller_roller::action_controller_roller(
        skeletal_motion_templates const  motion_templates,
        scene_binding_ptr const  binding
        )
    : m_config {
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

            50000.0f,           // JUMP_MAX_FORCE_MAGNITUDE

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
    , m_motion_templates(motion_templates)
    , m_binding(binding)

    , m_root_folder_guid(m_binding->context->insert_folder(m_binding->folder_guid_of_agent, "roller_root", false))

    , m_roller_folder_guid(m_binding->context->insert_folder(m_root_folder_guid, "roller", false))
    , m_roller_frame_guid(com::invalid_object_guid())
    , m_roller_rigid_body_guid(com::invalid_object_guid())
    , m_roller_collider_guid(com::invalid_object_guid())

    , m_body_folder_guid(m_binding->context->insert_folder(m_root_folder_guid, "body", false))
    , m_body_frame_guid(com::invalid_object_guid())
    , m_body_rigid_body_guid(com::invalid_object_guid())
    , m_body_collider_guid(com::invalid_object_guid())

    , m_skeleton_sync_folder_guid(com::invalid_object_guid())
    , m_skeleton_sync_frame_guid(com::invalid_object_guid())

    , m_ccid_lower_joint_roller_and_body{
            m_binding->context->acquire_fresh_custom_constraint_id_from_physics(),
            m_binding->context->acquire_fresh_custom_constraint_id_from_physics(),
            m_binding->context->acquire_fresh_custom_constraint_id_from_physics()
            }
    , m_ccid_upper_joint_roller_and_body{
            m_binding->context->acquire_fresh_custom_constraint_id_from_physics(),
            m_binding->context->acquire_fresh_custom_constraint_id_from_physics(),
            m_binding->context->acquire_fresh_custom_constraint_id_from_physics()
            }
    , m_ccid_body_spin{ m_binding->context->acquire_fresh_custom_constraint_id_from_physics() }

    , m_roller_frame()
    , m_body_frame()
    , m_skeleton_frame(ctx().frame_explicit_coord_system_in_world_space(get_binding()->frame_guid_of_skeleton))
    , m_desire_frame()
    , m_roller_contacts()
    , m_body_contacts()
    , m_seconds_since_last_contact(0.0f)
    , m_environment_linear_velocity(vector3_zero())
    , m_environment_angular_velocity(vector3_zero())
    , m_external_linear_acceleration(vector3_zero())
    , m_angle_to_straight_pose(0.0f)
    , m_prev_angle_to_straight_pose(0.0f)
    , m_linear_velocity(vector3_zero())
    , m_angular_velocity(vector3_zero())
{
    m_roller_frame_guid = m_binding->context->insert_frame(
            m_roller_folder_guid,
            com::invalid_object_guid(),
            m_skeleton_frame.origin() + m_config.ROLLER_RADIUS * m_skeleton_frame.basis_vector_z(),
            ctx().frame_coord_system_in_world_space(get_binding()->frame_guid_of_skeleton).orientation()
            );
    m_roller_rigid_body_guid = m_binding->context->insert_rigid_body(
            m_roller_folder_guid,
            true,
            vector3_zero(),
            vector3_zero(),
            vector3_zero(),
            vector3_zero(),
            m_config.ROLLER_MASS_INVERTED,
            matrix33_zero()
            );
    m_roller_collider_guid = m_binding->context->insert_collider_sphere(
            m_roller_folder_guid,
            "COLLIDER.roller_sphere",
            m_config.ROLLER_RADIUS,
            m_config.ROLLER_MATERIAL,
            m_config.ROLLER_COLLISION_CLASS,
            0U
            );

    m_roller_frame = ctx().frame_explicit_coord_system_in_world_space(m_roller_frame_guid);

    m_body_frame_guid = m_binding->context->insert_frame(
            m_body_folder_guid,
            com::invalid_object_guid(),
            m_roller_frame.origin() + (m_config.ROLLER_RADIUS + m_config.BODY_EXCENTRICITY) * m_roller_frame.basis_vector_z(),
            ctx().frame_coord_system_in_world_space(m_roller_frame_guid).orientation()
            );
    m_body_rigid_body_guid = m_binding->context->insert_rigid_body(
            m_body_folder_guid,
            true,
            vector3_zero(),
            vector3_zero(),
            vector3_zero(),
            vector3_zero(),
            m_config.BODY_MASS_INVERTED,
            m_config.BODY_INERTIA_TENSOR_INVERTED
            );
    m_body_collider_guid = m_binding->context->insert_collider_capsule(
            m_body_folder_guid,
            "COLLIDER.body_capsule",
            m_config.BODY_EXCENTRICITY,
            m_config.ROLLER_RADIUS, // Yes, roller, because it is the same radius used for the capsule too.
            m_config.BODY_MATERIAL,
            m_config.BODY_COLLISION_CLASS,
            0U
            );

    ctx().request_enable_colliding(m_roller_collider_guid, m_body_collider_guid, false);

    m_body_frame = ctx().frame_explicit_coord_system_in_world_space(m_body_frame_guid);

    m_desire_frame = m_roller_frame;

    m_skeleton_sync_folder_guid = m_binding->context->insert_folder(m_body_folder_guid, "skeleton_sync_folder", false);
    m_skeleton_sync_frame_guid = m_binding->context->insert_frame(
            m_skeleton_sync_folder_guid,
            m_body_frame_guid,
            (m_config.AGENT_FRAME_ORIGIN_Z_OFFSET_FROM_BOTTOM - (2.0f * m_config.ROLLER_RADIUS + m_config.BODY_EXCENTRICITY))
                    * vector3_unit_z(),
            quaternion_identity()
            );

    ctx().request_set_parent_frame(get_binding()->frame_guid_of_skeleton, m_skeleton_sync_frame_guid);
    ctx().request_relocate_frame(get_binding()->frame_guid_of_skeleton, vector3_zero(), quaternion_identity());

    set_desire_frame();
}


action_controller_roller::~action_controller_roller()
{
    for (angeo::custom_constraint_id const  ccid : m_ccid_lower_joint_roller_and_body)
        ctx().release_acquired_custom_constraint_id_back_to_physics(ccid);
    for (angeo::custom_constraint_id const  ccid : m_ccid_upper_joint_roller_and_body)
        ctx().release_acquired_custom_constraint_id_back_to_physics(ccid);
    ctx().release_acquired_custom_constraint_id_back_to_physics(m_ccid_body_spin);

    if (ctx().is_valid_frame_guid(get_binding()->frame_guid_of_skeleton))
        ctx().request_set_parent_frame(get_binding()->frame_guid_of_skeleton, com::invalid_object_guid());
    if (ctx().is_valid_frame_guid(m_root_folder_guid))
        ctx().request_erase_non_root_folder(m_root_folder_guid);
}


void  action_controller_roller::synchronise_with_scene()
{
    TMPROF_BLOCK();

    m_roller_frame = ctx().frame_explicit_coord_system_in_world_space(ctx().frame_of_rigid_body(m_roller_rigid_body_guid));
    m_body_frame = ctx().frame_explicit_coord_system_in_world_space(ctx().frame_of_rigid_body(m_body_rigid_body_guid));
    m_skeleton_frame = ctx().frame_explicit_coord_system_in_world_space(get_binding()->frame_guid_of_skeleton);

    m_roller_contacts.clear();
    filter_contacts(m_roller_collider_guid, m_roller_contacts);
    m_body_contacts.clear();
    filter_contacts(m_body_collider_guid, m_body_contacts);

    if (!m_roller_contacts.empty() || !m_body_contacts.empty())
    {
        m_seconds_since_last_contact = 0.0f;
        m_environment_linear_velocity = compute_environment_linear_velocity();
        m_environment_angular_velocity = compute_environment_angular_velocity();
    }
    m_external_linear_acceleration =
            0.5f * (ctx().linear_acceleration_of_rigid_body(m_roller_rigid_body_guid) +
                    ctx().linear_acceleration_of_rigid_body(m_body_rigid_body_guid));

    set_desire_frame();

    m_prev_angle_to_straight_pose = m_angle_to_straight_pose;
    m_angle_to_straight_pose = angle(m_body_frame.basis_vector_z(), m_desire_frame.basis_vector_z());

    m_linear_velocity = ctx().linear_velocity_of_rigid_body(m_roller_rigid_body_guid) - get_environment_linear_velocity();
    m_angular_velocity = ctx().angular_velocity_of_rigid_body(m_body_rigid_body_guid) - get_environment_angular_velocity();
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
            && std::fabs(m_angle_to_straight_pose - m_prev_angle_to_straight_pose) / time_step_in_seconds
                    <= m_config.MAX_JUMP_ANGULAR_SPEED
            )
        insert_jump_constraints_between_roller_body_and_floor(
                    angeo::vector3_from_coordinate_system(desired_jump_velocity_in_local_space, m_desire_frame)
                    );

    insert_lower_joint_between_roller_and_body();
    if (!disable_upper_joint && (!m_roller_contacts.empty() || !m_body_contacts.empty()))
        insert_upper_joint_between_roller_and_body();
    insert_roller_and_body_spin_motor();

    m_seconds_since_last_contact += time_step_in_seconds;
}


// Sub-routines colled from  synchronise_with_scene()


void  action_controller_roller::filter_contacts(
        com::object_guid const  collider_guid,
        std::vector<com::collision_contact const*>&  contacts
        ) const
{
    for (natural_32_bit  contact_idx : ctx().collision_contacts_of_collider(collider_guid))
    {
        com::collision_contact const* const  contact_ptr = &ctx().get_collision_contact(contact_idx);
        switch (ctx().collision_class_of(contact_ptr->other_collider(collider_guid)))
        {
        case angeo::COLLISION_CLASS::STATIC_OBJECT:
        case angeo::COLLISION_CLASS::COMMON_MOVEABLE_OBJECT:
        case angeo::COLLISION_CLASS::HEAVY_MOVEABLE_OBJECT:
            contacts.push_back(contact_ptr);
            break;
        default:
            break;
        }
    }
}


vector3  action_controller_roller::compute_environment_linear_velocity() const
{
    vector3  environment_linear_velocity = vector3_zero();
    natural_32_bit const  num_contacts = (natural_32_bit)(m_roller_contacts.size() + m_body_contacts.size());
    if (num_contacts > 0U)
    {
        for (com::collision_contact const*  info : m_roller_contacts)
            environment_linear_velocity += ctx().compute_velocity_of_point_of_rigid_body(
                    ctx().rigid_body_of_collider(info->other_collider(m_roller_collider_guid)),
                    m_roller_frame.origin()
                    );
        for (com::collision_contact const*  info : m_body_contacts)
            environment_linear_velocity += ctx().compute_velocity_of_point_of_rigid_body(
                    ctx().rigid_body_of_collider(info->other_collider(m_body_collider_guid)),
                    m_body_frame.origin()
                    );
        environment_linear_velocity /= (float_32_bit)num_contacts;
    }
    return  environment_linear_velocity;
}


vector3  action_controller_roller::compute_environment_angular_velocity() const
{
    vector3  environment_angular_velocity = vector3_zero();
    natural_32_bit const  num_contacts = (natural_32_bit)(m_roller_contacts.size() + m_body_contacts.size());
    if (num_contacts > 0U)
    {
        for (com::collision_contact const*  info : m_roller_contacts)
            environment_angular_velocity += ctx().angular_velocity_of_rigid_body(
                    ctx().rigid_body_of_collider(info->other_collider(m_roller_collider_guid))
                    );
        for (com::collision_contact const*  info : m_body_contacts)
            environment_angular_velocity += ctx().angular_velocity_of_rigid_body(
                    ctx().rigid_body_of_collider(info->other_collider(m_body_collider_guid))
                    );
        environment_angular_velocity /= (float_32_bit)num_contacts;
    }
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
    ctx().request_set_rigid_body_angular_velocity(m_roller_rigid_body_guid, angular_velocity + m_environment_angular_velocity);
}


void  action_controller_roller::insert_jump_constraints_between_roller_body_and_floor(
        vector3 const&  desired_jump_velocity_in_world_space
        ) const
{
    float_32_bit const  jump_velocity_magnitude = length(desired_jump_velocity_in_world_space);
    if (jump_velocity_magnitude < 0.001f)
        return;

    std::unordered_map<com::object_guid, float_32_bit>  floor_guids;
    for (com::collision_contact const*  info : m_roller_contacts)
    {
        float_32_bit const  cosangle =
            dot_product(info->unit_normal(m_roller_collider_guid), desired_jump_velocity_in_world_space) / jump_velocity_magnitude;
        if (cosangle > 0.5f) // 0.5 == cos(60deg)
            floor_guids.insert({ info->other_collider(m_roller_collider_guid), cosangle });
    }
    if (floor_guids.empty())
        return;

    vector3  unit_constraint_vector = (1.0f / jump_velocity_magnitude) * desired_jump_velocity_in_world_space;
    for (auto  it = floor_guids.begin(); it != floor_guids.end(); ++it)
    {
        vector3 const  floor_mass_centre = ctx().frame_coord_system_in_world_space(ctx().frame_of_collider(it->first)).origin();
        com::object_guid const  floor_rb_guid = ctx().rigid_body_of_collider(it->first);
        float_32_bit const  bias = jump_velocity_magnitude * it->second;

        ctx().request_early_insertion_of_instant_constraint_to_physics(
                m_roller_rigid_body_guid,
                unit_constraint_vector,
                vector3_zero(),
                floor_rb_guid,
                -unit_constraint_vector,
                -cross_product(m_roller_frame.origin() - floor_mass_centre, unit_constraint_vector),
                bias,
                0.0f,
                m_config.JUMP_MAX_FORCE_MAGNITUDE,
                0.0f
                );
        ctx().request_early_insertion_of_instant_constraint_to_physics(
                m_body_rigid_body_guid,
                unit_constraint_vector,
                vector3_zero(),
                floor_rb_guid,
                -unit_constraint_vector,
                -cross_product(m_body_frame.origin() - floor_mass_centre, unit_constraint_vector),
                bias,
                0.0f,
                m_config.JUMP_MAX_FORCE_MAGNITUDE,
                0.0f
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
            10.0f * joint_separation_distance,
            0.0f,
            0.0f
            };
    for (natural_32_bit i = 0U; i != 3U; ++i)
        ctx().request_early_insertion_of_custom_constraint_to_physics(
                m_ccid_lower_joint_roller_and_body[i],
                m_roller_rigid_body_guid,
                unit_constraint_vector[i],
                cross_product(joint - m_roller_frame.origin(), unit_constraint_vector[i]),
                m_body_rigid_body_guid,
                -unit_constraint_vector[i],
                -cross_product(joint - m_body_frame.origin(), unit_constraint_vector[i]),
                bias[i],
                -std::numeric_limits<float_32_bit>::max(),
                std::numeric_limits<float_32_bit>::max(),
                0.0f
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
        ctx().request_early_insertion_of_custom_constraint_to_physics(
                m_ccid_upper_joint_roller_and_body[i],
                m_roller_rigid_body_guid,
                unit_constraint_vector[i],
                vector3_zero(),
                m_body_rigid_body_guid,
                -unit_constraint_vector[i],
                -cross_product(joint - m_body_frame.origin(), unit_constraint_vector[i]),
                bias[i],
                -m_config.UPPER_JOINT_MAX_FORCE_MAGNITUDE,
                m_config.UPPER_JOINT_MAX_FORCE_MAGNITUDE,
                0.0f
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

    ctx().request_early_insertion_of_custom_constraint_to_physics(
            m_ccid_body_spin,
            m_roller_rigid_body_guid,
            vector3_zero(),
            m_desire_frame.basis_vector_z(),
            m_body_rigid_body_guid,
            vector3_zero(),
            body_unit_rot_axis,
            bias,
            -m_config.SPIN_MAX_FORCE_MAGNITUDE,
            m_config.SPIN_MAX_FORCE_MAGNITUDE,
            0.0f
            );
}


}}
