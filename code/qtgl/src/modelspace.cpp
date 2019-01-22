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
    std::string const  error_message = load_modelspace_coordinate_systems(finaliser->get_key().get_unique_id(), m_coord_systems);
    if (!error_message.empty())
        throw std::runtime_error(error_message);
}


modelspace_data::~modelspace_data()
{
    TMPROF_BLOCK();
}


}}

namespace qtgl {


std::string  load_modelspace_coordinate_systems(
        boost::filesystem::path const&  pose_file_pathname,
        std::vector<angeo::coordinate_system>&  output_coord_systems
        )
{
    TMPROF_BLOCK();

    if (!boost::filesystem::is_regular_file(pose_file_pathname))
        return msgstream() << "Cannot access file '" << pose_file_pathname << "'.";
    if (boost::filesystem::file_size(pose_file_pathname) < 4ULL)
        return msgstream() << "The modelspace file '" << pose_file_pathname << "' is not a qtgl file (wrong size).";

    std::ifstream  istr(pose_file_pathname.string(),std::ios_base::binary);
    ASSUMPTION(istr.good());

    natural_32_bit  num_coord_systems;
    {
        std::string  line;
        if (!read_line(istr,line))
            return msgstream() << "Cannot read the number of coord systems in the file '" << pose_file_pathname << "' file.";
        std::istringstream istr(line);
        istr >> num_coord_systems;
        if (num_coord_systems == 0U)
            return msgstream() << "The file '" << pose_file_pathname << "' does not contain any coodinate system.";
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
                                        << " in the file '" << pose_file_pathname << "'.";
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
                                        << " in the file '" << pose_file_pathname << "'.";
                std::istringstream istr(line);
                istr >> coords[j];
                sum += coords[j] * coords[j];
            }
            if (std::fabsf(1.0f - sum) > 1e-2f)
                msgstream() << "The orientation of coodinate system #" << i << " in the file '" << pose_file_pathname << "' is not normalised.";
            orientation = quaternion(coords[0],coords[1],coords[2],coords[3]);
            orientation.normalize();
        }
        output_coord_systems.push_back({position,orientation});
    }
    return "";
}


}