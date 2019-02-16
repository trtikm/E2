#ifndef ANGEO_SKELETON_KINEMATICS_HPP_INCLUDED
#   define ANGEO_SKELETON_KINEMATICS_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <vector>
#   include <unordered_map>

namespace angeo {


struct  joint_rotation_props
{
    // definition of the rotation axis
    vector3  m_axis;                    // unit vector
    bool  m_axis_in_parent_space;       // true if 'm_axis' is expressed in bone's parent cood system.
    float_32_bit  m_max_angular_speed;  // in rad/s; must be a positive number.

    // definition of rotation angle around the axis (see function 'compute_angle_default' for interpatation of members):
    vector3  m_zero_angle_direction;    // unit vector; in bone's parent cood system.
    vector3  m_direction;               // unit vector; in bone's cood system.

    float_32_bit  m_max_angle;

    float_32_bit  m_convergence_coef;   // in range (0.0, 1.0>; how quickly the joint should converge to target angle computed a skeletal algo.
};


using  bone_look_at_targets =
            std::unordered_map< integer_32_bit,     // bone index
                                std::pair<vector3,  // "eye", i.e. a unit vector in bone's coord. system space, to look at "target".
                                          vector3   // "target", i.e. a vector in world space, the "eye" should look at.
            > >;


void  skeleton_look_at(
        std::vector<coordinate_system>&  output_frames,     // output coordinate systems of bones rotated so that they look at the target.
        bone_look_at_targets const&  look_at_targets,       // the targets to look at.
        std::vector<coordinate_system> const&  pose_frames, // pose coordinate systems of bones, i.e. in a neutral position from which to start the look at algo; DO NOT PASS THE CURRENT COORDINATE SYSTEMS OF BONES.
        std::vector<integer_32_bit> const&  parents,        // value -1 at index 'i' means, the bone 'i' does not have a parent
        std::vector<std::vector<integer_32_bit> > const&  children, // use function 'compute_rotation_angle' to compute children from parents.
        std::vector<std::vector<joint_rotation_props> > const&  rotation_props, // specification of rotation props of each bone at joint to its parent bone.
        natural_32_bit const  num_sub_iterations = 1U
        );


float_32_bit  clip_all_joint_rotations_to_allowed_limits(
        coordinate_system&  frame,
        std::vector<joint_rotation_props> const&  rotation_props,
        std::vector<float_32_bit> const&  start_angles,
        coordinate_system const&  target_frame
        );


float_32_bit  clip_joint_rotation_to_allowed_limits(
        coordinate_system&  frame,
        joint_rotation_props const&  props,
        float_32_bit const  start_angle,
        coordinate_system const&  target_frame
        );


/// Given a parent for a bone (-1 when no parent), the function computes children the bone.
void  skeleton_compute_child_bones(std::vector<integer_32_bit> const&  parents, std::vector<std::vector<integer_32_bit> >&  children);

/// Computes an angle for rotation along 'axis' so that 'axis x rotated(current)' and 'axis x target' will be linearly dependent.
/// 'axis' must be a unit vector.
float_32_bit  compute_rotation_angle(vector3 const&  unit_axis, vector3 const&  current, vector3 const&  target);


float_32_bit  compute_rotation_angle_of_projected_vector(
        vector3 const&  from,
        vector3 const&  to,
        float_32_bit const from_length,
        float_32_bit const to_length,
        vector3 const&  unit_axis
        );


void  compute_to_bone_space_matrix(
        integer_32_bit  bone_index,
        std::vector<coordinate_system> const&  frames,
        std::vector<integer_32_bit> const&  parents,
        matrix44&  W
        );

    
void  compute_from_child_to_parent_bone_space_matrix(
        integer_32_bit  child_bone_index,
        integer_32_bit const  parent_bone_index,
        std::vector<coordinate_system> const&  frames,
        std::vector<integer_32_bit> const&  parents,
        matrix44&  W
        );


}

#endif
