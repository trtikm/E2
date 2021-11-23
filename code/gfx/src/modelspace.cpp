#include <gfx/modelspace.hpp>
#include <angeo/utility.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/msgstream.hpp>
#include <filesystem>
#include <fstream>

namespace gfx { namespace detail {


modelspace_data::modelspace_data(async::finalise_load_on_destroy_ptr const  finaliser)
    : m_coord_systems()
{
    TMPROF_BLOCK();

    std::filesystem::path const  pathname = finaliser->get_key().get_unique_id();

    if (!std::filesystem::is_regular_file(pathname))
        throw std::runtime_error(msgstream() << "Cannot access file '" << pathname << "'.");
    if (std::filesystem::file_size(pathname) < 4ULL)
        throw std::runtime_error(msgstream() << "The modelspace file '" << pathname << "' is invalid (wrong size).");

    std::ifstream  istr(pathname.string(), std::ios_base::binary);
    if (!istr.good())
        throw std::runtime_error(msgstream() << "Cannot open the modelspace file '" << pathname << "'.");

    angeo::read_all_coord_systems(istr, pathname, m_coord_systems);
}


modelspace_data::~modelspace_data()
{
    TMPROF_BLOCK();
}


}}
