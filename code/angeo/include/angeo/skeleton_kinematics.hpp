#ifndef ANGEO_SKELETON_KINEMATICS_HPP_INCLUDED
#   define ANGEO_SKELETON_KINEMATICS_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <vector>
#   include <unordered_map>
#   include <unordered_set>

namespace angeo {


/**
 * This module provides data and algorithms for forward and inverse kinematics
 * operating on a chain (see definition below) of bones of a skeleton.
 */


//////////////////////////////////////////////////////////////////////////////
// IMPORTANT ASSUMPOTIONS USED IN ALL DATA AND ALGORITHMS OF THIS MODULE:   //
//    (1) A BONE HAS AT MOST ONE PARENT BONE IN A SKELETON.                 //
//    (2) THERE IS EXACTLY ONE BONE WITHOUT A PARENT BONE IN A SKELETON.    //
//    (3) BONES DOES NOT FORM A LOOP IN A SKELETON.                         //
//////////////////////////////////////////////////////////////////////////////


// DEFINITION:
// An 'end-effector' bone is a bone which is NOT a parent bone of any bone in the skeleton.

// DEFINITION:
// A 'root' bone of a sub-set of bones of a skeleton is a bone which either does not have
// a parent bone at all in the skeleton or its parent bone does not belong to the sub-set.
// Note: A root bone must always be a member of that sub-set.

// DEFINITION:
// A 'chain' is a sub-set of bones of a skeleton containing exactly one end-effector bone,
// exactly one 'root' bone, and each bone in the chain, except the root bone, has a parent
// bone which also belongs to the chain.

// DEFINITION:
// The 'world space' of a chain of bones of a skeleton is the coordinate system, i.e. the 'frame',
// of the parent bone of the root bone of the chain. In the case, when the root bone does not have
// a parent bone, then the world space of the chain is the default frame, i.e. the frame where the
// origin is vector3_zero() and x, y, and z axis vectors are vector3_unit_x(), vector3_unit_y()
// and vector3_unit_z() respectively.


// This is a specification of a single rotational degree of freedom of a joint between two bones.
// A joint is always defined between a given bone and its direct parent bone in a skeleton.
struct  joint_rotation_props
{
    // Defines the rotation axis of the joint. Namely, it says how the given bone may rotate in the frame of the parent bone.
    // It must be a unit vector defined in the frame of the parent bone.
    vector3  axis;
    // Defines the zero (neutral) orientation of the given bone w.r.t. the frame of the parent bone.
    // It must be a unit vector, defined in the frame of the parent bone, and perpendicular to the 'axis' vector.
    vector3  zero_angle_direction;
    // Defines the current orientation of the given bone w.r.t. the frame of the parent bone.
    // It must be a unit vector, defined in the frame of the given bone, and perpendicular to the 'axis' vector
    // (when transformed to the frame of the parent bone).
    vector3  direction;
    // Defines allowed range of angles between 'zero_angle_direction' and 'direction' (when transformed
    // to the frame of the parent bone), which is <-'max_angle'/2, +'max_angle'/2>. The variable 'max_angle'
    // itself must be in the range <0, 2*PI). If it is 0.0f exactly, then the 'max_angle' limit is ignored
    // and the given bone may rotate along the 'axis' without any restriction.
    float_32_bit  max_angle;
    // Defines a fastest angular speed for the joint rotation along its axis.
    // It is expressed in rad/s; must be a positive number.
    float_32_bit  max_angular_speed;
};


// This is a specification of all rotational degrees of freedom of a joint between a given bone and its
// direct parent bone in the skeleton.
using  joint_rotation_props_vector = std::vector<joint_rotation_props>;


// This is a specification of all rotational degrees of freedom between bones in a skeleton.
using  joint_rotation_props_of_bones = std::unordered_map<
        // Index of a given bone.
        natural_32_bit,
        // All rotational degrees of freedom of a joint between the given bone and its direct
        // parent bone in the skeleton. 
        joint_rotation_props_vector
        >;


// This is a specification of the current state of a single rotational degree of freedom of a joint between two bones.
// An instance of this data type always correspond to a related instance of the 'joint_rotation_props'
struct joint_rotation_state
{
    // The current frame of the bone relative to the frame of the parent bone or to the world space, in case there is no parent bone.
    coordinate_system  frame;
    float_32_bit  current_angle;
    // FOR INTERNAL USE ONLY! This matrix is an auxiliary member. It may not be up-to-date or not initialised at all.
    // It is only computed when needed by some algorithm below. Therefore, it is highly recommended to *NOT* access this
    // member from outside of this module.
    mutable matrix44  from_bone_to_world_space_matrix;
};


// This is a specification of all current states of rotational degrees of freedom of a joint between a given bone and its
// direct parent bone in the skeleton.
using  joint_rotation_state_vector = std::vector<joint_rotation_state>;


void  skeleton_setup_joint_states_from_default_pose_frame(
        joint_rotation_state_vector&  joint_states,
        joint_rotation_props_vector const&  joint_defeintion,
        coordinate_system const&  pose_frame
        );


void  skeleton_commit_joint_states_to_frame(coordinate_system&  output_frame, joint_rotation_state_vector const&  joint_states);


// This is a specification of all current states of rotational degrees of freedom between bones in a skeleton.
using  joint_rotation_states_of_bones = std::unordered_map<
        // Index of a given bone.
        natural_32_bit,
        // All current states of rotational degrees of freedom of a joint between the given bone and its direct
        // parent bone in the skeleton. 
        joint_rotation_state_vector
        >;


void  skeleton_commit_joint_states_to_frames(
        std::unordered_map<natural_32_bit, coordinate_system*>&  output_frames,
        joint_rotation_states_of_bones const&  joint_states
        );


// For each rotational degree of freedom of a given joint there is one angle delta in the vector.
using  joint_angle_deltas_vector = std::vector<float_32_bit>;


// This a specification of the input of the forward kinematics algorithm as well as the output of the inverse
// kinematics algorithm. So, it specifies for each bone in a considered chain all angle deltas for the joint
// between the bone and its direct parent bone.
using  joint_angle_deltas_of_bones = std::unordered_map<
        // Index of a given bone.
        natural_32_bit,
        // All angle deltas for the joint between the given bone and its direct parent bone in the skeleton. 
        joint_angle_deltas_vector
        >;


// An operation on angle deltas of joints: adds to each angle delta in 'angle_deltas' the corresponding angle delta
// from 'added_angle_deltas'.
void  skeleton_add_joint_angle_deltas(joint_angle_deltas_vector&  angle_deltas, joint_angle_deltas_vector const&  added_angle_deltas);
void  skeleton_add_joint_angle_deltas(
        joint_angle_deltas_of_bones&  angle_deltas,
        joint_angle_deltas_of_bones const&  added_angle_deltas
        );


// An operation on angle deltas of joints: multiplies each angle delta in 'angle_deltas' by the passed 'scale'.
void  skeleton_scale_angle_deltas(joint_angle_deltas_vector&  angle_deltas, float_32_bit const  scale);
void  skeleton_scale_joint_angle_deltas(joint_angle_deltas_of_bones&  angle_deltas, float_32_bit const  scale);


void  skeleton_increment_bone_count(std::unordered_map<natural_32_bit, natural_32_bit>&  bone_counts, natural_32_bit const  bone);


template<typename  T>
void  skeleton_add_bone_counts(
        std::unordered_map<natural_32_bit, natural_32_bit>&  bone_counts,
        std::unordered_map<natural_32_bit, T> const&  bones)
{
    for (auto const&  bone_and_data : bones)
        skeleton_increment_bone_count(bone_counts, bone_and_data.first);
}


void  skeleton_average_joint_angle_deltas(
        joint_angle_deltas_of_bones&  averaged_angle_deltas,
        std::vector<joint_angle_deltas_of_bones> const&  angle_deltas,
        // In the case when elements in 'angle_deltas' vector are NOT ALL defined over the SAME set of bones,
        // then use the function 'skeleton_add_bone_counts' above to compute the counts
        // of bones used accross the angle deltas and pass its address here. Otherwise, pass 'nullptr'.
        std::unordered_map<natural_32_bit, natural_32_bit> const* const  bone_counts = nullptr
        );


// This is the forward kinematics algorithm for a single joint.
void  skeleton_apply_angle_deltas(
        joint_rotation_state_vector&  joint_states,
        joint_angle_deltas_vector const&  angle_deltas,
        joint_rotation_props_vector const&  joint_definitions
        );
// This is the forward kinematics algorithm for a given chain.
void  skeleton_apply_angle_deltas(
        joint_rotation_states_of_bones&  joint_states,
        joint_angle_deltas_of_bones const&  angle_deltas,
        joint_rotation_props_of_bones const&  joint_definitions
        );


struct  skeleton_kinematics_of_chain_of_bones
{
    // In order to understand meaning of paraneters it is recomended to read definitions at the top of this file.
    skeleton_kinematics_of_chain_of_bones(
            // Frames of bones of the chain arranged in the pose in which the passed 'rotation_props' were defined.
            // This is typically the T-pose of the skeleton. A frame of a bone must be represented in the frame of its parent
            // bone, if such exists.
            std::unordered_map<natural_32_bit, coordinate_system const*> const&  pose_frames,

            // Here you define the chain of bones by a set of all bones in the chain and the end-effector bone.
            // The root bone is infered automatically.
            std::unordered_set<natural_32_bit> const&  bones_of_the_chain,
            natural_32_bit const  end_effector_bone_,

            // Specification of all rotation props for each bone in 'bones_of_the_chain'. The passed map may contain more
            // entries, but they will be ignored.
            joint_rotation_props_of_bones const&  rotation_props, 

            // The parent relation for all bones of the skeleton. The set 'bones_of_the_chain' will be used to pick the needed
            // sub-set of this relation. Bones of the skeletion represent indices to the passed vector. Values in the vector
            // then represent parent bones. The value -1 at index 'i' means, the bone 'i' does not have a parent.
            std::vector<integer_32_bit> const&  parent_bones_,

            // Lengths of all bones in the skeleton.
            std::vector<float_32_bit> const&  bone_lengths_
            );

