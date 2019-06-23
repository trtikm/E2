#include <qtgl/keyframe.hpp>
#include <angeo/utility.hpp>
#include <utility/read_line.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/canonical_path.hpp>
#include <utility/msgstream.hpp>
#include <boost/filesystem.hpp>

namespace qtgl { namespace detail {


keyframe_data::keyframe_data(async::finalise_load_on_destroy_ptr const  finaliser)
    : m_time_point(0.0)
    , m_coord_systems()
{
    TMPROF_BLOCK();

    boost::filesystem::path const  pathname = finaliser->get_key().get_unique_id();

    if (!boost::filesystem::exists(pathname))
        throw std::runtime_error(msgstream() << "The passed file '" << pathname << "' does not exist.");

    if (boost::filesystem::file_size(pathname) < 4ULL)
        throw std::runtime_error(msgstream() << "The passed file '" << pathname << "' is not a qtgl file (wrong size).");

    std::ifstream  istr(pathname.string(),std::ios_base::binary);
    if (!istr.good())
        throw std::runtime_error(msgstream() << "Cannot open the keyframe file '" << pathname << "'.");

    {
        std::string  line;
        if (!read_line(istr,line))
            throw std::runtime_error(msgstream() << "Cannot read time point in the file '" << pathname << "'.");
        std::istringstream istr(line);
        istr >> m_time_point;
        if (m_time_point < 0.0f)
            throw std::runtime_error(msgstream() << "The time point in the file '" << pathname << "' is negative.");
    }

    angeo::read_all_coord_systems(istr, pathname, m_coord_systems);
}


keyframe_data::~keyframe_data()
{
    TMPROF_BLOCK();
}


}}

namespace qtgl { namespace detail {


keyframes_data::keyframes_data(
        async::finalise_load_on_destroy_ptr const  finaliser)
    : m_keyframes()
{
    TMPROF_BLOCK();

    boost::filesystem::path const  keyframes_dir = finaliser->get_key().get_unique_id();

    if (!boost::filesystem::is_directory(keyframes_dir))
        throw std::runtime_error("Cannot access the directory of keyframes: " + keyframes_dir.string());

    async::finalise_load_on_destroy_ptr const  keyframes_finaliser =
        async::finalise_load_on_destroy::create(
                [this, keyframes_dir](async::finalise_load_on_destroy_ptr) {
                    // All keyframes are loaded. So, let's check for their consistency and sort them by time.

                    if (m_keyframes.empty())
                        throw std::runtime_error("There is no keyframe file in the directory: " + keyframes_dir.string());

                    auto const  num_coord_systems_per_keyframe = m_keyframes.front().get_coord_systems().size();
                    for (std::size_t i = 0ULL; i != m_keyframes.size(); ++i)
                        if (m_keyframes.at(i).get_coord_systems().size() != num_coord_systems_per_keyframe)
                            throw std::runtime_error("Loaded of keyframes have different counts of coordinate "
                                                     "systems (inconsystent animation).");

                    std::sort(
                        m_keyframes.begin(),
                        m_keyframes.end(),
                        [](keyframe const& left, keyframe const& right) -> bool {
                            return left.get_time_point() < right.get_time_point();
                            }
                        );
                    },
                finaliser
                );

    for (boost::filesystem::directory_entry const& entry : boost::filesystem::directory_iterator(keyframes_dir))
    {
        std::string const  filename = entry.path().filename().string();
        std::string const  extension = entry.path().filename().extension().string();

        if (filename.find("keyframe") == 0UL && extension == ".txt")
        {
            m_keyframes.push_back(keyframe());
            m_keyframes.back().insert_load_request(canonical_path(entry.path()), keyframes_finaliser);
        }
    }
}


keyframes_data::~keyframes_data()
{
    TMPROF_BLOCK();
}


}}
