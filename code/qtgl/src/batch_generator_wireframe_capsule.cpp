#include <qtgl/batch_generators.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>

namespace qtgl {


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

    return create_lines3d(vertices, colour, fog_type_, id);
}


}
