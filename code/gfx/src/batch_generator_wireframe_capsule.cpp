#include <gfx/batch_generators.hpp>
#include <gfx/detail/as_json.hpp>
#include <gfx/detail/from_json.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>

namespace gfx {


std::string  make_capsule_id_without_prefix(
        float_32_bit const  half_distance_between_end_points,
        float_32_bit const  thickness_from_central_line,
        natural_8_bit const  num_lines_per_quarter_of_circle,
        vector4 const&  colour,
        bool const  wireframe,
        FOG_TYPE const  fog_type
        )
{
    using namespace gfx::detail;
    std::stringstream  sstr;
    sstr << '{'
         << as_json("kind") << ':' << as_json(sketch_kind_capsule()) << ','
         << as_json("half_distance") << ':' << as_json(half_distance_between_end_points) << ','
         << as_json("thickness") << ':' << as_json(thickness_from_central_line) << ','
         << as_json("num_lines") << ':' << as_json(num_lines_per_quarter_of_circle) << ','
         << as_json("colour") << ':' << as_json(colour) << ','
         << as_json("fog") << ':' << as_json(name(fog_type)) << ','
         << as_json("wireframe") << ':' << as_json(wireframe)
         << '}';
    return sstr.str();
}


bool  parse_capsule_info_from_id(
        boost::property_tree::ptree const&  ptree,
        float_32_bit&  half_distance_between_end_points,
        float_32_bit&  thickness_from_central_line,
        natural_8_bit&  num_lines_per_quarter_of_circle,
        vector4&  colour,
        FOG_TYPE&  fog_type,
        bool&  wireframe
        )
{
    using namespace gfx::detail;
    if (ptree.get<std::string>("kind") != sketch_kind_capsule())
        return false;
    half_distance_between_end_points = ptree.get<float_32_bit>("half_distance");
    thickness_from_central_line = ptree.get<float_32_bit>("thickness");
    num_lines_per_quarter_of_circle = ptree.get<natural_8_bit>("num_lines");
    colour = from_json_vector4(ptree.get_child("colour"));
    fog_type = fog_type_from_name(ptree.get<std::string>("fog"));
    wireframe = ptree.get<bool>("wireframe");
    return true;
}


batch  create_wireframe_capsule(
        float_32_bit const  half_distance_between_end_points,
        float_32_bit const  thickness_from_central_line,
        natural_8_bit const  num_lines_per_quarter_of_circle,
        vector4 const&  colour,
        FOG_TYPE const  fog_type_,
        std::string const&  id
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(half_distance_between_end_points > 1e-4f);
    ASSUMPTION(thickness_from_central_line > 1e-4f);
    ASSUMPTION(num_lines_per_quarter_of_circle != 0U);

    std::vector< std::array<float_32_bit,3> >  vertices;
    vertices.reserve(2U * ((3U + 2U) * 4U * static_cast<natural_16_bit>(num_lines_per_quarter_of_circle) + 4U));

    float_32_bit const  delta_phi = (PI() / 2.0f) / static_cast<float_32_bit>(num_lines_per_quarter_of_circle);
    vertices.push_back({ thickness_from_central_line, 0.0f, 0.0f });
    for (natural_16_bit  i = 1U; i != num_lines_per_quarter_of_circle; ++i)
    {
        float_32_bit const  phi = static_cast<float_32_bit>(i) * delta_phi;
        vertices.push_back({ thickness_from_central_line * std::cosf(phi), thickness_from_central_line * std::sinf(phi), 0.0f });
        vertices.push_back(vertices.back());
    }
    vertices.push_back({ 0.0f, thickness_from_central_line, 0.0f });

    std::size_t const  num_vertices_per_quarter_ring = vertices.size();

    for (std::size_t  i = 0U, n = vertices.size(); i != n; ++i)
    {
        std::array<float_32_bit,3> const&  v = vertices.at(i);
        vertices.push_back({ -v[0], v[1], 0.0f });
    }

    std::size_t const  num_vertices_per_half_ring = vertices.size();

    for (std::size_t  i = 0U, n = vertices.size(); i != n; ++i)
    {
        std::array<float_32_bit,3> const&  v =  vertices.at(i);
        vertices.push_back({ v[0], -v[1], 0.0f });
    }

    std::size_t const  num_vertices_per_ring = vertices.size();
    INVARIANT(num_vertices_per_ring == 2UL * 4UL * num_lines_per_quarter_of_circle);

    for (std::size_t i = 0U; i != num_vertices_per_ring; ++i)
    {
        std::array<float_32_bit, 3> const&  v = vertices.at(i);
        vertices.push_back({ v[0], v[1], half_distance_between_end_points });
    }

    for (std::size_t i = 0U; i != num_vertices_per_ring; ++i)
    {
        std::array<float_32_bit, 3> const&  v = vertices.at(i);
        vertices.push_back({ v[0], v[1], -half_distance_between_end_points });
    }

    {
        std::array<float_32_bit, 3> const&  v = vertices.at(0U);
        vertices.push_back({  v[0], v[1],  half_distance_between_end_points });
        vertices.push_back({  v[0], v[1], -half_distance_between_end_points });
        vertices.push_back({ -v[0], v[1],  half_distance_between_end_points });
        vertices.push_back({ -v[0], v[1], -half_distance_between_end_points });
    }

    {
        std::array<float_32_bit, 3> const&  v = vertices.at(num_vertices_per_quarter_ring - 1UL);
        vertices.push_back({ v[0],  v[1],  half_distance_between_end_points });
        vertices.push_back({ v[0],  v[1], -half_distance_between_end_points });
        vertices.push_back({ v[0], -v[1],  half_distance_between_end_points });
        vertices.push_back({ v[0], -v[1], -half_distance_between_end_points });
    }

    for (std::size_t  i = 0U; i != num_vertices_per_half_ring; ++i)
    {
        std::array<float_32_bit,3> const&  v = vertices.at(i);
        vertices.push_back({ v[0], 0.0f, v[1] + half_distance_between_end_points });
    }

    for (std::size_t  i = 0U; i != num_vertices_per_half_ring; ++i)
    {
        std::array<float_32_bit, 3> const&  v = vertices.at(i);
        vertices.push_back({ v[0], 0.0f, -v[1] - half_distance_between_end_points });
    }

    for (std::size_t  i = 0U; i != num_vertices_per_half_ring; ++i)
    {
        std::array<float_32_bit,3> const&  v = vertices.at(i);
        vertices.push_back({ 0.0f, v[0], v[1] + half_distance_between_end_points });
    }

    for (std::size_t  i = 0U; i != num_vertices_per_half_ring; ++i)
    {
        std::array<float_32_bit,3> const&  v = vertices.at(i);
        vertices.push_back({ 0.0f, v[0], -v[1] - half_distance_between_end_points });
    }

    return create_lines3d(
                vertices,
                colour,
                fog_type_,
                id.empty() ? make_capsule_id_without_prefix(
                                    half_distance_between_end_points,
                                    thickness_from_central_line,
                                    num_lines_per_quarter_of_circle,
                                    colour,
                                    true,
                                    fog_type_
                                    )
                           : id
                );
}


batch  create_solid_capsule(
        float_32_bit const  half_distance_between_end_points,
        float_32_bit const  thickness_from_central_line,
        natural_8_bit const  num_lines_per_quarter_of_circle,
        vector4 const&  colour,
        FOG_TYPE const  fog_type_,
        std::string const&  id
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(half_distance_between_end_points > 1e-4f);
    ASSUMPTION(thickness_from_central_line > 1e-4f);
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
    for (natural_32_bit i = 0U; i < num_horisontal_steps; ++i)
    {
        float_32_bit const cFI1 = std::cosf(i * delta_phi);
        float_32_bit const sFI1 = std::sinf(i * delta_phi);

        float_32_bit const cFI2 = std::cosf((i + 1U) * delta_phi);
        float_32_bit const sFI2 = std::sinf((i + 1U) * delta_phi);

        for (natural_8_bit j = 0U; j < num_vertical_steps; ++j)
        {
            float_32_bit const cPSI1 = std::cosf(j * delta_phi - PI() / 2.0f);
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
            vector3 const  shift_z =
                    (j < num_lines_per_quarter_of_circle ? -1.0f : 1.0f) * half_distance_between_end_points * vector3_unit_z();
            vector3 const  v[4] = {
                thickness_from_central_line * w[0] + shift_z,
                thickness_from_central_line * w[1] + shift_z,
                thickness_from_central_line * w[2] + shift_z,
                thickness_from_central_line * w[3] + shift_z
            };

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
    for (natural_32_bit i = 0U; i < num_horisontal_steps; ++i)
    {
        float_32_bit const cFI1 = std::cosf(i * delta_phi);
        float_32_bit const sFI1 = std::sinf(i * delta_phi);

        float_32_bit const cFI2 = std::cosf((i + 1U) * delta_phi);
        float_32_bit const sFI2 = std::sinf((i + 1U) * delta_phi);

        vector3 const  w[2] = { { cFI1, sFI1, 0.0f }, { cFI2, sFI2, 0.0f } };
        vector3 const  n = normalised(w[0] + w[1]);
        vector3 const  v[4] = {
            thickness_from_central_line * w[0] - half_distance_between_end_points * vector3_unit_z(),
            thickness_from_central_line * w[0] + half_distance_between_end_points * vector3_unit_z(),
            thickness_from_central_line * w[1] - half_distance_between_end_points * vector3_unit_z(),
            thickness_from_central_line * w[1] + half_distance_between_end_points * vector3_unit_z(),
        };

        push_back_triangle(v[0], v[2], v[1], n);
        push_back_triangle(v[2], v[3], v[1], n);
    }

    return create_triangle_mesh(
                vertices,
                normals,
                colour,
                fog_type_,
                id.empty() ? make_capsule_id_without_prefix(
                                    half_distance_between_end_points,
                                    thickness_from_central_line,
                                    num_lines_per_quarter_of_circle,
                                    colour,
                                    false,
                                    fog_type_
                                    )
                           : id
                );
}

}
