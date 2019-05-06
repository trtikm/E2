#ifndef ANGEO_SKELETON_KINEMATICS_HPP_INCLUDED
#   define ANGEO_SKELETON_KINEMATICS_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <vector>
#   include <unordered_map>

namespace angeo {


struct  joint_rotation_props
{
    vector3  m_axis;                    // unit vector
    bool  m_axis_in_parent_space;       // true if 'm_axis' is expressed in bone's parent cood system.
    vector3  m_zero_angle_direction;    // unit vector; in bone's parent cood system.
    vector3  m_direction;               // unit vector; in bone's cood system.
    float_32_bit  m_max_angle;          // In range <0, 2*PI>; Defines an interval <-m_max_angle/2, +m_max_angle/2> of allowed
                                        // rotation angles along m_axis from m_zero_angle_direction to m_direction (projected to
                                        // the rotation plane).
    float_32_bit  m_stiffness_with_parent_bone;    // In range <0,1>; How a rotation of a bone affect a rotation of the parent bone.
    float_32_bit  m_max_angular_speed;  // in rad/s; must be a positive number.
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
        std::vector<std::vector<joint_rotation_props> > const&  rotation_props, // specification of rotation props of each bone at joint to its parent bone.
        std::unordered_map<integer_32_bit, std::vector<natural_32_bit> >* const  involved_rotations_of_bones = nullptr,
        natural_32_bit const  max_iterations = 25U,
        float_32_bit const  angle_range_epsilon = 0.1f * (PI() / 180.0f)
        );


void  skeleton_rotate_bones_towards_target_pose(
        std::vector<coordinate_system>&  frames,    // coordinate systems of bones in the current pose which (some of them) will be moved towards the target pose frames 'target_pose_frames'.
        std::vector<coordinate_system> const&  target_pose_frames,  // coordinate systems of bones in the target pose.
        std::vector<std::vector<joint_rotation_props> > const&  rotation_props, // specification of rotation props of each bone at joint to its parent bone.
        std::unordered_map<integer_32_bit, std::vector<natural_32_bit> > const&  bones_to_rotate,
        float_32_bit const  dt
        );



/// Given a parent for a bone (-1 when no parent), the function computes children the bone.
void  skeleton_compute_child_bones(std::vector<integer_32_bit> const&  parents, std::vector<std::vector<integer_32_bit> >&  children);

/// Computes an angle for rotation 'current' vector along 'axis' so that distance of the rotated 'current' to 'target' will be minimal.
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
