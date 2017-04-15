#ifndef ANGEO_COORDINATE_SYSTEM_HPP_INCLUDED
#   define ANGEO_COORDINATE_SYSTEM_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <memory>

namespace angeo {


struct coordinate_system;
using  coordinate_system_ptr = std::shared_ptr<coordinate_system>;
using  coordinate_system_const_ptr = std::shared_ptr<coordinate_system const>;


struct coordinate_system
{
    static coordinate_system_ptr  create(vector3 const&  origin, quaternion const&  orientation);

    coordinate_system() : coordinate_system(vector3_zero(),quaternion_identity()) {}
    coordinate_system(vector3 const&  origin, quaternion const&  orientation);

    vector3 const&  origin() const { return m_origin; }
    quaternion const&  orientation() const { return m_orientation; }

    void set_origin(vector3 const&  new_origin) { m_origin = new_origin; }
    void set_orientation(quaternion const&  new_normalised_orientation);

private:
    vector3  m_origin;
    quaternion  m_orientation;
};


void  translate(coordinate_system&  coord_system, vector3 const&  shift);
void  rotate(coordinate_system&  coord_system, quaternion const&  rotation);

void  from_base_matrix(coordinate_system const&  coord_system, matrix44&  output);
void  to_base_matrix(coordinate_system const&  coord_system, matrix44&  output);

vector3  axis(coordinate_system const&  coord_system, natural_8_bit const  axis_index);
vector3  axis_x(coordinate_system const&  coord_system);
vector3  axis_y(coordinate_system const&  coord_system);
vector3  axis_z(coordinate_system const&  coord_system);

void  interpolate_linear(
        coordinate_system const&  head,
        coordinate_system const&  tail,
        float_32_bit const  parameter, // it must be in range <0,1>
        coordinate_system&  result
        );
void  interpolate_spherical(
        coordinate_system const&  head,
        coordinate_system const&  tail,
        float_32_bit const  parameter, // it must be in range <0,1>
        coordinate_system&  result
        );


}

#endif
