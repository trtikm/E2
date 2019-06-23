#include <angeo/utility.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/read_line.hpp>
#include <utility/canonical_path.hpp>
#include <utility/msgstream.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <locale>

namespace angeo {


void  get_random_vector_of_magnitude(
        float_32_bit const  magnitude,
        random_generator_for_natural_32_bit&   random_generator,
        vector3&  resulting_vector
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(magnitude >= 0.0f);

    float_32_bit const  phi = get_random_float_32_bit_in_range(0.0f,2.0f * PI(),random_generator);
    float_32_bit const  sin_phi = std::sinf(phi);
    float_32_bit const  cos_phi = std::cosf(phi);

    float_32_bit const  psi = get_random_float_32_bit_in_range(-PI(),PI(),random_generator);
    float_32_bit const  sin_psi = std::sinf(psi);
    float_32_bit const  cos_psi = std::cosf(psi);

    resulting_vector = magnitude * vector3(cos_psi * cos_phi, cos_psi * sin_phi, sin_psi);
}


void  compute_tangent_space_of_unit_vector(
        vector3 const&  input_unit_vector,
        vector3&  output_unit_tangent,
        vector3&  output_unit_bitangent
        )
{
    output_unit_tangent = normalised(orthogonal(input_unit_vector));
    output_unit_bitangent = cross_product(input_unit_vector, output_unit_tangent);
}


natural_32_bit  read_scalar(
        float_32_bit&  output,
        std::ifstream&  istr,
        natural_32_bit const  line_number,
        boost::filesystem::path const&  pathname
        )
{
    std::string  line;
    if (!read_line(istr,line))
        throw std::runtime_error(msgstream() <<
                "Cannot read acalar at line " << line_number  << " in the file '" << pathname << "'.");
    std::istringstream sstr(line);
    sstr >> output;
    return line_number + 1U;
}


natural_32_bit  read_vector3(
        vector3&  output,
        std::ifstream&  istr,
        natural_32_bit const  line_number,
        boost::filesystem::path const&  pathname
        )
{
    for (natural_32_bit  j = 0U; j != 3U; ++j)
    {
        std::string  line;
        if (!read_line(istr,line))
            throw std::runtime_error(msgstream() <<
                    "Cannot read vector coordinate #" << j << " at line " << line_number + j  << " in the file '" << pathname << "'.");
        std::istringstream sstr(line);
        sstr >> output(j);
    }
    return line_number + 3U;
}


natural_32_bit  read_unit_quaternion(
        quaternion&  output,
        std::ifstream&  istr,
        natural_32_bit const  line_number,
        boost::filesystem::path const&  pathname
        )
{
    float_32_bit  coords[4];
    for (natural_32_bit  j = 0U; j != 4U; ++j)
    {
        std::string  line;
        if (!read_line(istr,line))
            throw std::runtime_error(msgstream() <<
                    "Cannot read quaternion coordinate #" << j << " at line " << line_number + j  << " in the file '" << pathname << "'.");
        std::istringstream sstr(line);
        sstr >> coords[j];
    }
    output = quaternion(coords[0],coords[1],coords[2],coords[3]).normalized();
    return line_number + 4U;
}


natural_32_bit  read_matrix33(
        matrix33&  output,
        std::ifstream&  istr,
        natural_32_bit const  line_number,
        boost::filesystem::path const&  pathname
        )
{
    for (natural_32_bit i = 0U; i != 3U; ++i)
        for (natural_32_bit  j = 0U; j != 3U; ++j)
        {
            std::string  line;
            if (!read_line(istr,line))
                throw std::runtime_error(msgstream() <<
                        "Cannot read matrix coordinate (" << i << "," << j << ") at line " << line_number + j  << " in the file '" << pathname << "'.");
            std::istringstream sstr(line);
            sstr >> output(i,j);
        }
    return line_number + 3U * 3U;
}


void  read_coord_systems(
        std::ifstream&  istr,
        boost::filesystem::path const&  pathname,
        natural_32_bit const  num_coord_systems_to_read,
        std::vector<coordinate_system>&  coord_systems
        )
{
    TMPROF_BLOCK();

    for (natural_32_bit  i = 0U, line_number = 0U; i != num_coord_systems_to_read; ++i)
    {
        vector3  position;
        line_number = read_vector3(position, istr, line_number, pathname);

        quaternion  orientation;
        line_number = read_unit_quaternion(orientation, istr, line_number, pathname);

        coord_systems.push_back({position,orientation});
    }
}


natural_32_bit  read_all_coord_systems(
        std::ifstream&  istr,
        boost::filesystem::path const&  pathname,
        std::vector<coordinate_system>&  coord_systems
        )
{
    TMPROF_BLOCK();

    natural_32_bit  num_coord_systems;
    {
        std::string  line;
        if (!read_line(istr,line))
            throw std::runtime_error(msgstream() << "Cannot read number of coord. systems in the file '" << pathname << "'.");
        std::istringstream sstr(line);
        sstr >> num_coord_systems;
        if (num_coord_systems == 0U)
            throw std::runtime_error(msgstream() << "The the file '" << pathname << "' does not contain any coodinate system.");
    }
    read_coord_systems(istr, pathname, num_coord_systems, coord_systems);
    return num_coord_systems;
}


}
