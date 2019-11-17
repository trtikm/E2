#ifndef ANGEO_TENSOR_MATH_HPP_INCLUDED
#   define ANGEO_TENSOR_MATH_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>


#   ifdef E2_USE_UBLAS_MATH_LIBRARY


#   include <boost/numeric/ublas/vector.hpp>
#   include <boost/numeric/ublas/matrix.hpp>
#   include <boost/math/quaternion.hpp>

typedef boost::numeric::ublas::c_vector<scalar,2>  vector2;
typedef boost::numeric::ublas::c_vector<scalar,3>  vector3;
typedef boost::numeric::ublas::c_vector<scalar,4>  vector4;
typedef boost::numeric::ublas::c_vector<scalar,6>  vector6;

typedef boost::math::quaternion<scalar>  quaternion;

typedef boost::numeric::ublas::c_matrix<scalar,2,2>  matrix22;
typedef boost::numeric::ublas::c_matrix<scalar,3,2>  matrix32;
typedef boost::numeric::ublas::c_matrix<scalar,3,3>  matrix33;
typedef boost::numeric::ublas::c_matrix<scalar,4,3>  matrix43;
typedef boost::numeric::ublas::c_matrix<scalar,4,4>  matrix44;


#   else    // Otherwise use the default math library: Eigen


#   include <Eigen/Dense>


typedef float_32_bit  scalar;

typedef Eigen::Matrix<float_32_bit,2,1>  vector2;
typedef Eigen::Matrix<float_32_bit,3,1>  vector3;
typedef Eigen::Matrix<float_32_bit,4,1>  vector4;
typedef Eigen::Matrix<float_32_bit,6,1>  vector6;

typedef Eigen::Quaternion<float_32_bit>  quaternion;

typedef Eigen::Matrix<float_32_bit,2,2>  matrix22;
typedef Eigen::Matrix<float_32_bit,3,2>  matrix32;
typedef Eigen::Matrix<float_32_bit,3,3>  matrix33;
typedef Eigen::Matrix<float_32_bit,4,3>  matrix43;
typedef Eigen::Matrix<float_32_bit,4,4>  matrix44;

bool are_equal_2d(vector2 const&  left, vector2 const&  right, float_32_bit const  epsilon);
bool are_equal_3d(vector3 const&  left, vector3 const&  right, float_32_bit const  epsilon);
bool are_equal_4d(vector4 const&  left, vector4 const&  right, float_32_bit const  epsilon);
bool are_equal_6d(vector6 const&  left, vector6 const&  right, float_32_bit const  epsilon);

bool are_equal(quaternion const&  left, quaternion const&  right, float_32_bit const  epsilon);

bool are_equal_22(matrix22 const&  left, matrix22 const&  right, float_32_bit const  epsilon);
bool are_equal_32(matrix32 const&  left, matrix32 const&  right, float_32_bit const  epsilon);
bool are_equal_33(matrix33 const&  left, matrix33 const&  right, float_32_bit const  epsilon);
bool are_equal_43(matrix43 const&  left, matrix43 const&  right, float_32_bit const  epsilon);
bool are_equal_44(matrix44 const&  left, matrix44 const&  right, float_32_bit const  epsilon);

inline vector2  vector2_zero() { return vector2::Zero(); }
inline vector2  vector2_unit_x() { return vector2::UnitX(); }
inline vector2  vector2_unit_y() { return vector2::UnitY(); }
inline scalar  dot_product_2d(vector2 const& u, vector2 const& v) { return u.dot(v); }
inline scalar  length_squared_2d(vector2 const& u) { return dot_product_2d(u,u); }
inline scalar  length_2d(vector2 const& u) { return u.norm(); }
inline vector2  normalised_2d(vector2 const&  u) { return u.normalized(); }
inline void  normalise_2d(vector2& u) { u.normalize(); }
inline vector2  orthogonal_2d(vector2 const&  u) { return { -u(1), u(0) }; }
scalar  cos_angle_2d(vector2 const& u, vector2 const& v);
inline scalar  angle_2d(vector2 const& u, vector2 const& v) { return std::acosf(cos_angle_2d(u, v)); }

inline vector3  expand23(vector2 const& u, scalar h=scalar(1.0)) { return { u(0), u(1), h }; }
inline vector2  contract32(vector3 const& u) { return { u(0), u(1) }; }

