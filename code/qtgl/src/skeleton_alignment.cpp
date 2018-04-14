#include <qtgl/skeleton_alignment.hpp>
#include <utility/read_line.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/msgstream.hpp>
#include <boost/filesystem.hpp>
#include <fstream>

namespace qtgl { namespace detail {


skeleton_alignment_data::skeleton_alignment_data(async::key_type const&  key, async::finalise_load_on_destroy_ptr)
    : m_skeleton_alignment()
    , m_skeleton_alignment_path(key.second)
{
    TMPROF_BLOCK();

    if (!boost::filesystem::exists(get_skeleton_alignment_path()))
        throw std::runtime_error(msgstream() << "The file '" << get_skeleton_alignment_path() << "' does not exists.");
    if (!boost::filesystem::is_regular_file(get_skeleton_alignment_path()))
        throw std::runtime_error(msgstream() << "The path '" << get_skeleton_alignment_path()
                                             << "' does not reference a regular file.");

    std::ifstream  istr(get_skeleton_alignment_path().string(), std::ios_base::binary);
    if (!istr.good())
        throw std::runtime_error(msgstream() << "Cannot open the file '" << get_skeleton_alignment_path() << "'.");

    auto const  read_string = [&istr, this](std::string const&  kind_message) -> std::string {
        std::string  line = read_line(istr);
        if (line.empty())
            throw std::runtime_error(msgstream() << "Failed to read " << kind_message << " in the file: "
                                                 << get_skeleton_alignment_path());
        return line;
    };

    m_skeleton_alignment.set_origin({
        std::stof(read_string("origin.x")),
        std::stof(read_string("origin.y")),
        std::stof(read_string("origin.z"))
        });
    m_skeleton_alignment.set_orientation(make_quaternion_wxyz(
        std::stof(read_string("orientation.w")),
        std::stof(read_string("orientation.x")),
        std::stof(read_string("orientation.y")),
        std::stof(read_string("orientation.z"))
        ));
}


skeleton_alignment_data::~skeleton_alignment_data()
{
    TMPROF_BLOCK();
}


}}
