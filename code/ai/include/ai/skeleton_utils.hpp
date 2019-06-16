#ifndef AI_SKELETON_UTILS_HPP_INCLUDED
#   define AI_SKELETON_UTILS_HPP_INCLUDED

#   include <ai/skeleton_composition.hpp>
#   include <ai/skeletal_motion_templates.hpp>
#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <boost/filesystem/path.hpp>
#   include <vector>
#   include <string>

namespace ai {


skeleton_composition_ptr   load_skeleton_composition(boost::filesystem::path const&  skeleton_dir, std::string&  error_message);
skeletal_motion_templates_ptr  load_skeletal_motion_templates(boost::filesystem::path const&  skeleton_dir, std::string&  error_message);


/**
 * The 'skeleton_directory' is an directory containg files 'pose.txt', 'names.txt', and
 * 'parents.txt' representing a skeleton.
 *
 * All output arrays are of the same size; elements in the vectors on the same index
 * correspond to the same bone, i.e a bone is an index to the vectors; the vectors are
 * in the topological order, i.e. the hierarchy can be created by reading the vectors
 * sequentionaly without worrying that some parent does not exist yet.
 * The value -1 in 'parent_of_bones' vector indicates the corresponding bone has no parent.
 *
 * The function returns the empty string on succeess and error message otherwise.
 */
std::string  load_skeleton(
        boost::filesystem::path const&  skeleton_directory,
        std::vector<angeo::coordinate_system>&  local_coord_systems_of_bones,
        std::vector<std::string>&  names_of_bones,
        std::vector<integer_32_bit>&  parent_of_bones,
        // Either both pointers below are 'nullptr' on none of them.
        vector3* const  forward_direction_in_anim_space = nullptr,
        vector3* const  up_direction_in_anim_space = nullptr
        );

/**
 * The following four functions are used by the function 'load_skeleton' above
 * for loading individual vectors.
 */
std::string  load_skeleton_bone_local_coord_systems(
        boost::filesystem::path const&  skeleton_pose_file,
        std::vector<angeo::coordinate_system>&  local_coord_systems
        );
std::string  load_skeleton_bone_names(
        boost::filesystem::path const&  skeleton_names_file,
        std::vector<std::string>&  names_of_bones
        );
std::string  load_skeleton_bone_parents(
        boost::filesystem::path const&  skeleton_parents_file,
        std::vector<integer_32_bit>&  parent_of_bones
        );
std::string  load_skeleton_forward_and_up_directions(
        boost::filesystem::path const&  skeleton_forward_and_up_directions_file,
        vector3* const  forward_direction_in_anim_space,    // must be != nullptr
        vector3* const  up_direction_in_anim_space          // must be != nullptr
        );


void  transform_skeleton_coord_systems_from_world_to_local_space(
        std::vector<angeo::coordinate_system> const&  world_space_coord_systems,
        std::vector<integer_32_bit> const&  parent_of_bones,
        std::vector<angeo::coordinate_system>&  local_coord_systems_of_bones
        );


void  transform_skeleton_coord_systems_from_local_space_to_world(
        std::vector<angeo::coordinate_system> const&  bone_local_frames,
        std::vector<integer_32_bit> const&  bone_parents,
        angeo::coordinate_system const& reference_frame_in_world_space,
        std::vector<angeo::coordinate_system>&  output_bone_frames_in_world_space
        );


void  transform_skeleton_coord_systems_from_common_reference_frame_to_world(
        std::vector<angeo::coordinate_system> const&  bone_frames_in_reference_frame,
        angeo::coordinate_system const& reference_frame_in_world_space,
        std::vector<angeo::coordinate_system>&  output_bone_frames_in_world_space
        );


void  interpolate_keyframes_spherical(
        std::vector<angeo::coordinate_system> const&  src_frames,
        std::vector<angeo::coordinate_system> const&  dst_frames,
        float_32_bit const  interpolation_param, // in range <0,1>
        std::vector<angeo::coordinate_system>&  output
        );


}

#endif
