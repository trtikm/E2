#include <qtgl/modelspace.hpp>
#include <utility/read_line.hpp>
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


modelspace_data::modelspace_data(async::finalise_load_on_destroy_ptr const  finaliser)
    : m_coord_systems()
{
    TMPROF_BLOCK();

    boost::filesystem::path const  pathname = finaliser->get_key().get_unique_id();

    if (!boost::filesystem::exists(pathname))
        throw std::runtime_error(msgstream() << "The passed file '" << pathname << "' does not exist.");

    if (boost::filesystem::file_size(pathname) < 4ULL)
        throw std::runtime_error(msgstream() << "The passed file '" << pathname << "' is not a qtgl file (wrong size).");

    std::ifstream  istr(pathname.string(),std::ios_base::binary);
    if (!istr.good())
        throw std::runtime_error(msgstream() << "Cannot open the modelspace file '" << pathname << "'.");

    natural_32_bit  num_coord_systems;
    {
        std::string  line;
        if (!read_line(istr,line))
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
                if (!read_line(istr,line))
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
                if (!read_line(istr,line))
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


modelspace_data::~modelspace_data()
{
    TMPROF_BLOCK();
}


}}
