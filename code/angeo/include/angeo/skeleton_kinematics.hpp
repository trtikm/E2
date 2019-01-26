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
};


using  bone_look_at_targets =
            std::unordered_map< integer_32_bit,     // bone index
                                std::pair<vector3,  // "eye", i.e. a unit vector in bone's coord. system space, to look at "target".
                                          vector3   // "target", i.e. a vector in world space, the "eye" should look at.
            > >;


void  skeleton_bones_move_towards_targets(
        std::vector<coordinate_system>&  frames,        // coordinate systems, i.e. reference frames, of bones
        std::vector<integer_32_bit> const&  parents,    // value -1 at index 'i' means, the bone 'i' does not have a parent
        std::vector<std::vector<joint_rotation_props> > const&  rotation_props,
        bone_look_at_targets const&  look_at_targets,
        float_32_bit const  dt                          // simulation time step.
        );


/// Computes an angle for rotation along 'axis' so that 'axis x rotated(current)' and 'axis x target' will be linearly dependent.
/// 'axis' must be a unit vector.
float_32_bit  compute_rotation_angle(vector3 const&  axis, vector3 const&  current, vector3 const&  target);


}

#endif
