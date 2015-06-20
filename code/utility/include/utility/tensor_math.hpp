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


#   endif


#endif