    // Apply forward kinematics algo, i.e. rotate joints by corresponding passed angle deltas.
    void  apply_angle_deltas(joint_angle_deltas_of_bones const&  angle_deltas)
    { skeleton_apply_angle_deltas(joint_states, angle_deltas, joint_definitions); }

    // Commits rotations in joint_current_states into the into the vector 'output_frames'.
    void  commit_target_frames(
            // Frames of bones of the chains. Updated will only be those frames which correspond to bones in the chain.
            std::unordered_map<natural_32_bit, coordinate_system*>&  output_frames
            ) const
    { skeleton_commit_joint_states_to_frames(output_frames, joint_states); }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    // NEXT FOLLOW IMPLEMENTATION DETAILS - rather do not access from outside of this module.
    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    void  update_world_matrices_of_all_joint_current_states() const;
    joint_rotation_state const* find_predecessor_joint_state(natural_32_bit const  bone, natural_32_bit const  state_index) const;
    matrix44 const& get_world_matrix_of_predecessor_joint_state(natural_32_bit const  bone, natural_32_bit const  state_index) const;

    natural_32_bit  end_effector_bone;
    natural_32_bit  root_bone;
    // Use for iteration over bones fron the 'end_effector_bone' to the 'root_bone'.
    std::unordered_map<natural_32_bit, natural_32_bit>  parent_bones;
    std::unordered_map<natural_32_bit, float_32_bit>  bone_lengths;
    // The inverse of 'parent_bones'. Use for iteration over bones from the 'root_bone' to the 'end_effector_bone'.
    std::unordered_map<natural_32_bit, natural_32_bit>  child_bones;    
    joint_rotation_props_of_bones  joint_definitions;
    joint_rotation_states_of_bones  joint_states;
};


// This is a specification of a constraint on an end-effector bone, which is supposed to be
// satisfied by the look at algorithm as much as possible (i.e. to minimise the error). That
// means the algorithm is supposed to rotate the end-effector bone (possibly together with other
// bones) so that the distance of the 'target' point from the line defined by the origin
// of the end-effector bone and the 'direction' vector, when both the origin and the 'direction'
// vector are tranformed to the space where the 'target' point is defined, is minimal.
struct  look_at_target
{
    // The look at target point defined in the world space (of a chain of bones considered in the look at).
    vector3  target;
    // The look at direction *unit* vector defined in the space of the end-effector bone.
    vector3  direction;
};


// This is a specification of a constraint on an end-effector bone, which is supposed to be
// satisfied by the aim at algorithm as much as possible (i.e. to minimise the error). That
// means the algorithm is supposed to rotate the end-effector bone (possibly together with other
// bones) so that the distance between the 'target' point and the 'source' point, when tranformed
// to the space where the 'target' point is defined, is minimal.
struct  aim_at_target
{
    // The aim at target point defined in the world space (of a chain of bones considered in the aim at).
    vector3  target;
    // The look at direction *unit* vector defined in the space of the end-effector bone.
    vector3  source;
};


// This is a specification of all aim at constraints on a given end-effector bone.
using  bone_aim_at_targets = std::vector<aim_at_target>;


// This is the look-at inverse kinematics algorithm for given chains.
void  skeleton_look_at(
        // Each instance is the vector must have a different end_effector bone. However, other bones can be shared.
        std::vector<skeleton_kinematics_of_chain_of_bones>&  kinematics,
        // For each instance in the vector 'kinematics' above there must be the corresponding instance at the same
        // index in this vector 'look_at_targets'.
        std::vector<look_at_target> const&  look_at_targets,
        // Higher the number, more precise look at the target. Must be > 0U.
        natural_32_bit const  max_iterations = 5U,
        // Higher the number, less extreme rotation angles are computed (i.e. rotations are more spread over all bones in the chain).
        // Must be > 0U.
        natural_32_bit const  max_ik_solver_iterations = 3U
        );


// This is the aim-at inverse kinematics algorithm for a given chain.
void  skeleton_aim_at(
        skeleton_kinematics_of_chain_of_bones&  kinematics,
        bone_aim_at_targets const&  aim_at_targets,
        // Higher the number, more precise aim at the target. Must be > 0U.
        natural_32_bit const  max_iterations = 5U,
        // Higher the number, less extreme rotation angles are computed (i.e. rotations are more spread over all bones in the chain).
        // Must be > 0U.
        natural_32_bit const  max_ik_solver_iterations = 3U
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

/// Computes the unit axis and the angle for rotating the vector 'current' in order to get a vector 'rotated_current'
/// satisfying 'target == t * rotated_current' for some 't >= 0'.
float_32_bit  compute_rotation_axis_and_angle(vector3&  output_unit_axis, vector3 const&  current, vector3 const&  target);

vector3  compute_common_look_at_target_for_multiple_eyes(
        std::vector<std::pair<vector3, vector3> > const&  vector_of_eye_pos_and_eye_unit_dir_pairs
        );


}

#endif
