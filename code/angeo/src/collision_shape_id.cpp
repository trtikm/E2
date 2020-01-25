#include <angeo/collision_shape_id.hpp>
#include <utility/invariants.hpp>

namespace angeo {


char const*  as_string(COLLISION_SHAPE_TYPE const  cst)
{
    switch (cst)
    {
    case COLLISION_SHAPE_TYPE::BOX: return "box";
    case COLLISION_SHAPE_TYPE::CAPSULE: return "capsule";
    //case COLLISION_SHAPE_TYPE::CONE: return "cone";
    //case COLLISION_SHAPE_TYPE::CYLINDER: return "cylinder";
    case COLLISION_SHAPE_TYPE::LINE: return "line";
    case COLLISION_SHAPE_TYPE::POINT: return "point";
    //case COLLISION_SHAPE_TYPE::POLYHEDRON: return "polyhedron";
    case COLLISION_SHAPE_TYPE::SPHERE: return "sphere";
    case COLLISION_SHAPE_TYPE::TRIANGLE: return "triangle";
    default: UNREACHABLE();
    }
}


}
