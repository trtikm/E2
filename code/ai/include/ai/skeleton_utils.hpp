#ifndef AI_SKELETON_UTILS_HPP_INCLUDED
#   define AI_SKELETON_UTILS_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <boost/filesystem/path.hpp>
#   include <vector>
#   include <string>

namespace ai {


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
        std::vector<integer_32_bit>&  parent_of_bones
        );


}

#endif
