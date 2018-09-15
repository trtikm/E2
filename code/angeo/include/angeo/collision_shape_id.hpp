#ifndef ANGEO_COLLISION_SHAPE_ID_HPP_INCLUDED
#   define ANGEO_COLLISION_SHAPE_ID_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>

namespace angeo {

/**
 * Types of "primitive" collision shapes for which we provide contact point computation.
 * All these shapes represent a CONVEX set of points. It implies that POLYHEDRON shape
 * kind represents only convex polyhedra. More complex shapes (e.g. non-convex) can be
 * approximated by several primitive collision shapes properly arranged in the space.
 */
enum struct  COLLISION_SHAPE_TYPE : natural_8_bit
{
    //BOX                     = 0,
    CAPSULE                 = 1,
    //CONE                    = 2,
    //CYLINDER                = 3,
    LINE                    = 4,
    POINT                   = 5,
    //POLYHEDRON              = 6,
    SPHERE                  = 7,
    TRIANGLE                = 8,
};

inline natural_8_bit  as_number(COLLISION_SHAPE_TYPE const  cst)
{
    return static_cast<natural_8_bit>(cst);
}

inline constexpr natural_8_bit  get_max_collision_shape_type_id()
{ 
    return static_cast<natural_8_bit>(COLLISION_SHAPE_TYPE::TRIANGLE);
}


}

#endif
