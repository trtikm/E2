#ifndef ANGEO_SHAPE_HPP_INCLUDED
#   define ANGEO_SHAPE_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <unordered_map>
#   include <vector>
#   include <tuple>

namespace angeo {


/**
 * In this module we define geometric shapes in Euklidian 3D space and operations
 * over them, like intersections. The core shapes are convex ones. We have several
 *  Concave shapes
 * are emulated; they are just lists (i.e. union) of convex shapes.
 *
 */

enum struct  COLLISION_PRIMITIVE_TYPE : natural_8_bit
{
    // Convex primitives with no volume.
    POINT           = 0,
    LINE            = 1,
    TRIANGLE        = 2,
    PLANE           = 3,
    HALF_SPACE      = 4,

    // Convex primitives with volume.
    SPHERE          = 5,
    CAPSULE         = 6,
    BOX             = 7,
    CONVEX_POLYTOPE = 8,
};


enum struct  COLLISION_MATERIAL_TYPE : natural_8_bit
{
    WOOD            = 0,
    STEEL           = 1,
    CONCRETE        = 2,
    GLASS           = 3,
    GUM             = 4,
    PLASTIC         = 5,
};

struct  collision_shape
{
    struct sphere
    {

    };

    static sphere  s_spheres;
};


//enum struct convex_shape_kind : natural_8_bit
//{
//    // Extremal shapes

//    EPMTY           = 0U,   /// The empty shape has no point of the 3D space.
//    FULL            = 255U, /// The full shape has all points of the 3D space.

//    // 0D shapes

//    POINT           = 1U,   /// The origin of the 3D space; i.e. the point (0,0,0).

//    // 1D shapes

//    LINE            = 2U,   /// X-axis of the 3D space, i.e. all X=(t,0,0), -inf<t<+inf
//    RAY             = 3U,   /// Non-negative X-axis; i.e. all X=(t,0,0), 0<=t<+inf
//    INTERVAL        = 4U,   /// An interval on X-axis; i.e. all X=(t,0,0), 0<=t<=c, c>0 is a constant scalar

//    // 2D shapes (linear)

//    PLANE           = 8U,   /// XY-axis plane of the 3D space; i.e. all X=(u,v,0), -inf<u,v<+inf
//    SQUARE          = 9U,   /// A square in XY-axis plane; i.e. all X=(u,v,0), 0<=u,v<=a, a>0 is a constant scalars
//    RECTANGLE       = 10U,  /// A rectangle in XY-axis plane; i.e. all X=(u,v,0), 0<=u<=p, 0<=u<=q, p,q>0 are constant scalars
//    POLYGON         = 11U,  /// A polygon of n>2 edges in XY-axis plane; i.e. all X=(u,v,0) s.t. for all i=1..n-1:
//                            ///     -(q[i] - q[i-1])*u + (p[i] - p[i-1])*v >= 0
//                            ///     where p[i] and q[i] form xy-coordinates the i-th extremal point (p[i],q[i],0) of the polygon,
//                            ///     and p[0] = q[0] = q[1] = p[n-1] = q[n-1] = 0, p[1] > 0, q[2] > 0.

//    // 2D shapes (non-linear)

//    CIRCLE          = 16U,  /// A circle in XY-axis plane; i.e. all X=(u,v,0), u^2 + v^2 <= r^2, r>0 is a constant scalar

//    // 3D shapes (linear)

//    HALF_SPACE      = 32U,  /// All X=(u,v,w), -inf<u<=0, -inf<v,w<+inf
//    CUBE            = 33U,
//    BOX             = 34U,
//    CONE            = 35U,
//    POLYHEDRON      = 36U,

//    // 3D shapes (quadratic)

//    SPHERE          = 64U,
//    CYLINDER        = 65U,
//    CAPSULE         = 66U,
//};

//struct convex_shape_id
//{

//private:
//    static_assert(sizeof(convex_shape_id) <= sizeof(natural_64_bit), "");
//    convex_shape_kind  m_kind;
//    union
//    {
//        float_32_bit  m_value;
//        natural_32_bit  m_key;
//    };
//};


//struct shape
//{
//private:
//    std::vector< std::pair<coordinate_system_const_ptr, convex_shape_id> >  m_convex_shapes;
//};


}

#endif
