#ifndef AI_SKELETON_UTILS_HPP_INCLUDED
#   define AI_SKELETON_UTILS_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <vector>

namespace ai {


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
