#ifndef UTILITY_TENSOR_MATH_HPP_INCLUDED
#   define UTILITY_TENSOR_MATH_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>


typedef float_32_bit  scalar;


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


typedef Eigen::Matrix<scalar,2,1>  vector2;
typedef Eigen::Matrix<scalar,3,1>  vector3;
typedef Eigen::Matrix<scalar,4,1>  vector4;
typedef Eigen::Matrix<scalar,6,1>  vector6;

typedef Eigen::Quaternion<scalar>  quaternion;

typedef Eigen::Matrix<scalar,2,2>  matrix22;
typedef Eigen::Matrix<scalar,3,2>  matrix32;
typedef Eigen::Matrix<scalar,3,3>  matrix33;
typedef Eigen::Matrix<scalar,4,3>  matrix43;
typedef Eigen::Matrix<scalar,4,4>  matrix44;

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


inline vector3  vector3_zero() { return vector3::Zero(); }
inline vector3  vector3_unit_x() { return vector3::UnitX(); }
inline vector3  vector3_unit_y() { return vector3::UnitY(); }
inline vector3  vector3_unit_z() { return vector3::UnitZ(); }
inline scalar  dot_product(vector3 const& u, vector3 const& v) { return u.dot(v); }
inline scalar  lenght(vector3 const& u) { return u.norm(); }
inline vector3  normalised(vector3 const&  u) { return u.normalized(); }
inline void  normalise(vector3& u) { u.normalize(); }

inline quaternion  quaternion_identity() { return quaternion::Identity(); }
inline scalar  length_squared(quaternion const& q) { return q.squaredNorm(); }
inline scalar  lenght(quaternion const& q) { return q.norm(); }
inline quaternion  normalised(quaternion const& q) { return q.normalized(); }
inline void  normalise(quaternion& q) { q.normalize(); }

inline matrix44  inverse(matrix44 const&  M) { return M.inverse(); }

inline matrix33  quaternion_to_rotation_matrix(quaternion const& q) { return q.toRotationMatrix(); }
inline quaternion  rotation_matrix_to_quaternion(matrix33 const&  R) { return quaternion(R); }
inline quaternion  angle_axis_to_quaternion(scalar const  angle, vector3 const&  axis) {
    return quaternion(Eigen::AngleAxis<scalar>(angle,axis));
}
matrix33  yaw_pitch_roll_to_rotation(scalar const  yaw, scalar const  pitch, scalar const  roll);
void  rotation_to_yaw_pitch_roll(matrix33 const&  R, scalar&  yaw, scalar&  pitch, scalar&  roll);


#   endif


inline constexpr scalar PI() noexcept { return scalar(3.14159265); }

inline scalar  absolute_value(scalar const a) { return a < scalar(0) ? -a : a; }

inline bool  are_equal(scalar const a, scalar const b, scalar const  error = scalar(1e-5)) {
    return absolute_value(a - b) <= error;
}



#endif
