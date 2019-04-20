#include <ai/skeleton_utils.hpp>
#include <utility/read_line.hpp>
#include <utility/msgstream.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <unordered_set>
#include <memory>

namespace ai { namespace detail {


    


}}

namespace ai {


skeleton_composition_ptr   load_skeleton_composition(boost::filesystem::path const&  skeleton_dir, std::string&  error_message)
{
    TMPROF_BLOCK();

    skeleton_composition_ptr const  composition = std::make_shared<skeleton_composition>();
    {
        error_message =
                load_skeleton(
                        skeleton_dir,
                        composition->pose_frames,
                        composition->names,
                        composition->parents
                        );
        if (!error_message.empty())
            return nullptr;
        angeo::skeleton_compute_child_bones(composition->parents, composition->children);
    }
    return composition;
}


skeletal_motion_templates_ptr  load_skeletal_motion_templates(boost::filesystem::path const&  skeleton_dir, std::string&  error_message)
{
    TMPROF_BLOCK();

    skeletal_motion_templates_ptr const  motion_templates = std::make_shared<skeletal_motion_templates>();
    {
        for (boost::filesystem::directory_entry& entry : boost::filesystem::directory_iterator(skeleton_dir))
            if (boost::filesystem::is_directory(entry.path()))
                motion_templates->motions_map.insert({
                        entry.path().filename().string(),
                        skeletal_motion_templates::keyframes(entry.path())
                        });
    }
    return motion_templates;
}


std::string  load_skeleton(
        boost::filesystem::path const&  skeleton_directory,
        std::vector<angeo::coordinate_system>&  local_coord_systems_of_bones,
        std::vector<std::string>&  names_of_bones,
        std::vector<integer_32_bit>&  parent_of_bones
        )
{
    TMPROF_BLOCK();

    if (!boost::filesystem::is_directory(skeleton_directory))
        return "Cannot access the passed skeleton directory.";

    std::vector<angeo::coordinate_system>  world_space_coord_systems;
    std::string  error_message = load_skeleton_bone_world_coord_systems(skeleton_directory / "pose.txt", world_space_coord_systems);
    if (!error_message.empty())
        return error_message;

    error_message = load_skeleton_bone_names(skeleton_directory / "names.txt", names_of_bones);
    if (!error_message.empty())
        return error_message;
    if (names_of_bones.size() != world_space_coord_systems.size())
        return "The number of name in the file 'names.txt' differs from number of coodinate systems in the file 'pose.txt'.";

    error_message = load_skeleton_bone_parents(skeleton_directory / "parents.txt", parent_of_bones);
    if (!error_message.empty())
        return error_message;
    if (parent_of_bones.size() != world_space_coord_systems.size())
        return "The number of name in the file 'parents.txt' differs from number of coodinate systems in the file 'pose.txt'.";

    transform_skeleton_coord_systems_from_world_to_local_space(world_space_coord_systems, parent_of_bones, local_coord_systems_of_bones);

    return "";
}


std::string  load_skeleton_bone_world_coord_systems(
        boost::filesystem::path const&  skeleton_pose_file,
        std::vector<angeo::coordinate_system>&  world_space_coord_systems
        )
{
    TMPROF_BLOCK();

    if (!boost::filesystem::is_regular_file(skeleton_pose_file))
        return msgstream() << "Cannot access file '" << skeleton_pose_file << "'.";
    if (boost::filesystem::file_size(skeleton_pose_file) < 4ULL)
        return msgstream() << "The file '" << skeleton_pose_file << "' has wrong size.";

    std::ifstream  istr(skeleton_pose_file.string(), std::ios_base::binary);
    ASSUMPTION(istr.good());

    natural_32_bit  num_coord_systems;
    {
        std::string  line;
        if (!read_line(istr,line))
            return msgstream() << "Cannot read the number of coord systems in the file '" << skeleton_pose_file << "' file.";
        std::istringstream istr(line);
        istr >> num_coord_systems;
        if (num_coord_systems == 0U)
            return msgstream() << "The file '" << skeleton_pose_file << "' does not contain any coodinate system.";
    }

    for (natural_32_bit  i = 0U; i != num_coord_systems; ++i)
    {
        vector3  position;
        {
            for (natural_32_bit  j = 0U; j != 3U; ++j)
            {
                std::string  line;
                if (!read_line(istr,line))
                    return msgstream() << "Cannot read coordinate #" << j
                                        << " of position of coodinate system #" << i
                                        << " in the file '" << skeleton_pose_file << "'.";
                std::istringstream istr(line);
                istr >> position(j);
            }
        }
        quaternion  orientation;
        {
            float_32_bit  coords[4];
            float_32_bit  sum = 0.0f;
            for (natural_32_bit  j = 0U; j != 4U; ++j)
            {
                std::string  line;
                if (!read_line(istr,line))
                    return msgstream() << "Cannot read coordinate #" << j
                                        << " of orientation of coodinate system #" << i
                                        << " in the file '" << skeleton_pose_file << "'.";
                std::istringstream istr(line);
                istr >> coords[j];
                sum += coords[j] * coords[j];
            }
            if (std::fabsf(1.0f - sum) > 1e-2f)
                msgstream() << "The orientation of coodinate system #" << i << " in the file '" << skeleton_pose_file << "' is not normalised.";
            orientation = quaternion(coords[0],coords[1],coords[2],coords[3]);
            orientation.normalize();
        }
        world_space_coord_systems.push_back({position,orientation});
    }

    return "";
}


std::string  load_skeleton_bone_names(
        boost::filesystem::path const&  skeleton_names_file,
        std::vector<std::string>&  names_of_bones
        )
{
    TMPROF_BLOCK();

    if (!boost::filesystem::is_regular_file(skeleton_names_file))
        return msgstream() << "Cannot access file '" << skeleton_names_file << "'.";
    if (boost::filesystem::file_size(skeleton_names_file) < 4ULL)
        return msgstream() << "The file '" << skeleton_names_file << "' has wrong size.";

    std::ifstream  istr(skeleton_names_file.string(), std::ios_base::binary);
    ASSUMPTION(istr.good());

    natural_32_bit  num_names;
    {
        std::string  line;
        if (!read_line(istr, line))
            return msgstream() << "Cannot read the number of names in the file '" << skeleton_names_file << "' file.";
        std::istringstream istr(line);
        istr >> num_names;
    }

    std::unordered_set<std::string>  visited;
    for (natural_32_bit i = 0U; i != num_names; ++i)
    {
        std::string  line;
        if (!read_line(istr, line))
            return msgstream() << "Cannot read name #" << i << " in the file '" << skeleton_names_file << "'.";
        boost::algorithm::trim(line);

        if (visited.count(line) != 0UL)
            return msgstream() << "Duplicate bone name '" << line << "' at line #" << i << " in the file '" << skeleton_names_file << "'.";
        visited.insert(line);

        names_of_bones.push_back(line);
    }

    return "";
}


std::string  load_skeleton_bone_parents(
        boost::filesystem::path const&  skeleton_parents_file,
        std::vector<integer_32_bit>&  parent_of_bones
        )
{
    TMPROF_BLOCK();

    if (!boost::filesystem::is_regular_file(skeleton_parents_file))
        return msgstream() << "Cannot access file '" << skeleton_parents_file << "'.";
    if (boost::filesystem::file_size(skeleton_parents_file) < 4ULL)
        return msgstream() << "The file '" << skeleton_parents_file << "' has wrong size.";

    std::ifstream  istr(skeleton_parents_file.string(), std::ios_base::binary);
    ASSUMPTION(istr.good());

    natural_32_bit  num_names;
    {
        std::string  line;
        if (!read_line(istr, line))
            return msgstream() << "Cannot read the number of parent indices in the file '" << skeleton_parents_file << "' file.";
        std::istringstream istr(line);
        istr >> num_names;
    }

    for (natural_32_bit i = 0U; i != num_names; ++i)
    {
        std::string  line;
        if (!read_line(istr, line))
            return msgstream() << "Cannot read name #" << i << " in the file '" << skeleton_parents_file << "'.";
        integer_32_bit  parent_index;
        {
            std::istringstream istr(line);
            istr >> parent_index;
        }
        if (parent_index < -1)
            return msgstream() << "Wrong parent bone index " << parent_index << " for bone #" << i << " in the file '"
                               << skeleton_parents_file << "'. E.i. the bones were not saved in the topological order.";
        if (parent_index > (integer_32_bit)i)
            return msgstream() << "Bone #" << i << " comes before its parent bone #" << parent_index << " in the file '"
                               << skeleton_parents_file << "'. E.i. the bones were not saved in the topological order.";
        parent_of_bones.push_back(parent_index);
    }

    return "";
}


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


void  interpolate_keyframes(
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


void  infer_parent_frame_from_local_and_world_frames(
        angeo::coordinate_system const&  local_frame,
        matrix44 const&  world_frame,
        angeo::coordinate_system&  output_parent_frame
        )
{
    TMPROF_BLOCK();

    vector3  u;
    matrix33  R;
    {
        matrix44  N;
        angeo::to_base_matrix(local_frame, N);
        decompose_matrix44(world_frame * N, u, R);
    }
    output_parent_frame.set_origin(u);
    output_parent_frame.set_orientation(normalised(rotation_matrix_to_quaternion(R)));
}


void  infer_parent_frame_from_local_and_world_frames(
        angeo::coordinate_system const&  local_frame,
        angeo::coordinate_system const&  world_frame,
        angeo::coordinate_system&  output_parent_frame
        )
{
    TMPROF_BLOCK();

    matrix44  M;
    angeo::from_base_matrix(world_frame, M);
    infer_parent_frame_from_local_and_world_frames(local_frame, M, output_parent_frame);
}


}
