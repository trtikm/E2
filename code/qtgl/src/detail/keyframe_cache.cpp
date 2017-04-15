#include <qtgl/detail/keyframe_cache.hpp>
#include <qtgl/detail/read_line.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/msgstream.hpp>
#include <boost/filesystem.hpp>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>

namespace qtgl { namespace detail {


keyframe_data::keyframe_data(boost::filesystem::path const&  pathname)
    : m_time_point(0.0)
    , m_coord_systems()
{
    TMPROF_BLOCK();

    if (!boost::filesystem::exists(pathname))
        throw std::runtime_error(msgstream() << "The passed file '" << pathname << "' does not exist.");

    if (boost::filesystem::file_size(pathname) < 4ULL)
        throw std::runtime_error(msgstream() << "The passed file '" << pathname << "' is not a qtgl file (wrong size).");

    std::ifstream  istr(pathname.string(),std::ios_base::binary);
    if (!istr.good())
        throw std::runtime_error(msgstream() << "Cannot open the keyframe file '" << pathname << "'.");

    std::string  file_type;
    if (!detail::read_line(istr,file_type))
        throw std::runtime_error(msgstream() << "The passed file '" << pathname
                                         << "' is not a qtgl file (cannot read its type string).");

    if (file_type == "E2::qtgl/keyframe/text")
    {
        {
        std::string  line;
        if (!detail::read_line(istr,line))
            throw std::runtime_error(msgstream() << "Cannot read time point in the file '" << pathname << "'.");
        std::istringstream istr(line);
        istr >> m_time_point;
        if (m_time_point < 0.0f)
            throw std::runtime_error(msgstream() << "The time point in the file '" << pathname << "' is negative.");
        }

        natural_32_bit  num_coord_systems;
        {
            std::string  line;
            if (!detail::read_line(istr,line))
                throw std::runtime_error(msgstream() << "Cannot read time point in the file '" << pathname << "'.");
            std::istringstream istr(line);
            istr >> num_coord_systems;
            if (num_coord_systems == 0U)
                throw std::runtime_error(msgstream() << "The the file '" << pathname << "' does not contain any coodinate system.");
        }

        for (natural_32_bit  i = 0U; i != num_coord_systems; ++i)
        {
            vector3  position;
            {
                for (natural_32_bit  j = 0U; j != 3U; ++j)
                {
                    std::string  line;
                    if (!detail::read_line(istr,line))
                        throw std::runtime_error(msgstream() << "Cannot read coordinate #" << j
                                                             << " of position of coodinate system #" << i
                                                             << " in the file '" << pathname << "'.");
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
                    if (!detail::read_line(istr,line))
                        throw std::runtime_error(msgstream() << "Cannot read coordinate #" << j
                                                             << " of orientation of coodinate system #" << i
                                                             << " in the file '" << pathname << "'.");
                    std::istringstream istr(line);
                    istr >> coords[j];
                    sum += coords[j] * coords[j];
                }
                if (std::fabsf(1.0f - sum) > 1e-2f)
                    throw std::runtime_error(msgstream() << "The orientation of coodinate system #" << i
                                                         << " in the file '" << pathname << "' is not normalised.");
                orientation = quaternion(coords[0],coords[1],coords[2],coords[3]);
                orientation.normalize();
            }
            m_coord_systems.push_back({position,orientation});
        }
    }
}


}}
