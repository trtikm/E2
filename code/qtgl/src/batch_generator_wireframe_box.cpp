#include <qtgl/batch_generators.hpp>
#include <utility/timeprof.hpp>
#include <sstream>

namespace qtgl {


batch  create_wireframe_box( // Deprecated!
        vector3 const&  lo_corner,
        vector3 const&  hi_corner,
        vector4 const&  colour,
        FOG_TYPE const  fog_type_,
        std::string const&  id
        )
{
    TMPROF_BLOCK();

    std::vector< std::array<float_32_bit,3> > const  vertices {
        { lo_corner(0), lo_corner(1), lo_corner(2) }, { hi_corner(0), lo_corner(1), lo_corner(2) },
        { hi_corner(0), lo_corner(1), lo_corner(2) }, { hi_corner(0), hi_corner(1), lo_corner(2) },
        { hi_corner(0), hi_corner(1), lo_corner(2) }, { lo_corner(0), hi_corner(1), lo_corner(2) },
        { lo_corner(0), hi_corner(1), lo_corner(2) }, { lo_corner(0), lo_corner(1), lo_corner(2) },

        { lo_corner(0), lo_corner(1), hi_corner(2) }, { hi_corner(0), lo_corner(1), hi_corner(2) },
        { hi_corner(0), lo_corner(1), hi_corner(2) }, { hi_corner(0), hi_corner(1), hi_corner(2) },
        { hi_corner(0), hi_corner(1), hi_corner(2) }, { lo_corner(0), hi_corner(1), hi_corner(2) },
        { lo_corner(0), hi_corner(1), hi_corner(2) }, { lo_corner(0), lo_corner(1), hi_corner(2) },

        { lo_corner(0), lo_corner(1), lo_corner(2) }, { lo_corner(0), lo_corner(1), hi_corner(2) },
        { hi_corner(0), lo_corner(1), lo_corner(2) }, { hi_corner(0), lo_corner(1), hi_corner(2) },
        { hi_corner(0), hi_corner(1), lo_corner(2) }, { hi_corner(0), hi_corner(1), hi_corner(2) },
        { lo_corner(0), hi_corner(1), lo_corner(2) }, { lo_corner(0), hi_corner(1), hi_corner(2) },
    };

    return create_lines3d(vertices, colour, fog_type_, id);
}


static std::string  make_box_unique_id(
        vector3 const&  half_sizes_along_axes,
        vector4 const&  colour,
        FOG_TYPE const  fog_type_,
        bool const  wireframe
        )
{
    std::stringstream  sstr;
    sstr << "/generic/box/" << (wireframe ? "wireframe" : "solid") << "/"
         << "sizes[" << half_sizes_along_axes(0) << "," << half_sizes_along_axes(1) << "," << half_sizes_along_axes(2) << "],"
         << "colour[" << colour(0) << "," << colour(1) << "," << colour(2) << "," << colour(3) << "],"
         << "fog[" << name(fog_type_) << "]";
    return sstr.str();
}


batch  create_wireframe_box(
        vector3 const&  half_sizes_along_axes,
        vector4 const&  colour,
        FOG_TYPE const  fog_type_,
        std::string const&  id
        )
{
    TMPROF_BLOCK();

    std::vector< std::array<float_32_bit,3> > const  vertices {
        { -half_sizes_along_axes(0), -half_sizes_along_axes(1), -half_sizes_along_axes(2) }, {  half_sizes_along_axes(0), -half_sizes_along_axes(1), -half_sizes_along_axes(2) },
        {  half_sizes_along_axes(0), -half_sizes_along_axes(1), -half_sizes_along_axes(2) }, {  half_sizes_along_axes(0),  half_sizes_along_axes(1), -half_sizes_along_axes(2) },
        {  half_sizes_along_axes(0),  half_sizes_along_axes(1), -half_sizes_along_axes(2) }, { -half_sizes_along_axes(0),  half_sizes_along_axes(1), -half_sizes_along_axes(2) },
        { -half_sizes_along_axes(0),  half_sizes_along_axes(1), -half_sizes_along_axes(2) }, { -half_sizes_along_axes(0), -half_sizes_along_axes(1), -half_sizes_along_axes(2) },

        { -half_sizes_along_axes(0), -half_sizes_along_axes(1),  half_sizes_along_axes(2) }, {  half_sizes_along_axes(0), -half_sizes_along_axes(1),  half_sizes_along_axes(2) },
        {  half_sizes_along_axes(0), -half_sizes_along_axes(1),  half_sizes_along_axes(2) }, {  half_sizes_along_axes(0),  half_sizes_along_axes(1),  half_sizes_along_axes(2) },
        {  half_sizes_along_axes(0),  half_sizes_along_axes(1),  half_sizes_along_axes(2) }, { -half_sizes_along_axes(0),  half_sizes_along_axes(1),  half_sizes_along_axes(2) },
        { -half_sizes_along_axes(0),  half_sizes_along_axes(1),  half_sizes_along_axes(2) }, { -half_sizes_along_axes(0), -half_sizes_along_axes(1),  half_sizes_along_axes(2) },

        { -half_sizes_along_axes(0), -half_sizes_along_axes(1), -half_sizes_along_axes(2) }, { -half_sizes_along_axes(0), -half_sizes_along_axes(1),  half_sizes_along_axes(2) },
        {  half_sizes_along_axes(0), -half_sizes_along_axes(1), -half_sizes_along_axes(2) }, {  half_sizes_along_axes(0), -half_sizes_along_axes(1),  half_sizes_along_axes(2) },
        {  half_sizes_along_axes(0),  half_sizes_along_axes(1), -half_sizes_along_axes(2) }, {  half_sizes_along_axes(0),  half_sizes_along_axes(1),  half_sizes_along_axes(2) },
        { -half_sizes_along_axes(0),  half_sizes_along_axes(1), -half_sizes_along_axes(2) }, { -half_sizes_along_axes(0),  half_sizes_along_axes(1),  half_sizes_along_axes(2) },
    };

    return create_lines3d(vertices, colour, fog_type_, id.empty() ? make_box_unique_id(half_sizes_along_axes, colour, fog_type_, true) : id);
}


batch  create_solid_box(
        vector3 const&  half_sizes_along_axes,
        vector4 const&  colour,
        FOG_TYPE const  fog_type_,
        std::string const&  id
        )
{
    TMPROF_BLOCK();

    std::vector< std::array<float_32_bit,3> > const  vertices {
        { -half_sizes_along_axes(0), -half_sizes_along_axes(1), -half_sizes_along_axes(2) },
        {  half_sizes_along_axes(0), -half_sizes_along_axes(1), -half_sizes_along_axes(2) },
        {  half_sizes_along_axes(0),  half_sizes_along_axes(1), -half_sizes_along_axes(2) },
        { -half_sizes_along_axes(0),  half_sizes_along_axes(1), -half_sizes_along_axes(2) },

        { -half_sizes_along_axes(0), -half_sizes_along_axes(1),  half_sizes_along_axes(2) },
        {  half_sizes_along_axes(0), -half_sizes_along_axes(1),  half_sizes_along_axes(2) },
        {  half_sizes_along_axes(0),  half_sizes_along_axes(1),  half_sizes_along_axes(2) },
        { -half_sizes_along_axes(0),  half_sizes_along_axes(1),  half_sizes_along_axes(2) },
    };
    std::vector< std::array<natural_32_bit, 3> > const  indices {
        { 0U, 3U, 2U },  { 0U, 2U, 1U },
        { 4U, 5U, 6U },  { 4U, 6U, 7U },
        { 0U, 1U, 5U },  { 0U, 5U, 4U },
        { 2U, 3U, 7U },  { 2U, 7U, 6U },
        { 0U, 4U, 7U },  { 0U, 7U, 3U },
        { 1U, 2U, 6U },  { 1U, 6U, 5U },
    };

    return create_triangle_mesh(vertices, indices, colour, fog_type_, id.empty() ? make_box_unique_id(half_sizes_along_axes, colour, fog_type_, false) : id);
}


}
