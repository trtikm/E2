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

namespace ai { namespace detail {


    


}}

namespace ai {


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
    for (natural_32_bit i = 0U; i != world_space_coord_systems.size(); ++i)
        if (parent_of_bones.at(i) == -1)
            local_coord_systems_of_bones.push_back(world_space_coord_systems.at(i));
        else
        {
            matrix44  to_parent_space_matrix;
            angeo::to_base_matrix(world_space_coord_systems.at(parent_of_bones.at(i)), to_parent_space_matrix);
            local_coord_systems_of_bones.push_back({
                    transform_point(world_space_coord_systems.at(i).origin(), to_parent_space_matrix),
                    transform(world_space_coord_systems.at(i).orientation(), to_parent_space_matrix)
                    });
        }

}


}