inline vector3  vector3_zero() { return vector3::Zero(); }
inline vector3  vector3_unit_x() { return vector3::UnitX(); }
inline vector3  vector3_unit_y() { return vector3::UnitY(); }
inline vector3  vector3_unit_z() { return vector3::UnitZ(); }
inline scalar  dot_product(vector3 const& u, vector3 const& v) { return u.dot(v); }
inline vector3  cross_product(vector3 const& u, vector3 const& v) { return u.cross(v); }
inline scalar  length_squared(vector3 const& u) { return dot_product(u,u); }
inline scalar  length(vector3 const& u) { return u.norm(); }
inline vector3  normalised(vector3 const&  u) { return u.normalized(); }
inline void  normalise(vector3& u) { u.normalize(); }
vector3  orthogonal(vector3 const&  u);
inline vector3  project_to_unit_vector(vector3 const&  u, vector3 const&  unit_target) { return dot_product(u, unit_target) * unit_target; }
inline vector3  project_to_vector(vector3 const&  u, vector3 const&  target) { return (dot_product(u, target) / dot_product(target, target)) * target; }
scalar  cos_angle(vector3 const& u, vector3 const& v);
inline scalar  angle(vector3 const& u, vector3 const& v) { return std::acosf(cos_angle(u,v)); }

inline vector4  expand34(vector3 const& u, scalar h=scalar(1.0)) { return { u(0), u(1), u(2), h }; }
inline vector3  contract43(vector4 const& u) { return { u(0), u(1), u(2) }; }
inline scalar  length_4d(vector4 const& u) { return u.norm(); }

inline vector3  transform_point(vector3 const& u, matrix44 const& M, scalar const h=scalar(1.0))
{ return contract43(M * expand34(u,h)); }
inline vector3  transform_vector(vector3 const& u, matrix44 const& M) { return transform_point(u,M,scalar(0.0)); }

vector3  vector3_to_orthonormal_base(vector3 const&  u, vector3 const&  x_axis, vector3 const&  y_axis, vector3 const&  z_axis);
vector3  point3_to_orthonormal_base(
        vector3 const&  p,
        vector3 const& origin, vector3 const&  x_axis, vector3 const&  y_axis, vector3 const&  z_axis
        );
vector3  vector3_from_orthonormal_base(vector3 const&  u, vector3 const&  x_axis, vector3 const&  y_axis, vector3 const&  z_axis);
vector3  point3_from_orthonormal_base(
        vector3 const&  p,
        vector3 const& origin, vector3 const&  x_axis, vector3 const&  y_axis, vector3 const&  z_axis
        );

vector3  interpolate_linear(vector3 const&  u, vector3 const&  v, float_32_bit const  t);

inline vector4  vector4_zero() { return vector4::Zero(); }
inline vector4  vector4_unit_x() { return vector4::UnitX(); }
inline vector4  vector4_unit_y() { return vector4::UnitY(); }
inline vector4  vector4_unit_z() { return vector4::UnitZ(); }
inline vector4  vector4_unit_w() { return vector4::UnitW(); }

inline vector6  vector6_zero() { return vector6::Zero(); }

inline quaternion  quaternion_identity() { return quaternion::Identity(); }
inline quaternion  make_quaternion(scalar const w, vector3 const& v) { return quaternion(w, v(0), v(1), v(2)); }
inline quaternion  make_quaternion_wxyz(scalar const w, scalar const x, scalar const y, scalar const z) { return quaternion(w,x,y,z); }
inline quaternion  make_quaternion_xyzw(scalar const x, scalar const y, scalar const z, scalar const w) { return quaternion(w,x,y,z); }
inline quaternion  operator+(quaternion const&  p, quaternion const&  q)
{
    return{ p.coeffs()(3) + q.coeffs()(3), p.coeffs()(0) + q.coeffs()(0), p.coeffs()(1) + q.coeffs()(1), p.coeffs()(2) + q.coeffs()(2) };
}
inline quaternion  scale(scalar const s, quaternion const& q)
{
    return{ s*q.coeffs()(3), s*q.coeffs()(0), s*q.coeffs()(1), s*q.coeffs()(2) };
}
inline scalar  length_squared(quaternion const& q) { return q.squaredNorm(); }
inline scalar  length(quaternion const& q) { return q.norm(); }
inline quaternion  normalised(quaternion const& q) { return q.normalized(); }
inline void  normalise(quaternion& q) { q.normalize(); }
inline scalar  dot_product(quaternion const& u, quaternion const& v) { return u.dot(v); }
inline vector4 const&  quaternion_coefficients_wxyz(quaternion const& q) { return { q.coeffs()(3), q.coeffs()(0), q.coeffs()(1), q.coeffs()(2) }; }
inline vector4 const&  quaternion_coefficients_xyzw(quaternion const& q) { return q.coeffs(); }

quaternion  transform(quaternion const&  orientation, matrix44 const&  transformation);

quaternion  interpolate_linear(quaternion const& u, quaternion const& v, float_32_bit const  t);
quaternion  interpolate_spherical(quaternion const& u, quaternion const& v, float_32_bit const  t);

