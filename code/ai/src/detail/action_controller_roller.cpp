#include <ai/detail/action_controller_roller.hpp>
#include <ai/skeletal_motion_templates.hpp>
#include <angeo/utility.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>

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

            PI() / 18.0f,       // JUMP_CONE_ANGLE
            5000.0f,            // JUMP_MAX_FORCE_MAGNITUDE

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
                    "motion_object.roller"
                    )
            )

    , m_roller_frame()
    , m_body_frame()
    , m_desire_frame()

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
{
    vector3  origin, forward, up;
    {
        angeo::coordinate_system  agent_frame;
        {
            angeo::coordinate_system  tmp;
            get_blackboard()->m_scene->get_frame_of_scene_node(get_blackboard()->m_bone_nids.front(), false, tmp);
            agent_frame.set_origin(tmp.origin());
            get_blackboard()->m_scene->get_frame_of_scene_node(get_blackboard()->m_self_rid.get_node_id(), false, tmp);
            agent_frame.set_orientation(tmp.orientation());
        }

        origin = agent_frame.origin();

        matrix44  W;
        angeo::from_base_matrix(agent_frame, W);

        forward = transform_vector(get_blackboard()->m_motion_templates.directions().forward(), W);
        up = transform_vector(get_blackboard()->m_motion_templates.directions().up(), W);
    }

    if (!get_blackboard()->m_scene->has_scene_node(m_roller_nid))
    {
        m_roller_frame.set_origin(origin - (0.5f * m_config.ROLLER_RADIUS + m_config.BODY_EXCENTRICITY) * up);
        m_roller_frame.set_basis_vector_y(-forward);
        m_roller_frame.set_basis_vector_z(up);
        m_roller_frame.set_basis_vector_x(normalised(cross_product(m_roller_frame.basis_vector_y(), m_roller_frame.basis_vector_z())));

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
        m_body_frame.set_origin(origin + (0.5f * m_config.ROLLER_RADIUS) * up);
        m_body_frame.set_basis_vector_y(-forward);
        m_body_frame.set_basis_vector_z(up);
        m_body_frame.set_basis_vector_x(normalised(cross_product(m_body_frame.basis_vector_y(), m_body_frame.basis_vector_z())));

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

    set_desire_frame(forward);
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
    get_blackboard()->m_scene->erase_collision_object_from_scene_node(m_body_nid);

    get_blackboard()->m_scene->erase_scene_node(m_roller_nid);
    get_blackboard()->m_scene->erase_scene_node(m_body_nid);
}


void  action_controller_roller::next_round(
        float_32_bit const  time_step_in_seconds,
        vector3 const&  desired_forward_unit_vector_in_local_space,
        vector3 const&  desired_linear_velocity_in_local_space,
        vector3 const&  desired_jump_velocity_in_local_space,
        std::vector<scene::collicion_contant_info> const&  collision_contacts
        )
{
    TMPROF_BLOCK();

    {
        angeo::coordinate_system  tmp;

        get_blackboard()->m_scene->get_frame_of_scene_node(m_roller_nid, false, tmp);
        m_roller_frame = tmp;

        get_blackboard()->m_scene->get_frame_of_scene_node(m_body_nid, false, tmp);
        m_body_frame = tmp;
    }

    matrix44  from_agent_frame_matrix;
    {
        angeo::coordinate_system  agent_frame;
        get_blackboard()->m_scene->get_frame_of_scene_node(get_blackboard()->m_self_rid.get_node_id(), false, agent_frame);
        angeo::from_base_matrix(agent_frame, from_agent_frame_matrix);
    }

    set_desire_frame(transform_vector(desired_forward_unit_vector_in_local_space, from_agent_frame_matrix));

    std::vector<scene::collicion_contant_info>  contacts;
    filter_contacts(collision_contacts, contacts);

    std::vector<scene::collicion_contant_info>  foot_contacts;
    get_foot_contacts(contacts, foot_contacts);

    set_roller_angular_velocity(
            transform_vector(desired_linear_velocity_in_local_space, from_agent_frame_matrix),
            compute_environment_angular_velocity(foot_contacts)
            );

    if (!foot_contacts.empty()
            && length_squared(desired_jump_velocity_in_local_space) >= 1e-6f
            && angle(m_body_frame.basis_vector_z(), m_desire_frame.basis_vector_z()) <= m_config.JUMP_CONE_ANGLE)
        insert_jump_constraints_between_roller_body_and_floor(
                    transform_vector(desired_jump_velocity_in_local_space, from_agent_frame_matrix),
                    foot_contacts
                    );

    insert_lower_joint_between_roller_and_body();
    if (!contacts.empty())
        insert_upper_joint_between_roller_and_body();
    insert_roller_and_body_spin_motor();
}


void  action_controller_roller::set_desire_frame(vector3 const&  desired_forward_in_world_space)
{
    vector3 const  total_external_accel =
            get_blackboard()->m_scene->get_external_linear_acceleration_of_rigid_body_of_scene_node(m_roller_nid) +
            get_blackboard()->m_scene->get_external_linear_acceleration_of_rigid_body_of_scene_node(m_body_nid)
            ;
    m_desire_frame.set_basis_vector_z(-normalised(total_external_accel));
    m_desire_frame.set_basis_vector_x(normalised(cross_product(desired_forward_in_world_space, m_desire_frame.basis_vector_z())));
    m_desire_frame.set_basis_vector_y(normalised(cross_product(m_desire_frame.basis_vector_z(), m_desire_frame.basis_vector_x())));
    m_desire_frame.set_origin(
            m_roller_frame.origin() + 2.0f * (m_config.ROLLER_RADIUS + m_config.BODY_EXCENTRICITY) * m_desire_frame.basis_vector_z()
            );
}


void  action_controller_roller::filter_contacts(
        std::vector<scene::collicion_contant_info> const&  src_contacts,
        std::vector<scene::collicion_contant_info>&  filtered_contacts   // cannot alias with src_contacts!
        ) const
{
    for (auto const&  contact : src_contacts)
    {
        scene::node_id const  self_nid =
                get_blackboard()->m_scene->get_scene_node_of_rigid_body_associated_with_collider(contact.self_coid);
        if (!self_nid.valid())
            continue;
        scene::node_id const  other_nid =
                get_blackboard()->m_scene->get_scene_node_of_rigid_body_associated_with_collider(contact.other_coid);
        if (!other_nid.valid())
            continue;
        if (self_nid == m_roller_nid || self_nid == m_body_nid)
            filtered_contacts.push_back(contact);
        else if (other_nid == m_roller_nid || other_nid == m_body_nid)
            filtered_contacts.push_back({
                contact.contact_point_in_world_space,
                -contact.unit_normal_in_world_space,
                0.0f,
                contact.other_coid,
                contact.other_material,
                contact.self_coid,
                contact.self_material
                });
    }
}


void  action_controller_roller::get_foot_contacts(
        std::vector<scene::collicion_contant_info> const&  src_contacts,
        std::vector<scene::collicion_contant_info>&  foot_contacts
        ) const
{
    for (auto const& contact : src_contacts)
        if (get_blackboard()->m_scene->get_scene_node_of_rigid_body_associated_with_collider(contact.self_coid) == m_roller_nid)
            foot_contacts.push_back(contact);
}


vector3  action_controller_roller::compute_environment_angular_velocity(
        std::vector<scene::collicion_contant_info> const&  contacts
        ) const
{
    vector3  environment_angular_velocity = vector3_zero();
    for (scene::collicion_contant_info const&  info : contacts)
        environment_angular_velocity +=
                get_blackboard()->m_scene->get_angular_velocity_of_rigid_body_of_scene_node(
                        get_blackboard()->m_scene->get_scene_node_of_rigid_body_associated_with_collider(info.other_coid)
                        );
    if (!contacts.empty())
        environment_angular_velocity /= (float_32_bit)contacts.size();
    return  environment_angular_velocity;
}


void  action_controller_roller::set_roller_angular_velocity(
        vector3 const&  desired_linear_velocity_in_world_space,
        vector3 const&  environment_angular_velocity
        ) const
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
            angular_velocity + environment_angular_velocity
            );
}


void  action_controller_roller::insert_jump_constraints_between_roller_body_and_floor(
        vector3 const&  desired_jump_velocity_in_world_space,
        std::vector<scene::collicion_contant_info> const&  foot_contacts
        )
{
    std::unordered_set<scene::node_id>  floor_nids;
    for (scene::collicion_contant_info const&  info : foot_contacts)
        if (dot_product(info.contact_point_in_world_space, desired_jump_velocity_in_world_space) > 0.0f)
            floor_nids.insert(get_blackboard()->m_scene->get_scene_node_of_rigid_body_associated_with_collider(info.other_coid));

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
