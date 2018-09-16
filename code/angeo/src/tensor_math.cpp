#include <angeo/tensor_math.hpp>
#include <utility/assumptions.hpp>
#include <utility/development.hpp>

#ifdef E2_USE_UBLAS_MATH_LIBRARY




#else


vector3  interpolate_linear(vector3 const&  u, vector3 const&  v, float_32_bit const  t)
{
    return u + t * (v - u);
}


quaternion  transform(quaternion const&  orientation, matrix44 const&  transformation)
{
    vector3  x, y, z;
    rotation_matrix_to_basis(quaternion_to_rotation_matrix(orientation), x, y, z);
    matrix33 rotation;
    basis_to_rotation_matrix(
        contract43(transformation * expand34(x, 0.0f)),
        contract43(transformation * expand34(y, 0.0f)),
        contract43(transformation * expand34(z, 0.0f)),
        rotation
        );
    return rotation_matrix_to_quaternion(rotation);
}


quaternion  interpolate_linear(quaternion const& u, quaternion const& v, float_32_bit const  t)
{
    quaternion q(u.coeffs() + t * (v.coeffs() - u.coeffs()));
    float_32_bit const  q_lenght2 = length_squared(q);
    if (std::fabsf(q_lenght2) > 0.001f)
        return quaternion((1.0f / std::sqrtf(q_lenght2)) * q.coeffs());
    else
        return quaternion_identity();
}

quaternion  interpolate_spherical(quaternion const& u, quaternion const& v, float_32_bit const  t)
{
    return u.slerp(t,v);
    //float_32_bit  dot = std::max(-1.0f,std::min(dot_product(u,v),1.0f));
    //if (dot > 0.999f)
    //    return interpolate_linear(u,v,t);

    //// If the dot product is negative, the quaternions
    //// have opposite handed-ness and slerp won't take
    //// the shorter path. Fix by reversing one quaternion.
    ////if (dot < 0.0f) {
    ////    v = quaternion(-v.coeffs());
    ////    dot = -dot;
    ////}

    //quaternion  w(v.coeffs() - dot * u.coeffs());
    //w.normalize();
    //float_32_bit const  theta = t * std::acosf(dot);
    //return quaternion(std::cosf(theta) * u.coeffs() + std::sinf(theta) * w.coeffs());
}


void  rotation_matrix_to_basis(
        matrix33 const&  R,
        vector3&  x_axis_unit_vector,
        vector3&  y_axis_unit_vector,
        vector3&  z_axis_unit_vector
        )
{
    x_axis_unit_vector(0) = R(0,0); x_axis_unit_vector(1) = R(1,0); x_axis_unit_vector(2) = R(2,0);
    y_axis_unit_vector(0) = R(0,1); y_axis_unit_vector(1) = R(1,1); y_axis_unit_vector(2) = R(2,1);
    z_axis_unit_vector(0) = R(0,2); z_axis_unit_vector(1) = R(1,2); z_axis_unit_vector(2) = R(2,2);
}


void  basis_to_rotation_matrix(
        vector3 const&  x_axis_unit_vector,
        vector3 const&  y_axis_unit_vector,
        vector3 const&  z_axis_unit_vector,
        matrix33&  R
        )
{
    R(0,0) = x_axis_unit_vector(0);  R(0,1) = y_axis_unit_vector(0);  R(0,2) = z_axis_unit_vector(0);
    R(1,0) = x_axis_unit_vector(1);  R(1,1) = y_axis_unit_vector(1);  R(1,2) = z_axis_unit_vector(1);
    R(2,0) = x_axis_unit_vector(2);  R(2,1) = y_axis_unit_vector(2);  R(2,2) = z_axis_unit_vector(2);
}


void  compose_from_base_matrix(
        vector3 const&  origin,
        matrix33 const&  rotation_matrix,
        matrix44&  result
        )
{
    result <<
        rotation_matrix(0, 0), rotation_matrix(0, 1), rotation_matrix(0, 2), origin(0),
        rotation_matrix(1, 0), rotation_matrix(1, 1), rotation_matrix(1, 2), origin(1),
        rotation_matrix(2, 0), rotation_matrix(2, 1), rotation_matrix(2, 2), origin(2),
        0.0f,                  0.0f,                  0.0f,                  1.0f
        ;
}


void  compose_from_base_matrix(
        vector3 const&  origin,
        vector3 const&  x_axis_unit_vector,
        vector3 const&  y_axis_unit_vector,
        vector3 const&  z_axis_unit_vector,
        matrix44&  M
        )
{
    M(0, 0) = x_axis_unit_vector(0);  M(0, 1) = y_axis_unit_vector(0);  M(0, 2) = z_axis_unit_vector(0);  M(0, 2) = origin(0);
    M(1, 0) = x_axis_unit_vector(1);  M(1, 1) = y_axis_unit_vector(1);  M(1, 2) = z_axis_unit_vector(1);  M(1, 2) = origin(1);
    M(2, 0) = x_axis_unit_vector(2);  M(2, 1) = y_axis_unit_vector(2);  M(2, 2) = z_axis_unit_vector(2);  M(2, 2) = origin(2);
    M(3, 0) = 0.0f;                   M(3, 1) = 0.0f;                   M(3, 2) = 0.0f;                   M(3, 3) = 1.0f;
}