inline matrix22 matrix22_identity() { return matrix22::Identity(); }
inline matrix32 matrix32_identity() { return matrix32::Identity(); }
inline matrix33 matrix33_identity() { return matrix33::Identity(); }
inline matrix43 matrix43_identity() { return matrix43::Identity(); }
inline matrix44 matrix44_identity() { return matrix44::Identity(); }

inline matrix22 matrix22_zero() { return matrix22::Zero(); }
inline matrix32 matrix32_zero() { return matrix32::Zero(); }
inline matrix33 matrix33_zero() { return matrix33::Zero(); }
inline matrix43 matrix43_zero() { return matrix43::Zero(); }
inline matrix44 matrix44_zero() { return matrix44::Zero(); }

inline matrix22  transpose22(matrix22 const&  M) { return M.transpose(); }
inline matrix33  transpose33(matrix33 const&  M) { return M.transpose(); }
inline matrix44  transpose44(matrix44 const&  M) { return M.transpose(); }

inline matrix22  inverse22(matrix22 const&  M) { return M.inverse(); }
inline matrix33  inverse33(matrix33 const&  M) { return M.inverse(); }
inline matrix44  inverse44(matrix44 const&  M) { return M.inverse(); }

inline matrix33  quaternion_to_rotation_matrix(quaternion const& q) { return q.toRotationMatrix(); }
inline quaternion  rotation_matrix_to_quaternion(matrix33 const&  R) { return quaternion(R); }
void  rotation_matrix_to_basis(
        matrix33 const&  R,
        vector3&  x_axis_unit_vector,
        vector3&  y_axis_unit_vector,
        vector3&  z_axis_unit_vector
        );
void  basis_to_rotation_matrix(
        vector3 const&  x_axis_unit_vector,
        vector3 const&  y_axis_unit_vector,
        vector3 const&  z_axis_unit_vector,
        matrix33&  R);
void  compose_from_base_matrix(
        vector3 const&  origin,
        matrix33 const&  rot_matrix,
        matrix44&  result
        );
void  compose_from_base_matrix(
        vector3 const&  origin,
        vector3 const&  x_axis_unit_vector,
        vector3 const&  y_axis_unit_vector,
        vector3 const&  z_axis_unit_vector,
        matrix44&  M
        );
void  compose_to_base_matrix(
        vector3 const&  origin,
        matrix33 const&  rot_matrix,   // Not transposed! The function applies the transpose automatically
        matrix44&  result
        );
void  compose_to_base_matrix(
        vector3 const&  origin,
        vector3 const&  x_axis_unit_vector,
        vector3 const&  y_axis_unit_vector,
        vector3 const&  z_axis_unit_vector,
        matrix44&  M
        );
void  decompose_matrix44(
        matrix44 const&  M,
        vector3&  origin,
        matrix33&  rot_matrix
        );
inline quaternion  angle_axis_to_quaternion(scalar const  angle, vector3 const&  axis) {
    return quaternion(Eigen::AngleAxis<scalar>(angle,axis));
}
inline scalar quaternion_to_angle_axis(quaternion const& q, vector3&  axis) {
    Eigen::AngleAxis<scalar> tmp(q);
    axis = tmp.axis();
    return tmp.angle();
}
matrix33  yaw_pitch_roll_to_rotation(scalar const  yaw, scalar const  pitch, scalar const  roll);
void  rotation_to_yaw_pitch_roll(matrix33 const&  R, scalar&  yaw, scalar&  pitch, scalar&  roll);

inline vector3  translation_vector(matrix44 const&  M) { return vector3(M(0, 3), M(1, 3), M(2, 3)); }
matrix33&  rotation_matrix(matrix44 const&  M, matrix33&  R);
matrix33  rotation_matrix(matrix44 const&  M);

//template<typename T, int nrows, int ncols>
//inline Eigen::ArrayWrapper< Eigen::Matrix<T, nrows, ncols> > as_array(Eigen::Matrix<T, nrows, ncols> const&  u)
//{
//    return u.array();
//}

template<typename T, int nrows, int ncols>
inline T min_element(Eigen::Matrix<T, nrows, ncols> const&  u)
{
    return u.minCoeff();
}


#   endif


inline constexpr scalar PI() noexcept { return scalar(3.14159265); }
inline constexpr scalar E() noexcept { return scalar(2.7182818); }

inline scalar  absolute_value(scalar const a) { return a < scalar(0) ? -a : a; }

inline bool  are_equal(scalar const a, scalar const b, scalar const  error = scalar(1e-5)) {
    return absolute_value(a - b) <= error;
}



#endif
