#include <qtgl/buffer_generators.hpp>
#include <utility/assumptions.hpp>

namespace qtgl {


void  create_grid_vertex_and_colour_buffers(
            float_32_bit const  max_x_coordinate,
            float_32_bit const  max_y_coordinate,
            float_32_bit const  max_z_coordinate,
            float_32_bit const  step_along_x_axis,
            float_32_bit const  step_along_y_axis,
            std::array<float_32_bit,3> const&  colour_for_x_lines,
            std::array<float_32_bit,3> const&  colour_for_y_lines,
            std::array<float_32_bit,3> const&  colour_for_highlighted_x_lines,
            std::array<float_32_bit,3> const&  colour_for_highlighted_y_lines,
            std::array<float_32_bit,3> const&  colour_for_central_x_line,
            std::array<float_32_bit,3> const&  colour_for_central_y_line,
            std::array<float_32_bit,3> const&  colour_for_central_z_line,
            natural_32_bit const  highlight_every,
            bool const  generate_triangle_at_origin,
            buffer_ptr&  output_vertex_buffer,
            buffer_ptr&  output_colour_buffer
            )
{
    ASSUMPTION(max_x_coordinate > 0.0f);
    ASSUMPTION(max_y_coordinate > 0.0f);
    ASSUMPTION(max_z_coordinate > 0.0f);
    ASSUMPTION(step_along_x_axis > 0.0f/* && step_along_x_axis < max_x_coordinate*/);
    ASSUMPTION(step_along_y_axis > 0.0f/* && step_along_y_axis < max_y_coordinate*/);
    ASSUMPTION(highlight_every > 0U);

    std::vector< std::array<float_32_bit,3> >  vertices;
    std::vector< std::array<float_32_bit,3> >  colours;
    {
        natural_32_bit  num_steps = 0U;
        for (float_32_bit  coord_x = 0.0f; coord_x <= max_x_coordinate; coord_x += step_along_x_axis, ++num_steps)
        {
            vertices.push_back({  coord_x, -max_y_coordinate, 0.0f });
            vertices.push_back({  coord_x,  max_y_coordinate, 0.0f });
            vertices.push_back({ -coord_x, -max_y_coordinate, 0.0f });
            vertices.push_back({ -coord_x,  max_y_coordinate, 0.0f });
            if (num_steps % highlight_every == 0U)
            {
                colours.push_back(num_steps == 0U ? colour_for_central_y_line : colour_for_highlighted_y_lines);
                colours.push_back(num_steps == 0U ? colour_for_central_y_line : colour_for_highlighted_y_lines);
                colours.push_back(num_steps == 0U ? colour_for_central_y_line : colour_for_highlighted_y_lines);
                colours.push_back(num_steps == 0U ? colour_for_central_y_line : colour_for_highlighted_y_lines);
            }
            else
            {
                colours.push_back(colour_for_y_lines);
                colours.push_back(colour_for_y_lines);
                colours.push_back(colour_for_y_lines);
                colours.push_back(colour_for_y_lines);
            }
        }

        num_steps = 0U;
        for (float_32_bit  coord_y = 0.0f; coord_y <= max_y_coordinate; coord_y += step_along_y_axis, ++num_steps)
        {
            vertices.push_back({ -max_x_coordinate,  coord_y, 0.0f });
            vertices.push_back({  max_x_coordinate,  coord_y, 0.0f });
            vertices.push_back({ -max_x_coordinate, -coord_y, 0.0f });
            vertices.push_back({  max_x_coordinate, -coord_y, 0.0f });
            if (num_steps % highlight_every == 0U)
            {
                colours.push_back(num_steps == 0U ? colour_for_central_x_line : colour_for_highlighted_x_lines);
                colours.push_back(num_steps == 0U ? colour_for_central_x_line : colour_for_highlighted_x_lines);
                colours.push_back(num_steps == 0U ? colour_for_central_x_line : colour_for_highlighted_x_lines);
                colours.push_back(num_steps == 0U ? colour_for_central_x_line : colour_for_highlighted_x_lines);
            }
            else
            {
                colours.push_back(colour_for_x_lines);
                colours.push_back(colour_for_x_lines);
                colours.push_back(colour_for_x_lines);
                colours.push_back(colour_for_x_lines);
            }
        }

        vertices.push_back({ 0.0f, 0.0f, -max_z_coordinate });
        vertices.push_back({ 0.0f, 0.0f,  max_z_coordinate });
        colours.push_back(colour_for_central_z_line);
        colours.push_back(colour_for_central_z_line);

        if (generate_triangle_at_origin)
        {
            vertices.push_back({ 1.0f, 0.0f, 0.0f });
            vertices.push_back({ 0.0f, 1.0f, 0.0f });
            colours.push_back(colour_for_central_x_line);
            colours.push_back(colour_for_central_y_line);

            vertices.push_back({ 0.0f, 1.0f, 0.0f });
            vertices.push_back({ 0.0f, 0.0f, 1.0f });
            colours.push_back(colour_for_central_y_line);
            colours.push_back(colour_for_central_z_line);

            vertices.push_back({ 0.0f, 0.0f, 1.0f });
            vertices.push_back({ 1.0f, 0.0f, 0.0f });
            colours.push_back(colour_for_central_z_line);
            colours.push_back(colour_for_central_x_line);
        }
    }
    output_vertex_buffer = buffer::create(vertices);
    output_colour_buffer = buffer::create(colours);
}


}
