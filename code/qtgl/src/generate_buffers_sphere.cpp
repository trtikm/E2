#include <qtgl/buffer_generators.hpp>
#include <utility/msgstream.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>

namespace qtgl {


void  create_wireframe_sphere_vertex_buffer(
        float_32_bit const  radius,
        natural_8_bit const  num_lines_per_quarter_of_circle,
        buffer_ptr&  output_vertex_buffer,
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

    output_vertex_buffer = buffer::create(vertices,msgstream() << id << (id.empty() ? "" : "/") << "sphere/vertices",true);
}



}
