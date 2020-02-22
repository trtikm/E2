#ifndef ANGEO_COLLISION_SHAPE_ID_HPP_INCLUDED
#   define ANGEO_COLLISION_SHAPE_ID_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <string>

namespace angeo {

/**
 * Types of "primitive" collision shapes for which we provide contact point computation.
 * All these shapes represent a CONVEX set of points. It implies that POLYHEDRON shape
 * kind represents only convex polyhedra. More complex shapes (e.g. non-convex) can be
 * approximated by several primitive collision shapes properly arranged in the space.
 */
enum struct  COLLISION_SHAPE_TYPE : natural_8_bit
{
    BOX,
    CAPSULE,
    //CONE,
    //CYLINDER,
    LINE,
    POINT,
    //POLYHEDRON,
    SPHERE,
    TRIANGLE,
};

inline natural_8_bit  as_number(COLLISION_SHAPE_TYPE const  cst)
{
    return static_cast<natural_8_bit>(cst);
}

inline COLLISION_SHAPE_TYPE  as_collision_shape_type(natural_8_bit const  index)
{
    return (COLLISION_SHAPE_TYPE)index;
}

std::string const&  description(COLLISION_SHAPE_TYPE const  cst);
std::string const&  as_string(COLLISION_SHAPE_TYPE const  cst);
COLLISION_SHAPE_TYPE  as_collision_shape_type(std::string const&  name);

inline constexpr natural_8_bit  get_max_collision_shape_type_id()
{ 
    return static_cast<natural_8_bit>(COLLISION_SHAPE_TYPE::TRIANGLE);
}


}

#endif
