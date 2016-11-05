#ifndef QTGL_BUFFER_GENERATORS_HPP_INCLUDED
#   define QTGL_BUFFER_GENERATORS_HPP_INCLUDED

#   include <qtgl/buffer.hpp>
#   include <angeo/tensor_math.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <array>
#   include <string>

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
        buffer_ptr&  output_colour_buffer,
        std::string const&  id = ""
        );


void  create_basis_vectors_vertex_and_colour_buffers(
        buffer_ptr&  output_vertex_buffer,
        buffer_ptr&  output_colour_buffer
        );


void  create_wireframe_box_vertex_buffer(
        vector3 const&  lo_corner,
        vector3 const&  hi_corner,
        buffer_ptr&  output_vertex_buffer,
        std::string const&  id = ""
        );


void  create_wireframe_sphere_vertex_buffer(
        float_32_bit const  radius,
        natural_8_bit const  num_lines_per_quarter_of_circle,
        buffer_ptr&  output_vertex_buffer,
        std::string const&  id = ""
        );


void  create_wireframe_perspective_frustum_vertex_buffer(
        float_32_bit const  near_plane,
        float_32_bit const  far_plane,
        float_32_bit const  left_plane,
        float_32_bit const  right_plane,
        float_32_bit const  top_plane,
        float_32_bit const  bottom_plane,
        buffer_ptr&  output_vertex_buffer,
        std::string const&  id = ""
        );


}

#endif
