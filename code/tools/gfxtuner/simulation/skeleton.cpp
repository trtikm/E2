#include <gfxtuner/simulation/skeleton.hpp>
#include <ai/skeleton_utils.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/canonical_path.hpp>
#include <boost/filesystem.hpp>
#include <exception>


namespace detail {


skeleton_data::skeleton_data(async::finalise_load_on_destroy_ptr const  finaliser)
    : m_relative_node_ids()
{
    boost::filesystem::path const  skeleton_directory = finaliser->get_key().get_unique_id();

    std::vector<std::string>  names_of_bones;
    std::string  error_message = ai::load_skeleton_bone_names(skeleton_directory / "names.txt", names_of_bones);
    if (!error_message.empty())
        throw std::runtime_error(error_message);

    std::vector<integer_32_bit>  parent_of_bones;
    error_message = ai::load_skeleton_bone_parents(skeleton_directory / "parents.txt", parent_of_bones);
    if (!error_message.empty())
        throw std::runtime_error(error_message);
    if (parent_of_bones.size() != names_of_bones.size())
        throw std::runtime_error("The number of bone parents in the file 'parents.txt' differs from number of bone names in the file 'names.txt'.");

    for (natural_32_bit i = 0U; i != names_of_bones.size(); ++i)
        m_relative_node_ids.push_back(
                parent_of_bones.at(i) == -1 ? scn::scene_node_id(names_of_bones.at(i))
                                            : m_relative_node_ids.at(parent_of_bones.at(i)) / names_of_bones.at(i)
                );
}


skeleton_data::~skeleton_data()
{
    TMPROF_BLOCK();
}


}

