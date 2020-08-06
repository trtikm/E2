#ifndef AI_DETAIL_ACTION_CONTROLLER_ROLLER_HPP_INCLUDED
#   define AI_DETAIL_ACTION_CONTROLLER_ROLLER_HPP_INCLUDED

#   include <ai/skeletal_motion_templates.hpp>
#   include <ai/scene_binding.hpp>
#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <angeo/custom_constraint_id.hpp>
#   include <angeo/collision_material.hpp>
#   include <angeo/collision_class.hpp>
#   include <com/object_guid.hpp>
#   include <com/collision_contact.hpp>
#   include <array>
#   include <vector>
#   include <string>

namespace ai { namespace detail {


struct  action_controller_roller  final
{
    struct  config
    {
        // Roller is always a sphere with zero inverted inertia tensor.
        float_32_bit  ROLLER_RADIUS;
        float_32_bit  ROLLER_MASS_INVERTED;
        angeo::COLLISION_MATERIAL_TYPE  ROLLER_MATERIAL;
        angeo::COLLISION_CLASS  ROLLER_COLLISION_CLASS;

        // Body can be any shape, but currently only a capsule is assumed with
        // the radius of the capsule is assumed to be same as of the roller's sphere.
        float_32_bit  BODY_EXCENTRICITY;
        float_32_bit  BODY_MASS_INVERTED;
        matrix33  BODY_INERTIA_TENSOR_INVERTED;
        angeo::COLLISION_MATERIAL_TYPE  BODY_MATERIAL;
        angeo::COLLISION_CLASS  BODY_COLLISION_CLASS;

        float_32_bit  MAX_NO_CONTACT_SECONDS_FOR_DESIRE_FRAME_UPDATE;

        float_32_bit  AGENT_FRAME_ORIGIN_Z_OFFSET_FROM_BOTTOM;

        float_32_bit  JUMP_MAX_FORCE_MAGNITUDE;

        float_32_bit  MAX_RUN_STRAIGHT_POSE_ANGLE;
        float_32_bit  MAX_JUMP_STRAIGHT_POSE_ANGLE;
        float_32_bit  MAX_JUMP_ANGULAR_SPEED;

        float_32_bit  UPPER_JOINT_REDUCTION_SPEED;
        float_32_bit  UPPER_JOINT_SLOW_DOWN_DISTANCE;
        float_32_bit  UPPER_JOINT_MAX_FORCE_MAGNITUDE;

        float_32_bit  SPIN_ANGLE_DELIMITER; // Must be < PI()/2
        float_32_bit  SPIN_ANGULAR_SPEED;
        float_32_bit  SPIN_SLOW_DOWN_ANGLE;
        float_32_bit  SPIN_MAX_FORCE_MAGNITUDE;
    };

    action_controller_roller(
            skeletal_motion_templates const  motion_templates,
            scene_binding_ptr const  binding
            );
    ~action_controller_roller();

    // In each simulation round, call this method before 'apply_desire'
    void  synchronise_with_scene();

    // In each simulation round, call this method after 'synchronise_with_scene'
    void  apply_desire(
            float_32_bit const  time_step_in_seconds,
            float_32_bit const  desired_ccw_angular_speed,
            vector3 const&  desired_linear_velocity_in_local_space,
            vector3 const&  desired_jump_velocity_in_local_space,
            bool const  disable_upper_joint = false
            );

    config const&  get_config() const { return m_config; }
    skeletal_motion_templates  get_motion_templates() const { return m_motion_templates; }
    scene_binding_ptr  get_binding() const { return m_binding; }
    com::simulation_context const&  ctx() const { return *m_binding->context; }
    std::vector<com::object_guid> const&  get_frame_guids_of_bones() const { return m_binding->frame_guids_of_bones; }

    std::string  get_name() const { return "ROLLER"; }

