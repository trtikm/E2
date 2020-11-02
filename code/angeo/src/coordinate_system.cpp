#include <angeo/coordinate_system.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>

namespace angeo {


coordinate_system_explicit::coordinate_system_explicit(vector3 const&  origin_, matrix33 const&  rotation_matrix_)
    : m_origin{ origin_ }
    , m_basis_vectors()
{
    rotation_matrix_to_basis(rotation_matrix_, m_basis_vectors.at(0), m_basis_vectors.at(1), m_basis_vectors.at(2));
}

coordinate_system_explicit::coordinate_system_explicit(coordinate_system const&  coord_system_)
    : coordinate_system_explicit(coord_system_.origin(), quaternion_to_rotation_matrix(coord_system_.orientation()))
{}


coordinate_system_explicit const&  get_world_coord_system_explicit()
{
    static coordinate_system_explicit const  world(vector3_zero(), vector3_unit_x(), vector3_unit_y(), vector3_unit_z());
    return world;
}


coordinate_system_ptr  coordinate_system::create(vector3 const&  origin, quaternion const&  orientation)
{
    return coordinate_system_ptr( new coordinate_system(origin,orientation) );
}

coordinate_system_ptr  coordinate_system::create(coordinate_system_explicit const&  coord_system_explicit)
{
    return coordinate_system_ptr( new coordinate_system(coord_system_explicit) );
}

coordinate_system::coordinate_system(vector3 const&  origin, quaternion const&  orientation)
    : m_origin(origin)
    , m_orientation(orientation)
{
    ASSUMPTION(are_equal(length_squared(m_orientation),1));
}

coordinate_system::coordinate_system(vector3 const&  origin, matrix33 const&  rotation_matrix)
    : coordinate_system(origin, rotation_matrix_to_quaternion(rotation_matrix))
{}

coordinate_system::coordinate_system(coordinate_system_explicit const&  coord_system_explicit)
    : coordinate_system(
            coord_system_explicit.origin(),
            [&coord_system_explicit]() -> matrix33 {
                matrix33 R;
                basis_to_rotation_matrix(
                        coord_system_explicit.basis_vector_x(),
                        coord_system_explicit.basis_vector_y(),
                        coord_system_explicit.basis_vector_z(),
                        R
                        );
                return R;
                }()
            )
{}

void coordinate_system::set_orientation(quaternion const&  new_normalised_orientation)
{
    ASSUMPTION(are_equal(length_squared(new_normalised_orientation),1));
    m_orientation = new_normalised_orientation;
}


void  translate(coordinate_system&  coord_system, vector3 const&  shift)
{
    coord_system.set_origin( coord_system.origin() + shift );
}


void  rotate(coordinate_system&  coord_system, quaternion const&  rotation)
{
    quaternion  composed = rotation * coord_system.orientation();
    normalise(composed);
    coord_system.set_orientation( composed );
}

void  integrate(
        coordinate_system& coord_system,
        float_32_bit const  time_step_in_seconds,
        vector3 const& linear_velocity,
        vector3 const& angular_velocity
        )
{
    coord_system.set_origin(coord_system.origin() + time_step_in_seconds * linear_velocity);
    quaternion const  orientation_derivative = scale(0.5f, make_quaternion(0.0f, angular_velocity) * coord_system.orientation());
    coord_system.set_orientation( normalised(coord_system.orientation() + scale(time_step_in_seconds, orientation_derivative)) );
}

void  from_coordinate_system(coordinate_system const&  base, coordinate_system const&  subject, coordinate_system&  result)
{
    //      F(r) = F(b)*F(s)
    matrix44  Fb, Fs;
    from_base_matrix(base, Fb);
    from_base_matrix(subject, Fs);
    matrix33 R;
    decompose_matrix44(Fb * Fs, result.origin_ref(), R);
    result.orientation_ref() = rotation_matrix_to_quaternion(R);
}

void  to_coordinate_system(coordinate_system const&  base, coordinate_system const&  subject, coordinate_system&  result)
{
    //      F(s) = F(b)*F(r)
    // T(b)*F(s) =      F(r)
    matrix44  Tb, Fs;
    to_base_matrix(base, Tb);
    from_base_matrix(subject, Fs);
    matrix33 R;
    decompose_matrix44(Tb * Fs, result.origin_ref(), R);
    result.orientation_ref() = rotation_matrix_to_quaternion(R);
}

void  from_base_matrix(coordinate_system const&  coord_system, matrix44&  output)
{
    compose_from_base_matrix(coord_system.origin(), quaternion_to_rotation_matrix(coord_system.orientation()), output);
}


void  to_base_matrix(coordinate_system const&  coord_system, matrix44&  output)
{
    compose_to_base_matrix(coord_system.origin(), quaternion_to_rotation_matrix(coord_system.orientation()), output);
}


vector3  axis(coordinate_system const&  coord_system, natural_8_bit const  axis_index)
{
    ASSUMPTION(axis_index == 0U || axis_index == 1U || axis_index == 2U);
    matrix33 const  rotation = quaternion_to_rotation_matrix(coord_system.orientation());
    return vector3(rotation(0,axis_index),rotation(1,axis_index),rotation(2,axis_index));
}

vector3  axis_x(coordinate_system const&  coord_system)
{
    return axis(coord_system,0U);
}

vector3  axis_y(coordinate_system const&  coord_system)
{
    return axis(coord_system,1U);
}

vector3  axis_z(coordinate_system const&  coord_system)
{
    return axis(coord_system,2U);
}


void  interpolate_linear(
        coordinate_system const&  head,
        coordinate_system const&  tail,
        float_32_bit const  parameter, // it must be in range <0,1>
        coordinate_system&  result
        )
{
    result.set_origin(::interpolate_linear(head.origin(),tail.origin(),parameter));
    result.set_orientation(::interpolate_linear(head.orientation(),tail.orientation(),parameter));
}

void  interpolate_spherical(
        coordinate_system const&  head,
        coordinate_system const&  tail,
        float_32_bit const  parameter, // it must be in range <0,1>
        coordinate_system&  result
        )
{
    result.set_origin(::interpolate_linear(head.origin(),tail.origin(),parameter));
    result.set_orientation(::interpolate_spherical(head.orientation(),tail.orientation(),parameter));
}


}
