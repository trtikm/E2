#include <gfx/batch_generators.hpp>
#include <gfx/detail/as_json.hpp>
#include <gfx/detail/from_json.hpp>
#include <utility/timeprof.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace gfx {


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


std::string  make_box_id_without_prefix(
        vector3 const&  half_sizes_along_axes,
        vector4 const&  colour,
        FOG_TYPE const  fog_type,
        bool const  wireframe
        )
{
    using namespace gfx::detail;
    std::stringstream  sstr;
    sstr << '{'
         << as_json("kind") << ':' << as_json(sketch_kind_box()) << ','
         << as_json("half_sizes") << ':' << as_json(half_sizes_along_axes) << ','
         << as_json("colour") << ':' << as_json(colour) << ','
         << as_json("fog") << ':' << as_json(name(fog_type)) << ','
         << as_json("wireframe") << ':' << as_json(wireframe)
         << '}';
    return sstr.str();
}


bool  parse_box_info_from_id(
        boost::property_tree::ptree const&  ptree,
        vector3&  half_sizes_along_axes,
        vector4&  colour,
        FOG_TYPE&  fog_type,
        bool&  wireframe
        )
{
    using namespace gfx::detail;
    if (ptree.get<std::string>("kind") != sketch_kind_box())
        return false;
    half_sizes_along_axes = from_json_vector3(ptree.get_child("half_sizes"));
    colour = from_json_vector4(ptree.get_child("colour"));
    fog_type = fog_type_from_name(ptree.get<std::string>("fog"));
    wireframe = ptree.get<bool>("wireframe");
    return true;
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

    return create_lines3d(
                vertices,
                colour,
                fog_type_, 
                id.empty() ? make_box_id_without_prefix(half_sizes_along_axes, colour, fog_type_, true) : id
                );
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
        // face xy, z-min
        { -half_sizes_along_axes(0), -half_sizes_along_axes(1), -half_sizes_along_axes(2) },    // 0
        {  half_sizes_along_axes(0), -half_sizes_along_axes(1), -half_sizes_along_axes(2) },
        {  half_sizes_along_axes(0),  half_sizes_along_axes(1), -half_sizes_along_axes(2) },
        { -half_sizes_along_axes(0),  half_sizes_along_axes(1), -half_sizes_along_axes(2) },

        // face xy, z-max
        { -half_sizes_along_axes(0), -half_sizes_along_axes(1),  half_sizes_along_axes(2) },    // 4
        {  half_sizes_along_axes(0), -half_sizes_along_axes(1),  half_sizes_along_axes(2) },
        {  half_sizes_along_axes(0),  half_sizes_along_axes(1),  half_sizes_along_axes(2) },
        { -half_sizes_along_axes(0),  half_sizes_along_axes(1),  half_sizes_along_axes(2) },

        // face xz, y-min
        { -half_sizes_along_axes(0), -half_sizes_along_axes(1), -half_sizes_along_axes(2) },    // 8
        {  half_sizes_along_axes(0), -half_sizes_along_axes(1), -half_sizes_along_axes(2) },
        {  half_sizes_along_axes(0), -half_sizes_along_axes(1),  half_sizes_along_axes(2) },
        { -half_sizes_along_axes(0), -half_sizes_along_axes(1),  half_sizes_along_axes(2) },

        // face xz, y-max
        { -half_sizes_along_axes(0),  half_sizes_along_axes(1), -half_sizes_along_axes(2) },    // 12
        {  half_sizes_along_axes(0),  half_sizes_along_axes(1), -half_sizes_along_axes(2) },
        {  half_sizes_along_axes(0),  half_sizes_along_axes(1),  half_sizes_along_axes(2) },
        { -half_sizes_along_axes(0),  half_sizes_along_axes(1),  half_sizes_along_axes(2) },

        // face yz, x-min
        { -half_sizes_along_axes(0), -half_sizes_along_axes(1), -half_sizes_along_axes(2) },    // 16
        { -half_sizes_along_axes(0),  half_sizes_along_axes(1), -half_sizes_along_axes(2) },
        { -half_sizes_along_axes(0),  half_sizes_along_axes(1),  half_sizes_along_axes(2) },
        { -half_sizes_along_axes(0), -half_sizes_along_axes(1),  half_sizes_along_axes(2) },

        // face yz, x-max
        {  half_sizes_along_axes(0), -half_sizes_along_axes(1), -half_sizes_along_axes(2) },    // 20
        {  half_sizes_along_axes(0),  half_sizes_along_axes(1), -half_sizes_along_axes(2) },
        {  half_sizes_along_axes(0),  half_sizes_along_axes(1),  half_sizes_along_axes(2) },
        {  half_sizes_along_axes(0), -half_sizes_along_axes(1),  half_sizes_along_axes(2) },
    };
    static std::vector< std::array<float_32_bit, 3> > const  normals {
        // face xy, z-min
        {  0.0f,  0.0f, -1.0f, },   // 0
        {  0.0f,  0.0f, -1.0f, },
        {  0.0f,  0.0f, -1.0f, },
        {  0.0f,  0.0f, -1.0f, },

        // face xy, z-max
        {  0.0f,  0.0f,  1.0f, },   // 4
        {  0.0f,  0.0f,  1.0f, },
        {  0.0f,  0.0f,  1.0f, },
        {  0.0f,  0.0f,  1.0f, },

        // face xz, y-min
        {  0.0f, -1.0f,  0.0f, },   // 8
        {  0.0f, -1.0f,  0.0f, },
        {  0.0f, -1.0f,  0.0f, },
        {  0.0f, -1.0f,  0.0f, },

        // face xz, y-max
        {  0.0f,  1.0f,  0.0f, },   // 12
        {  0.0f,  1.0f,  0.0f, },
        {  0.0f,  1.0f,  0.0f, },
        {  0.0f,  1.0f,  0.0f, },

        // face yz, x-min
        { -1.0f,  0.0f,  0.0f, },   // 16
        { -1.0f,  0.0f,  0.0f, },
        { -1.0f,  0.0f,  0.0f, },
        { -1.0f,  0.0f,  0.0f, },

        // face yz, x-max
        {  1.0f,  0.0f,  0.0f, },   // 20
        {  1.0f,  0.0f,  0.0f, },
        {  1.0f,  0.0f,  0.0f, },
        {  1.0f,  0.0f,  0.0f, },
    };
    static std::vector< std::array<natural_32_bit, 3> > const  indices {
        // 0 - face xy, z-min
        { 0U, 3U, 2U },  { 0U, 2U, 1U },
        // 4 - face xy, z-max
        { 4U, 5U, 6U },  { 4U, 6U, 7U },
        // 8 - face xz, y-min
        { 8U, 9U, 10U },  { 8U, 10U, 11U },
        // 12 - face xz, y-max
        { 12U, 15U, 14U },  { 12U, 14U, 13U },
        // 16 - face yz, x-min
        { 16U, 19U, 18U },  { 16U, 18U, 17U },
        // 20 - face yz, x-max
        { 20U, 21U, 22U },  { 20U, 22U, 23U },
    };

    return create_triangle_mesh(
                vertices,
                indices,
                normals,
                colour,
                fog_type_,
                id.empty() ? make_box_id_without_prefix(half_sizes_along_axes, colour, fog_type_, false) : id
                );
}


}