    angeo::coordinate_system_explicit const&  get_roller_frame() const { return m_skeleton_frame; }
    angeo::coordinate_system_explicit const&  get_body_frame() const { return m_skeleton_frame; }
    angeo::coordinate_system_explicit const&  get_skeleton_frame() const { return m_skeleton_frame; }
    angeo::coordinate_system_explicit const&  get_desire_frame() const { return m_skeleton_frame; }

    std::vector<com::collision_contact const*> const&  get_roller_contacts() const { return m_roller_contacts; }
    std::vector<com::collision_contact const*> const&  get_all_contacts() const { return m_all_contacts; }
    float_32_bit  get_seconds_since_last_contact() const { return m_seconds_since_last_contact; }

    vector3 const&  get_environment_linear_velocity() const { return m_environment_linear_velocity; }
    vector3 const&  get_environment_angular_velocity() const { return m_environment_angular_velocity; }
    vector3 const&  get_external_linear_acceleration() const { return m_external_linear_acceleration; }

    float_32_bit  get_angle_to_straight_pose() const { return m_angle_to_straight_pose; }
    vector3  get_spin_axis() const { return m_body_frame.basis_vector_z(); }

    vector3  get_linear_velocity() const { return m_linear_velocity; }
    vector3  get_angular_velocity() const { return m_angular_velocity; }

private:

    // Sub-routines colled from  synchronise_with_scene()
    void  filter_contacts(com::object_guid const  guid, std::vector<com::collision_contact const*>&  contacts) const;
    vector3  compute_environment_linear_velocity() const;
    vector3  compute_environment_angular_velocity() const;
    void  set_desire_frame();

    // Sub-routines colled from  apply_desire()
    void  rotate_desired_frame(float_32_bit const  time_step_in_seconds, float_32_bit const  desired_ccw_angular_speed);
    void  set_roller_angular_velocity(vector3 const&  desired_linear_velocity_in_world_space) const;
    void  insert_jump_constraints_between_roller_body_and_floor(vector3 const&  desired_jump_velocity_in_world_space) const;
    void  insert_lower_joint_between_roller_and_body() const;
    void  insert_upper_joint_between_roller_and_body() const;
    void  insert_roller_and_body_spin_motor() const;

    // --- DATA ------------------------------

    config  m_config;
    skeletal_motion_templates  m_motion_templates;
    scene_binding_ptr  m_binding;

    com::object_guid  m_root_folder_guid;

    com::object_guid  m_roller_folder_guid;
    com::object_guid  m_roller_frame_guid;
    com::object_guid  m_roller_rigid_body_guid;
    com::object_guid  m_roller_collider_guid;

    com::object_guid  m_body_folder_guid;
    com::object_guid  m_body_frame_guid;
    com::object_guid  m_body_rigid_body_guid;
    com::object_guid  m_body_collider_guid;

    com::object_guid  m_skeleton_sync_folder_guid;
    com::object_guid  m_skeleton_sync_frame_guid;

    // These constraint ids are persistent (not-changed) during whole life time of the roller instance.
    std::array<angeo::custom_constraint_id, 3>  m_ccid_lower_joint_roller_and_body;
    std::array<angeo::custom_constraint_id, 3>  m_ccid_upper_joint_roller_and_body;
    angeo::custom_constraint_id  m_ccid_body_spin;

    // The following data must be synchronised with scene, before apply_desire() is called.
    angeo::coordinate_system_explicit  m_roller_frame;
    angeo::coordinate_system_explicit  m_body_frame;
    angeo::coordinate_system_explicit  m_skeleton_frame;
    angeo::coordinate_system_explicit  m_desire_frame;
    std::vector<com::collision_contact const*>  m_roller_contacts;
    std::vector<com::collision_contact const*>  m_all_contacts;
    float_32_bit  m_seconds_since_last_contact;
    vector3  m_environment_linear_velocity;
    vector3  m_environment_angular_velocity;
    vector3  m_external_linear_acceleration;
    float_32_bit  m_angle_to_straight_pose;
    float_32_bit  m_prev_angle_to_straight_pose;
    vector3  m_linear_velocity;
    vector3  m_angular_velocity;
};


}}

#endif
