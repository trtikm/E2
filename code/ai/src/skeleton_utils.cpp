#include <ai/skeleton_utils.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


void  transform_skeleton_coord_systems_from_world_to_local_space(
        std::vector<angeo::coordinate_system> const&  world_space_coord_systems,
        std::vector<integer_32_bit> const&  parent_of_bones,
        std::vector<angeo::coordinate_system>&  local_coord_systems_of_bones
        )
{
    TMPROF_BLOCK();

    local_coord_systems_of_bones.resize(world_space_coord_systems.size());

    for (natural_32_bit i = 0U; i != world_space_coord_systems.size(); ++i)
        if (parent_of_bones.at(i) == -1)
            local_coord_systems_of_bones.at(i) = world_space_coord_systems.at(i);
        else
        {
            matrix44  to_parent_space_matrix;
            angeo::to_base_matrix(world_space_coord_systems.at(parent_of_bones.at(i)), to_parent_space_matrix);
            local_coord_systems_of_bones.at(i) =
                {
                    transform_point(world_space_coord_systems.at(i).origin(), to_parent_space_matrix),
                    transform(world_space_coord_systems.at(i).orientation(), to_parent_space_matrix)
                };
        }
}


void  transform_skeleton_coord_systems_from_local_space_to_world(
        std::vector<angeo::coordinate_system> const&  bone_local_frames,
        std::vector<integer_32_bit> const&  bone_parents,
        angeo::coordinate_system const& reference_frame_in_world_space,
        std::vector<angeo::coordinate_system>&  output_bone_frames_in_world_space
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(bone_local_frames.size() == bone_parents.size());

    std::vector<matrix44>  world_matrices_of_bones;
    {
        world_matrices_of_bones.resize(bone_local_frames.size() + 1U);
        angeo::from_base_matrix(reference_frame_in_world_space, world_matrices_of_bones.at(0U));
    }

    output_bone_frames_in_world_space.resize(bone_local_frames.size());

    for (natural_32_bit  bone = 0U; bone != bone_local_frames.size(); ++bone)
    {
        vector3  u;
        matrix33  R;
        {
            matrix44  M;
            angeo::from_base_matrix(bone_local_frames.at(bone), M);
            world_matrices_of_bones.at(bone + 1U) = world_matrices_of_bones.at(bone_parents.at(bone) + 1) * M;
            decompose_matrix44(world_matrices_of_bones.at(bone + 1U), u, R);
        }
        output_bone_frames_in_world_space.at(bone) = { u, normalised(rotation_matrix_to_quaternion(R)) };
    }
}


void  transform_skeleton_coord_systems_from_common_reference_frame_to_world(
        std::vector<angeo::coordinate_system> const&  bone_frames_in_reference_frame,
        angeo::coordinate_system const& reference_frame_in_world_space,
        std::vector<angeo::coordinate_system>&  output_bone_frames_in_world_space
        )
{
    TMPROF_BLOCK();

    output_bone_frames_in_world_space.resize(bone_frames_in_reference_frame.size());

    matrix44  W;
    angeo::from_base_matrix(reference_frame_in_world_space, W);
    for (natural_32_bit  bone = 0U; bone != bone_frames_in_reference_frame.size(); ++bone)
    {
        vector3  u;
        matrix33  R;
        {
            matrix44  M;
            angeo::from_base_matrix(bone_frames_in_reference_frame.at(bone), M);
            decompose_matrix44(W * M, u, R);
        }
        output_bone_frames_in_world_space.at(bone) = { u, normalised(rotation_matrix_to_quaternion(R)) };
    }
}


void  interpolate_keyframes_spherical(
        std::vector<angeo::coordinate_system> const&  src_frames,
        std::vector<angeo::coordinate_system> const&  dst_frames,
        float_32_bit const  interpolation_param, // in range <0,1>
        std::vector<angeo::coordinate_system>&  output
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(src_frames.size() == dst_frames.size());
    output.resize(src_frames.size());
    for (std::size_t i = 0UL; i != src_frames.size(); ++i)
        angeo::interpolate_spherical(src_frames.at(i), dst_frames.at(i), interpolation_param, output.at(i));
}


}
