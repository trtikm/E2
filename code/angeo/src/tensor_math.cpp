#include <angeo/tensor_math.hpp>
#include <utility/assumptions.hpp>
#include <utility/development.hpp>

#ifdef E2_USE_UBLAS_MATH_LIBRARY




#else


static bool  are_equal(float const* const  left, float const* const  right, int const  n, float_32_bit const  epsilon)
{
    for (int i = 0; i != n; ++i)
        if (!are_equal(left[i], right[i], epsilon))
            return false;
    return true;
}


bool are_equal_2d(vector2 const&  left, vector2 const&  right, float_32_bit const  epsilon)
{
    return are_equal(left.data(), right.data(), 2, epsilon);
}

bool are_equal_3d(vector3 const&  left, vector3 const&  right, float_32_bit const  epsilon)
{
    return are_equal(left.data(), right.data(), 3, epsilon);
}

bool are_equal_4d(vector4 const&  left, vector4 const&  right, float_32_bit const  epsilon)
{
    return are_equal(left.data(), right.data(), 4, epsilon);
}

bool are_equal_6d(vector6 const&  left, vector6 const&  right, float_32_bit const  epsilon)
{
    return are_equal(left.data(), right.data(), 6, epsilon);
}


bool are_equal(quaternion const&  left, quaternion const&  right, float_32_bit const  epsilon)
{
    return are_equal(left.coeffs().data(), right.coeffs().data(), 4, epsilon);
}


bool are_equal_22(matrix22 const&  left, matrix22 const&  right, float_32_bit const  epsilon)
{
    return are_equal(left.data(), right.data(), 2 * 2, epsilon);
}

bool are_equal_32(matrix32 const&  left, matrix32 const&  right, float_32_bit const  epsilon)
{
    return are_equal(left.data(), right.data(), 3 * 2, epsilon);
}

bool are_equal_33(matrix33 const&  left, matrix33 const&  right, float_32_bit const  epsilon)
{
    return are_equal(left.data(), right.data(), 3 * 3, epsilon);
}

bool are_equal_43(matrix43 const&  left, matrix43 const&  right, float_32_bit const  epsilon)
{
    return are_equal(left.data(), right.data(), 4 * 3, epsilon);
}

bool are_equal_44(matrix44 const&  left, matrix44 const&  right, float_32_bit const  epsilon)
{
    return are_equal(left.data(), right.data(), 4 * 4, epsilon);
}


scalar  cos_angle_2d(vector2 const& u, vector2 const& v)
{
    scalar const  denom = length_2d(u) * length_2d(v);
    return denom < 1e-5f ? 0.0f : std::min(std::max(-1.0f, dot_product_2d(u, v) / denom), 1.0f);
}


vector3  orthogonal(vector3 const&  u)
{
    scalar const x = std::abs(u(0));
    scalar const y = std::abs(u(1));
    scalar const z = std::abs(u(2));

    if (x < y)
        if (x < z)
            return cross_product(u, vector3_unit_x());
        else
            return cross_product(u, vector3_unit_z());
    else
        if (y < z)
            return cross_product(u, vector3_unit_y());
        else
            return cross_product(u, vector3_unit_z());
}


scalar  cos_angle(vector3 const& u, vector3 const& v)
{
    scalar const  denom = length(u) * length(v);
    return denom < 1e-5f ? 0.0f : std::min(std::max(-1.0f, dot_product(u, v) / denom), 1.0f);
}


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


vector3  vector3_to_orthonormal_base(vector3 const&  u, vector3 const&  x_axis, vector3 const&  y_axis, vector3 const&  z_axis)
{
    return { dot_product(u, x_axis), dot_product(u, y_axis), dot_product(u, z_axis) };
}


vector3  point3_to_orthonormal_base(
        vector3 const&  p,
        vector3 const& origin, vector3 const&  x_axis, vector3 const&  y_axis, vector3 const&  z_axis
        )
{
    return vector3_to_orthonormal_base(p - origin, x_axis, y_axis, z_axis);
}


vector3  vector3_from_orthonormal_base(vector3 const&  u, vector3 const&  x_axis, vector3 const&  y_axis, vector3 const&  z_axis)
{
    return u(0) * x_axis + u(1) * y_axis + u(2) * z_axis;
}


