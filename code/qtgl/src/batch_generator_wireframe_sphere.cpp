#include <qtgl/batch_generators.hpp>
#include <qtgl/detail/as_json.hpp>
#include <qtgl/detail/from_json.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>

namespace qtgl {


std::string  make_sphere_id_without_prefix(
        float_32_bit const  radius,
        natural_8_bit const  num_lines_per_quarter_of_circle,
        vector4 const&  colour,
        FOG_TYPE const  fog_type,
        bool const  wireframe
        )
{
    using namespace qtgl::detail;
    std::stringstream  sstr;
    sstr << '{'
         << as_json("kind") << ':' << as_json(sketch_kind_sphere()) << ','
         << as_json("radius") << ':' << as_json(radius) << ','
         << as_json("num_lines") << ':' << as_json(num_lines_per_quarter_of_circle) << ','
         << as_json("colour") << ':' << as_json(colour) << ','
         << as_json("fog") << ':' << as_json(name(fog_type)) << ','
         << as_json("wireframe") << ':' << as_json(wireframe)
         << '}';
    return sstr.str();
}


bool  parse_sphere_info_from_id(
        boost::property_tree::ptree const&  ptree,
        float_32_bit&  radius,
        natural_8_bit&  num_lines_per_quarter_of_circle,
        vector4&  colour,
        FOG_TYPE&  fog_type,
        bool&  wireframe
        )
{
    using namespace qtgl::detail;
    if (ptree.get<std::string>("kind") != sketch_kind_sphere())
        return false;
    radius = ptree.get<float_32_bit>("radius");
    num_lines_per_quarter_of_circle = ptree.get<natural_8_bit>("num_lines");
    colour = from_json_vector4(ptree.get_child("colour"));
    fog_type = fog_type_from_name(ptree.get<std::string>("fog"));
    wireframe = ptree.get<bool>("wireframe");
    return true;
}


batch  create_wireframe_sphere(
        float_32_bit const  radius,
        natural_8_bit const  num_lines_per_quarter_of_circle,
        vector4 const&  colour,
        FOG_TYPE const  fog_type_,
        std::string const&  id
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(radius > 1e-4f);
    ASSUMPTION(num_lines_per_quarter_of_circle != 0U);

    std::vector< std::array<float_32_bit,3> >  vertices;
    vertices.reserve(2U * 3U * 4U * static_cast<natural_16_bit>(num_lines_per_quarter_of_circle));

    float_32_bit const  delta_phi = (PI() / 2.0f) / static_cast<float_32_bit>(num_lines_per_quarter_of_circle);
    vertices.push_back({ radius, 0.0f, 0.0f });
    for (natural_16_bit  i = 1U; i != num_lines_per_quarter_of_circle; ++i)
    {
        float_32_bit const  phi = static_cast<float_32_bit>(i) * delta_phi;
        vertices.push_back({ radius * std::cosf(phi), radius * std::sinf(phi), 0.0f });
        vertices.push_back(vertices.back());
    }
    vertices.push_back({ 0.0f, radius, 0.0f });

    for (std::size_t  i = 0U, n = vertices.size(); i != n; ++i)
    {
        std::array<float_32_bit,3> const&  v = vertices.at(i);
        vertices.push_back({ -v[0], v[1], 0.0f });
    }
    for (std::size_t  i = 0U, n = vertices.size(); i != n; ++i)
    {
        std::array<float_32_bit,3> const&  v =  vertices.at(i);
        vertices.push_back({ v[0], -v[1], 0.0f });
    }

    std::size_t const  num_vertices_per_ring = vertices.size();
    INVARIANT(num_vertices_per_ring == 2UL * 4UL * num_lines_per_quarter_of_circle);

    for (std::size_t  i = 0U; i != num_vertices_per_ring; ++i)
    {
        std::array<float_32_bit,3> const&  v =  vertices.at(i);
        vertices.push_back({ v[0], 0.0f, v[1] });
    }

    for (std::size_t  i = 0U; i != num_vertices_per_ring; ++i)
    {
        std::array<float_32_bit,3> const&  v =  vertices.at(i);
        vertices.push_back({ 0.0f, v[0], v[1] });
    }

    return create_lines3d(
                vertices,
                colour,
                fog_type_,
                id.empty() ? make_sphere_id_without_prefix(
                                    radius,
                                    num_lines_per_quarter_of_circle,
                                    colour,
                                    fog_type_,
                                    true
                                    )
                           : id
                );
}


batch  create_solid_sphere(
        float_32_bit const  radius,
        natural_8_bit const  num_lines_per_quarter_of_circle,
        vector4 const&  colour,
        FOG_TYPE const  fog_type_,
        std::string const&  id
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(radius > 1e-4f);
    ASSUMPTION(num_lines_per_quarter_of_circle != 0U);

    NOT_IMPLEMENTED_YET();
}


}
