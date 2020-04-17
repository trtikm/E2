#ifndef AI_DETAIL_ACTION_CONTROLLER_ROLLER_HPP_INCLUDED
#   define AI_DETAIL_ACTION_CONTROLLER_ROLLER_HPP_INCLUDED

#   include <ai/blackboard_agent.hpp>
#   include <ai/scene.hpp>
#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <array>
#   include <vector>

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

        float_32_bit  JUMP_CONE_ANGLE;
        float_32_bit  JUMP_MAX_FORCE_MAGNITUDE;

        float_32_bit  UPPER_JOINT_REDUCTION_SPEED;
        float_32_bit  UPPER_JOINT_SLOW_DOWN_DISTANCE;
        float_32_bit  UPPER_JOINT_MAX_FORCE_MAGNITUDE;

        float_32_bit  SPIN_ANGLE_DELIMITER; // Must be < PI()/2
        float_32_bit  SPIN_ANGULAR_SPEED;
        float_32_bit  SPIN_SLOW_DOWN_ANGLE;
        float_32_bit  SPIN_MAX_FORCE_MAGNITUDE;
    };

    explicit action_controller_roller(blackboard_agent_weak_ptr const  blackboard_);
    ~action_controller_roller();

    void  next_round(
            bool const  apply_upper_joint,
            vector3 const&  desired_forward_unit_vector_in_local_space,
            vector3 const&  desired_linear_velocity_in_local_space,
            vector3 const&  desired_jump_velocity_in_local_space
            );

    scene::node_id const&  get_roller_nid() const { return m_roller_nid; }
    scene::node_id const&  get_body_nid() const { return m_body_nid; }

    config const&  get_config() const { return m_config; }

private:

    blackboard_agent_ptr  get_blackboard() const { return m_blackboard.lock(); }

    void  synchronise_with_scene();

    void  get_from_agent_frame_matrix(matrix44&  from_agent_frame_matrix) const;

    void  set_desire_frame(vector3 const&  desired_forward_in_world_space);

    void  filter_contacts(scene::node_id const&  nid, std::vector<scene::collicion_contant_info_ptr>&  contacts) const;

    vector3  compute_environment_angular_velocity(std::vector<scene::collicion_contant_info_ptr> const&  contacts) const;

    void  set_roller_angular_velocity(
            vector3 const&  desired_linear_velocity_in_world_space,
            vector3 const&  environment_angular_velocity
            ) const;

    void  insert_jump_constraints_between_roller_body_and_floor(
            vector3 const&  desired_jump_velocity_in_world_space,
            std::vector<scene::collicion_contant_info_ptr> const&  roller_contacts
            );

    void  insert_lower_joint_between_roller_and_body() const;
    void  insert_upper_joint_between_roller_and_body() const;
    void  insert_roller_and_body_spin_motor() const;

    blackboard_agent_weak_ptr  m_blackboard;

    config  m_config;

    scene::node_id  m_roller_nid;
    scene::node_id  m_body_nid;

    angeo::coordinate_system_explicit  m_roller_frame;
    angeo::coordinate_system_explicit  m_body_frame;
    angeo::coordinate_system_explicit  m_desire_frame;

    std::array<scene::custom_constraint_id, 3>  m_ccid_lower_joint_roller_and_body;
    std::array<scene::custom_constraint_id, 3>  m_ccid_upper_joint_roller_and_body;
    scene::custom_constraint_id  m_ccid_body_spin;
};


}}

#endif
