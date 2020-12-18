#ifndef ANGEO_COORDINATE_SYSTEM_HPP_INCLUDED
#   define ANGEO_COORDINATE_SYSTEM_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <array>
#   include <memory>

namespace angeo {


struct coordinate_system;
using  coordinate_system_ptr = std::shared_ptr<coordinate_system>;
using  coordinate_system_const_ptr = std::shared_ptr<coordinate_system const>;


struct  coordinate_system_explicit
{
    coordinate_system_explicit()
        : coordinate_system_explicit(
                vector3_zero(),
                vector3_unit_x(),
                vector3_unit_y(),
                vector3_unit_z()
                )
    {}

    coordinate_system_explicit(
            vector3 const&  origin_,
            vector3 const&  basis_x_unit_vector_,
            vector3 const&  basis_y_unit_vector_,
            vector3 const&  basis_z_unit_vector_
            )
        : m_origin{ origin_ }
        , m_basis_vectors{ basis_x_unit_vector_, basis_y_unit_vector_, basis_z_unit_vector_ }
    {}

    coordinate_system_explicit(vector3 const&  origin_, matrix33 const&  rotation_matrix_);
    coordinate_system_explicit(coordinate_system const&  coord_system_);

    vector3 const&  origin() const { return m_origin; }
    vector3 const&  basis_vector_x() const { return m_basis_vectors.at(0); }
    vector3 const&  basis_vector_y() const { return m_basis_vectors.at(1); }
    vector3 const&  basis_vector_z() const { return m_basis_vectors.at(2); }
    vector3 const&  basis_vector(natural_32_bit const  index) const { return m_basis_vectors.at(index); }

    void  set_origin(vector3 const&  u) { m_origin = u; }
    void  set_basis_vector_x(vector3 const&  u) { m_basis_vectors.at(0) = u; }
    void  set_basis_vector_y(vector3 const&  u) { m_basis_vectors.at(1) = u; }
    void  set_basis_vector_z(vector3 const&  u) { m_basis_vectors.at(2) = u; }

    vector3&  origin_ref() { return m_origin; }
    vector3&  basis_vector_x_ref() { return m_basis_vectors.at(0); }
    vector3&  basis_vector_y_ref() { return m_basis_vectors.at(1); }
    vector3&  basis_vector_z_ref() { return m_basis_vectors.at(2); }
    vector3&  basis_vector_ref(natural_32_bit const  index) { return m_basis_vectors.at(index); }

private:
    vector3  m_origin;
    std::array<vector3, 3U>  m_basis_vectors;
};

coordinate_system_explicit const&  get_world_coord_system_explicit();

void  translate(coordinate_system_explicit&  coord_system, vector3 const&  shift);
void  rotate(coordinate_system_explicit&  coord_system, quaternion const&  rotation);
void  relocate(coordinate_system_explicit&  coord_system, vector3 const&  shift, quaternion const&  rotation);
void  relocate(coordinate_system_explicit&  coord_system, quaternion const&  rotation, vector3 const&  shift);

inline void  from_base_matrix(coordinate_system_explicit const&  coord_system, matrix44&  output)
{
    compose_from_base_matrix(
            coord_system.origin(),
            coord_system.basis_vector_x(),
            coord_system.basis_vector_y(),
            coord_system.basis_vector_z(),
            output);
}

inline void  to_base_matrix(coordinate_system_explicit const&  coord_system, matrix44&  output)
{
    compose_to_base_matrix(
            coord_system.origin(),
            coord_system.basis_vector_x(),
            coord_system.basis_vector_y(),
            coord_system.basis_vector_z(),
            output);
}

inline vector3  vector3_from_coordinate_system(vector3 const&  u, coordinate_system_explicit const&  cse)
{
    return vector3_from_orthonormal_base(u, cse.basis_vector_x(), cse.basis_vector_y(), cse.basis_vector_z());
}


inline vector3  vector3_to_coordinate_system(vector3 const&  u, coordinate_system_explicit const&  cse)
{
    return vector3_to_orthonormal_base(u, cse.basis_vector_x(), cse.basis_vector_y(), cse.basis_vector_z());
}

inline vector3  point3_from_coordinate_system(vector3 const&  p, coordinate_system_explicit const&  cse)
{
    return point3_from_orthonormal_base(p, cse.origin(), cse.basis_vector_x(), cse.basis_vector_y(), cse.basis_vector_z());
}

inline vector3  point3_to_coordinate_system(vector3 const&  p, coordinate_system_explicit const&  cse)
{
    return point3_to_orthonormal_base(p, cse.origin(), cse.basis_vector_x(), cse.basis_vector_y(), cse.basis_vector_z());
}


struct coordinate_system
{
    static coordinate_system_ptr  create(vector3 const&  origin, quaternion const&  orientation);
    static coordinate_system_ptr  create(coordinate_system_explicit const&  coord_system_explicit);

    coordinate_system() : coordinate_system(vector3_zero(),quaternion_identity()) {}
    coordinate_system(vector3 const&  origin, quaternion const&  orientation);
    coordinate_system(vector3 const&  origin, matrix33 const&  rotation_matrix);
    coordinate_system(coordinate_system_explicit const&  coord_system_explicit);

    vector3 const&  origin() const { return m_origin; }
    quaternion const&  orientation() const { return m_orientation; }

    void set_origin(vector3 const&  new_origin) { m_origin = new_origin; }
    void set_orientation(quaternion const&  new_normalised_orientation);

    vector3&  origin_ref() { return m_origin; }
    quaternion&  orientation_ref() { return m_orientation; }

private:
    vector3  m_origin;
    quaternion  m_orientation;
};


void  translate(coordinate_system&  coord_system, vector3 const&  shift);
void  rotate(coordinate_system&  coord_system, quaternion const&  rotation);

void  integrate(
        coordinate_system&  coord_system,
        float_32_bit const  time_step_in_seconds,
        vector3 const&  linear_velocity,
        vector3 const&  angular_velocity
        );

inline vector3  vector3_from_coordinate_system(vector3 const&  u, coordinate_system const&  coord_system)
{
    return quaternion_to_rotation_matrix(coord_system.orientation()) * u;
}

inline vector3  vector3_to_coordinate_system(vector3 const&  u, coordinate_system const&  coord_system)
{
    return transpose33(quaternion_to_rotation_matrix(coord_system.orientation())) * u;
}

inline vector3  point3_from_coordinate_system(vector3 const&  p, coordinate_system const&  coord_system)
{
    return coord_system.origin() + vector3_from_coordinate_system(p, coord_system);
}

inline vector3  point3_to_coordinate_system(vector3 const&  p, coordinate_system const&  coord_system)
{
    return vector3_to_coordinate_system(p - coord_system.origin(), coord_system);
}

void  from_coordinate_system(coordinate_system const&  base, coordinate_system const&  subject, coordinate_system&  result);
void  to_coordinate_system(coordinate_system const&  base, coordinate_system const&  subject, coordinate_system&  result);

void  from_base_matrix(coordinate_system const&  coord_system, matrix44&  output);
void  to_base_matrix(coordinate_system const&  coord_system, matrix44&  output);

vector3  axis(coordinate_system const&  coord_system, natural_8_bit const  axis_index);
vector3  axis_x(coordinate_system const&  coord_system);
vector3  axis_y(coordinate_system const&  coord_system);
vector3  axis_z(coordinate_system const&  coord_system);
inline void  get_basis_vectors(coordinate_system const&  coord_system, vector3&  x, vector3&  y, vector3&  z)
{ rotation_matrix_to_basis(quaternion_to_rotation_matrix(coord_system.orientation()), x, y, z); }

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
