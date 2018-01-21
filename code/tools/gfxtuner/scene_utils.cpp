#include <gfxtuner/scene_utils.hpp>
#include <utility/timeprof.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>


void  transform_origin_and_orientation(
        matrix44 const  transformation,
        vector3&  origin,
        quaternion&  orientation
        )
{
    origin = contract43(transformation * expand34(origin));
    vector3  x, y, z;
    rotation_matrix_to_basis(quaternion_to_rotation_matrix(orientation), x, y, z);
    matrix33 rotation;
    basis_to_rotation_matrix(
        contract43(transformation * expand34(x, 0.0f)),
        contract43(transformation * expand34(y, 0.0f)),
        contract43(transformation * expand34(z, 0.0f)),
        rotation
        );
    orientation = rotation_matrix_to_quaternion(rotation);
}
