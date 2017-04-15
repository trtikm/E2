#include <angeo/tensor_math.hpp>
#include <utility/assumptions.hpp>

#ifdef E2_USE_UBLAS_MATH_LIBRARY




#else


vector3  interpolate_linear(vector3 const&  u, vector3 const&  v, float_32_bit const  t)
{
    return u + t * (v - u);
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


void  basis_to_rotation_matrix(vector3 const&  x_axis_unit_vector,
                               vector3 const&  y_axis_unit_vector,
                               vector3 const&  z_axis_unit_vector,
                               matrix33&  R)
{
    R(0,0) = x_axis_unit_vector(0);  R(0,1) = y_axis_unit_vector(0);  R(0,2) = z_axis_unit_vector(0);
    R(1,0) = x_axis_unit_vector(1);  R(1,1) = y_axis_unit_vector(1);  R(1,2) = z_axis_unit_vector(1);
    R(2,0) = x_axis_unit_vector(2);  R(2,1) = y_axis_unit_vector(2);  R(2,2) = z_axis_unit_vector(2);
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
