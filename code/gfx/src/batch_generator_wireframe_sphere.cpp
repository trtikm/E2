#include <gfx/batch_generators.hpp>
#include <gfx/detail/as_json.hpp>
#include <gfx/detail/from_json.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>

namespace gfx {


std::string  make_sphere_id_without_prefix(
        float_32_bit const  radius,
        natural_8_bit const  num_lines_per_quarter_of_circle,
        vector4 const&  colour,
        FOG_TYPE const  fog_type,
        bool const  wireframe
        )
{
    using namespace gfx::detail;
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
    using namespace gfx::detail;
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


batch  create_solid_smooth_sphere(
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

    natural_16_bit const  num_vertices_in_horisontal_slice = num_lines_per_quarter_of_circle * 4U;
    natural_16_bit const  num_horisontal_slices = 2U * (num_lines_per_quarter_of_circle - 1U) + 1U;

    natural_16_bit const  num_vertices = num_horisontal_slices * num_vertices_in_horisontal_slice + 2U;

    natural_16_bit const  num_indices = 2U * (num_horisontal_slices - 1U) * num_vertices_in_horisontal_slice +
                                        2U * num_vertices_in_horisontal_slice;

    std::vector< std::array<float_32_bit, 3> >  vertices;
    std::vector< std::array<float_32_bit, 3> >  normals;
    {
        vertices.reserve(num_vertices);
        normals.reserve(num_vertices);
        float_32_bit const  delta_phi = (PI() / 2.0f) / static_cast<float_32_bit>(num_lines_per_quarter_of_circle);
        for (integer_32_bit  i = -(integer_32_bit)(num_lines_per_quarter_of_circle - 1U); i < num_lines_per_quarter_of_circle; ++i)
        {
            float_32_bit const  psi = static_cast<float_32_bit>(i)* delta_phi;
            float_32_bit const  cos_psi = std::cosf(psi);
            float_32_bit const  sin_psi = std::sinf(psi);
            for (integer_32_bit  j = 0U; j < num_vertices_in_horisontal_slice; ++j)
            {
                float_32_bit const  phi = static_cast<float_32_bit>(j)* delta_phi;
                float_32_bit const  cos_phi = std::cosf(phi);
                float_32_bit const  sin_phi = std::sinf(phi);
                vector3 const  v( cos_phi * cos_psi, sin_phi * cos_psi, sin_psi);                
                vertices.push_back({ radius * v(0), radius * v(1), radius * v(2) });
                normals.push_back({ v(0), v(1), v(2) });
            }
        }
        vertices.push_back({ 0.0f, 0.0f, -radius });
        normals.push_back({ 0.0f, 0.0f, -1.0f });
        vertices.push_back({ 0.0f, 0.0f, radius });
        normals.push_back({ 0.0f, 0.0f, 1.0f });
        INVARIANT(vertices.size() == num_vertices && normals.size() == num_vertices);
    }

    std::vector< std::array<natural_32_bit, 3> >  indices;
    {
        indices.reserve(num_indices);
        for (natural_16_bit i = num_vertices_in_horisontal_slice - 1U, j = 0U; j < num_vertices_in_horisontal_slice; i = j, ++j)
            indices.push_back({ j, i, num_vertices - 2U });
        for (natural_16_bit k = 0U; k + 1U < num_horisontal_slices; ++k)
            for (natural_16_bit  i = num_vertices_in_horisontal_slice - 1U, j = 0U; j < num_vertices_in_horisontal_slice; i = j, ++j)
            {
                natural_32_bit const  lo_shift = k * num_vertices_in_horisontal_slice;
                natural_32_bit const  hi_shift = lo_shift + num_vertices_in_horisontal_slice;
                indices.push_back({ lo_shift + i, lo_shift + j, hi_shift + j });
                indices.push_back({ lo_shift + i, hi_shift + j, hi_shift + i });
            }
        natural_32_bit const  shift = num_vertices - 2U - num_vertices_in_horisontal_slice;
        for (natural_16_bit i = num_vertices_in_horisontal_slice - 1U, j = 0U; j < num_vertices_in_horisontal_slice; i = j, ++j)
            indices.push_back({ shift + i, shift + j, num_vertices - 1U });
        INVARIANT(indices.size() == num_indices);
    }

    return create_triangle_mesh(
                vertices,
                indices,
                normals,
                colour,
                fog_type_,
                id.empty() ? make_sphere_id_without_prefix(radius, num_lines_per_quarter_of_circle, colour, fog_type_, false) : id
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

    std::vector< std::array<float_32_bit, 3> >  vertices;
    std::vector< std::array<float_32_bit, 3> >  normals;

    auto const  push_back_vector = [](vector3 const&  what, std::vector< std::array<float_32_bit, 3> >&  where) -> void {
        where.push_back({ what(0), what(1), what(2) });
    };

    auto const  push_back_triangle = [&vertices, &normals, &push_back_vector](
        vector3 const&  A, vector3 const&  B, vector3 const&  C, vector3 const&  n) -> void {
        push_back_vector(A, vertices); push_back_vector(n, normals);
        push_back_vector(B, vertices); push_back_vector(n, normals);
        push_back_vector(C, vertices); push_back_vector(n, normals);
    };

    float_32_bit const  delta_phi = (PI() / 2.0f) / static_cast<float_32_bit>(num_lines_per_quarter_of_circle);
    natural_32_bit const  num_horisontal_steps = 4U * num_lines_per_quarter_of_circle;
    natural_32_bit const  num_vertical_steps = 2U * num_lines_per_quarter_of_circle;
    for (natural_32_bit  i = 0U; i < num_horisontal_steps; ++i)
    {
        float_32_bit const cFI1 = std::cosf(i * delta_phi);
        float_32_bit const sFI1 = std::sinf(i * delta_phi);

        float_32_bit const cFI2 = std::cosf((i + 1U) * delta_phi);
        float_32_bit const sFI2 = std::sinf((i + 1U) * delta_phi);

        for (natural_8_bit  j = 0U; j < num_vertical_steps; ++j)
        {
            float_32_bit const cPSI1 = std::cosf(j * delta_phi - PI()/2.0f);
            float_32_bit const sPSI1 = std::sinf(j * delta_phi - PI() / 2.0f);

            float_32_bit const cPSI2 = std::cosf((j + 1U) * delta_phi - PI() / 2.0f);
            float_32_bit const sPSI2 = std::sinf((j + 1U) * delta_phi - PI() / 2.0f);

            vector3 const  w[4] = {
                { cFI1 * cPSI1, sFI1 * cPSI1, sPSI1 },
                { cFI1 * cPSI2, sFI1 * cPSI2, sPSI2 },
                { cFI2 * cPSI1, sFI2 * cPSI1, sPSI1 },
                { cFI2 * cPSI2, sFI2 * cPSI2, sPSI2 },
            };
            vector3 const  n = normalised(w[0] + w[1] + w[2] + w[3]);
            vector3 const  v[4] = { radius * w[0], radius * w[1], radius * w[2], radius * w[3] };

            if (j == 0U)
                push_back_triangle(v[2], v[3], v[1], n);
            else if (j == num_vertical_steps - 1U)
                push_back_triangle(v[0], v[2], v[1], n);
            else
            {
                push_back_triangle(v[0], v[2], v[1], n);
                push_back_triangle(v[2], v[3], v[1], n);
            }
        }
    }

    return create_triangle_mesh(
                vertices,
                normals,
                colour,
                fog_type_,
                id.empty() ? make_sphere_id_without_prefix(radius, num_lines_per_quarter_of_circle, colour, fog_type_, false) : id
                );
}


}