void  decompose_from_base_matrix(
        matrix44 const&  M,
        vector3&  origin,
        matrix33&  rotation_matrix
        )
{
    NOT_IMPLEMENTED_YET();
}


void  decompose_from_base_matrix(
        matrix44 const&  M,
        vector3&  origin,
        vector3&  x_axis_unit_vector,
        vector3&  y_axis_unit_vector,
        vector3&  z_axis_unit_vector
        )
{
    NOT_IMPLEMENTED_YET();
}


void  compose_to_base_matrix(
        vector3 const&  origin,
        matrix33 const&  rotation_matrix,
        matrix44&  result
        )
{
    vector3 const  p {
        -(rotation_matrix(0, 0) * origin(0) + rotation_matrix(1, 0) * origin(1) + rotation_matrix(2, 0) * origin(2)),
        -(rotation_matrix(0, 1) * origin(0) + rotation_matrix(1, 1) * origin(1) + rotation_matrix(2, 1) * origin(2)),
        -(rotation_matrix(0, 2) * origin(0) + rotation_matrix(1, 2) * origin(1) + rotation_matrix(2, 2) * origin(2))
    };
    result <<
        rotation_matrix(0, 0), rotation_matrix(1, 0), rotation_matrix(2, 0), p(0),
        rotation_matrix(0, 1), rotation_matrix(1, 1), rotation_matrix(2, 1), p(1),
        rotation_matrix(0, 2), rotation_matrix(1, 2), rotation_matrix(2, 2), p(2),
        0.0f,                  0.0f,                  0.0f,                  1.0f
        ;
}


void  compose_to_base_matrix(
        vector3 const&  origin,
        vector3 const&  x_axis_unit_vector,
        vector3 const&  y_axis_unit_vector,
        vector3 const&  z_axis_unit_vector,
        matrix44&  M
        )
{
    M(0, 0) = x_axis_unit_vector(0);  M(0, 1) = x_axis_unit_vector(1);  M(0, 2) = x_axis_unit_vector(2);  M(0, 2) = -dot_product(x_axis_unit_vector, origin);
    M(1, 0) = y_axis_unit_vector(0);  M(1, 1) = y_axis_unit_vector(1);  M(1, 2) = y_axis_unit_vector(2);  M(1, 2) = -dot_product(y_axis_unit_vector, origin);
    M(2, 0) = z_axis_unit_vector(0);  M(2, 1) = z_axis_unit_vector(1);  M(2, 2) = z_axis_unit_vector(2);  M(2, 2) = -dot_product(z_axis_unit_vector, origin);
    M(3, 0) = 0.0f;                   M(3, 1) = 0.0f;                   M(3, 2) = 0.0f;                   M(3, 3) = 1.0f;
}


void  decompose_to_base_matrix(
        matrix44 const&  M,
        vector3&  origin,
        matrix33&  rotation_matrix
        )
{
    NOT_IMPLEMENTED_YET();
}


void  decompose_to_base_matrix(
        matrix44 const&  M,
        vector3&  origin,
        vector3&  x_axis_unit_vector,
        vector3&  y_axis_unit_vector,
        vector3&  z_axis_unit_vector
        )
{
    NOT_IMPLEMENTED_YET();
}


matrix33  yaw_pitch_roll_to_rotation(scalar const  yaw, scalar const  pitch, scalar const  roll)
{
    // yaw-pitch-roll ~ zy'x''
    scalar const  c1 = std::cos(yaw);
    scalar const  s1 = std::sin(yaw);
    scalar const  c2 = std::cos(pitch);
    scalar const  s2 = std::sin(pitch);
    scalar const  c3 = std::cos(roll);
    scalar const  s3 = std::sin(roll);
    matrix33  result;
    result <<   c1*c2,      c1*s2*s3 - c3*s1,       s1*s3 + c1*c3*s2,
                c2*s1,      c1*c3 + s1*s2*s3,       c3*s1*s2 - c1*s3,
                -s2,        c2*s3,                  c2*c3
                ;
    return result;
}

void  rotation_to_yaw_pitch_roll(matrix33 const&  R, scalar&  yaw, scalar&  pitch, scalar&  roll)
{
    yaw = std::atan2(R(1,0),R(0,0));
    pitch = -( R(2,0) >= 1.0f ? PI()/2.0f : R(2,0) <= -1.0f ? -PI()/2.0f : std::asin(R(2,0)) );
    roll = std::atan2(R(2,1),R(2,2));
}


#endif
