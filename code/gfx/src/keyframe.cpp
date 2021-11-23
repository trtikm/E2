#include <gfx/keyframe.hpp>
#include <angeo/utility.hpp>
#include <utility/read_line.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/canonical_path.hpp>
#include <utility/msgstream.hpp>
#include <filesystem>
#include <fstream>

namespace gfx { namespace detail {


keyframe_data::keyframe_data(async::finalise_load_on_destroy_ptr const  finaliser)
    : m_time_point(0.0)
    , m_coord_systems()
    , m_from_indices_to_bones(nullptr)
    , m_from_bones_to_indices(nullptr)
{
    TMPROF_BLOCK();

    std::filesystem::path const  pathname = finaliser->get_key().get_unique_id();

    if (!std::filesystem::exists(pathname))
        throw std::runtime_error(msgstream() << "The passed file '" << pathname << "' does not exist.");

    if (std::filesystem::file_size(pathname) < 4ULL)
        throw std::runtime_error(msgstream() << "The passed file '" << pathname << "' is not a gfx file (wrong size).");

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

namespace gfx { namespace detail {


keyframes_data::keyframes_data(
        async::finalise_load_on_destroy_ptr const  finaliser)
    : m_keyframes()
    , m_from_indices_to_bones(nullptr)
    , m_from_bones_to_indices(nullptr)
{
    TMPROF_BLOCK();

    std::filesystem::path const  keyframes_dir = finaliser->get_key().get_unique_id();

    if (!std::filesystem::is_directory(keyframes_dir))
        throw std::runtime_error("Cannot access the directory of keyframes: " + keyframes_dir.string());

    async::finalise_load_on_destroy_ptr const  keyframes_finaliser =
        async::finalise_load_on_destroy::create(
                [this, keyframes_dir](async::finalise_load_on_destroy_ptr) {
                    // All keyframes are loaded. So, let's check for their consistency and sort them by time.

                    if (m_keyframes.empty())
                        throw std::runtime_error("There is no keyframe file in the directory: " + keyframes_dir.string());

                    auto const  num_coord_systems_per_keyframe = m_keyframes.front().get_coord_systems().size();

                    //INVARIANT(m_from_indices_to_bones != nullptr && m_from_bones_to_indices != nullptr);
                    if (m_from_indices_to_bones == nullptr)
                    {
                        auto  bones_ptr = std::make_shared<translation_map>();
                        for (natural_32_bit i = 0U; i != (natural_32_bit)num_coord_systems_per_keyframe; ++i)
                            bones_ptr->insert({ i, i });
                        m_from_indices_to_bones = bones_ptr;
                        m_from_bones_to_indices = bones_ptr;
                    }

                    for (std::size_t i = 0ULL; i != m_keyframes.size(); ++i)
                    {
                        if (m_keyframes.at(i).get_coord_systems().size() != num_coord_systems_per_keyframe)
                            throw std::runtime_error("Loaded of keyframes have different counts of coordinate "
                                                     "systems (inconsystent animation).");
                        m_keyframes.at(i).resource().m_from_indices_to_bones = m_from_indices_to_bones;
                        m_keyframes.at(i).resource().m_from_bones_to_indices = m_from_bones_to_indices;
                    }

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

    for (std::filesystem::directory_entry const& entry : std::filesystem::directory_iterator(keyframes_dir))
    {
        std::string const  filename = entry.path().filename().string();
        std::string const  extension = entry.path().filename().extension().string();

        if (filename.find("keyframe") == 0UL && extension == ".txt")
        {
            m_keyframes.push_back(keyframe());
            m_keyframes.back().insert_load_request(canonical_path(entry.path()), keyframes_finaliser);
        }
        else if (filename == "bones.txt")
        {
            auto  to_bones_ptr = std::make_shared<translation_map>();
            auto  from_bones_ptr = std::make_shared<translation_map>();

            std::filesystem::path const  parents_pathname = canonical_path(entry.path());

            std::ifstream  istr;
            angeo::open_file_stream_for_reading(istr, parents_pathname);
            for (natural_32_bit i = 0U, n = angeo::read_num_records(istr, parents_pathname); i != n; ++i)
            {
                std::string  line;
                if (!read_line(istr, line))
                    throw std::runtime_error(msgstream() << "Cannot read " << i << "-th bone in the file '" << parents_pathname << "'.");
                natural_32_bit  bone;
                {
                    std::istringstream sstr(line);
                    sstr >> bone;
                }
                if (from_bones_ptr->count(bone) != 0UL)
                    throw std::runtime_error(msgstream() << "Bone " << bone << " appears more than once in the file '" << parents_pathname << "'.");
                to_bones_ptr->insert({ i, bone });
                from_bones_ptr->insert({ bone, i });
            }

            m_from_indices_to_bones = to_bones_ptr;
            m_from_bones_to_indices = from_bones_ptr;
        }
    }
}


keyframes_data::~keyframes_data()
{
    TMPROF_BLOCK();
}


}}
