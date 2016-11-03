#ifndef QTGL_BATCH_GENERATORS_HPP_INCLUDED
#   define QTGL_BATCH_GENERATORS_HPP_INCLUDED

#   include <qtgl/batch.hpp>
#   include <angeo/tensor_math.hpp>
#   include <array>
#   include <vector>

namespace qtgl {


batch_ptr  create_grid(
    float_32_bit const  max_x_coordinate,
    float_32_bit const  max_y_coordinate,
    float_32_bit const  max_z_coordinate,
    float_32_bit const  step_along_x_axis,
    float_32_bit const  step_along_y_axis,
    std::array<float_32_bit, 3> const&  colour_for_x_lines,
    std::array<float_32_bit, 3> const&  colour_for_y_lines,
    std::array<float_32_bit, 3> const&  colour_for_highlighted_x_lines,
    std::array<float_32_bit, 3> const&  colour_for_highlighted_y_lines,
    std::array<float_32_bit, 3> const&  colour_for_central_x_line,
    std::array<float_32_bit, 3> const&  colour_for_central_y_line,
    std::array<float_32_bit, 3> const&  colour_for_central_z_line,
    natural_32_bit const  highlight_every,
    bool const  generate_triangle_at_origin,
    std::string const&  id = ""
    );


batch_ptr  create_lines3d(
    std::vector< std::array<float_32_bit, 3> >  vertices,
    std::vector< std::array<float_32_bit, 3> >  colours,
    std::string const&  id = ""
    );

batch_ptr  create_lines3d(
    std::vector< std::pair<vector3,vector3> > const&  lines,
    vector3 const&  common_colour,
    std::string const&  id = ""
    );


batch_ptr  create_basis_vectors();


batch_ptr  create_wireframe_box(
        vector3 const&  lo_corner,
        vector3 const&  hi_corner,
        std::string const&  id = ""
        );


batch_ptr  create_wireframe_sphere(
        float_32_bit const  radius,
        natural_8_bit const  num_lines_per_quarter_of_circle,
        std::string const&  id = ""
        );

}

#endif