vector3  point3_from_orthonormal_base(
        vector3 const&  p,
        vector3 const& origin, vector3 const&  x_axis, vector3 const&  y_axis, vector3 const&  z_axis
        )
{
    return origin + vector3_from_orthonormal_base(p, x_axis, y_axis, z_axis);
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


matrix33  row_vectors_to_matrix(vector3 const&  row_0, vector3 const&  row_1, vector3 const&  row_2)
{
    matrix33  R;
    R(0, 0) = row_0(0);  R(0, 1) = row_0(1);  R(0, 2) = row_0(2);
    R(1, 0) = row_1(0);  R(1, 1) = row_1(1);  R(1, 2) = row_1(2);
    R(2, 0) = row_2(0);  R(2, 1) = row_2(1);  R(2, 2) = row_2(2);
    return R;
}


matrix33  column_vectors_to_matrix(vector3 const&  col_0, vector3 const&  col_1, vector3 const&  col_2)
{
    matrix33  R;
    R(0, 0) = col_0(0);  R(0, 1) = col_1(0);  R(0, 2) = col_2(0);
    R(1, 0) = col_0(1);  R(1, 1) = col_1(1);  R(1, 2) = col_2(1);
    R(2, 0) = col_0(2);  R(2, 1) = col_1(2);  R(2, 2) = col_2(2);
    return R;
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
        matrix33 const&  rot_matrix,
        matrix44&  result
        )
{
    result <<
        rot_matrix(0, 0), rot_matrix(0, 1), rot_matrix(0, 2), origin(0),
        rot_matrix(1, 0), rot_matrix(1, 1), rot_matrix(1, 2), origin(1),
        rot_matrix(2, 0), rot_matrix(2, 1), rot_matrix(2, 2), origin(2),
        0.0f,             0.0f,             0.0f,             1.0f
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
    M(0, 0) = x_axis_unit_vector(0);  M(0, 1) = y_axis_unit_vector(0);  M(0, 2) = z_axis_unit_vector(0);  M(0, 3) = origin(0);
    M(1, 0) = x_axis_unit_vector(1);  M(1, 1) = y_axis_unit_vector(1);  M(1, 2) = z_axis_unit_vector(1);  M(1, 3) = origin(1);
    M(2, 0) = x_axis_unit_vector(2);  M(2, 1) = y_axis_unit_vector(2);  M(2, 2) = z_axis_unit_vector(2);  M(2, 3) = origin(2);
    M(3, 0) = 0.0f;                   M(3, 1) = 0.0f;                   M(3, 2) = 0.0f;                   M(3, 3) = 1.0f;
}


void  compose_to_base_matrix(
        vector3 const&  origin,
        matrix33 const&  rot_matrix,
        matrix44&  result
        )
{
    vector3 const  p {
        -(rot_matrix(0, 0) * origin(0) + rot_matrix(1, 0) * origin(1) + rot_matrix(2, 0) * origin(2)),
        -(rot_matrix(0, 1) * origin(0) + rot_matrix(1, 1) * origin(1) + rot_matrix(2, 1) * origin(2)),
        -(rot_matrix(0, 2) * origin(0) + rot_matrix(1, 2) * origin(1) + rot_matrix(2, 2) * origin(2))
    };
    result <<
        rot_matrix(0, 0), rot_matrix(1, 0), rot_matrix(2, 0), p(0),
        rot_matrix(0, 1), rot_matrix(1, 1), rot_matrix(2, 1), p(1),
        rot_matrix(0, 2), rot_matrix(1, 2), rot_matrix(2, 2), p(2),
        0.0f,             0.0f,             0.0f,             1.0f
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
    M(0, 0) = x_axis_unit_vector(0);  M(0, 1) = x_axis_unit_vector(1);  M(0, 2) = x_axis_unit_vector(2);  M(0, 3) = -dot_product(x_axis_unit_vector, origin);
    M(1, 0) = y_axis_unit_vector(0);  M(1, 1) = y_axis_unit_vector(1);  M(1, 2) = y_axis_unit_vector(2);  M(1, 3) = -dot_product(y_axis_unit_vector, origin);
    M(2, 0) = z_axis_unit_vector(0);  M(2, 1) = z_axis_unit_vector(1);  M(2, 2) = z_axis_unit_vector(2);  M(2, 3) = -dot_product(z_axis_unit_vector, origin);
    M(3, 0) = 0.0f;                   M(3, 1) = 0.0f;                   M(3, 2) = 0.0f;                   M(3, 3) = 1.0f;
}


void  decompose_matrix44(
        matrix44 const&  M,
        vector3&  origin,
        matrix33&  rot_matrix
        )
{
    origin = translation_vector(M);
    rotation_matrix(M, rot_matrix);
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


matrix33&  rotation_matrix(matrix44 const&  M, matrix33&  R)
{
    R(0, 0) = M(0, 0);  R(0, 1) = M(0, 1);  R(0, 2) = M(0, 2);
    R(1, 0) = M(1, 0);  R(1, 1) = M(1, 1);  R(1, 2) = M(1, 2);
    R(2, 0) = M(2, 0);  R(2, 1) = M(2, 1);  R(2, 2) = M(2, 2);
    return R;
}


matrix33  rotation_matrix(matrix44 const&  M)
{
    matrix33  R;
    rotation_matrix(M, R);
    return R;
}


#endif
