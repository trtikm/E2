#ifndef QTGL_BUFFER_GENERATORS_HPP_INCLUDED
#   define QTGL_BUFFER_GENERATORS_HPP_INCLUDED

#   include <qtgl/buffer.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <array>

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
            );


}

#endif
