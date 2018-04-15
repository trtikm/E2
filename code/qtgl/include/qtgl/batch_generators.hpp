#ifndef QTGL_BATCH_GENERATORS_HPP_INCLUDED
#   define QTGL_BATCH_GENERATORS_HPP_INCLUDED

#   include <qtgl/batch.hpp>
#   include <angeo/tensor_math.hpp>
#   include <array>
#   include <vector>

namespace qtgl {


batch  create_lines3d(
        std::vector< std::array<float_32_bit, 3> > const&  vertices,
        std::vector< std::array<float_32_bit, 4> > const&  colours,
        bool const  use_fog = false,
        std::string const&  id = ""
        );

batch  create_lines3d(
        std::vector< std::array<float_32_bit, 3> > const&  vertices,
        std::array<float_32_bit, 4> const&  common_colour,
        bool const  use_fog = false,
        std::string const&  id = ""
        );

batch  create_lines3d(
        std::vector< std::array<float_32_bit, 3> > const&  vertices,
        vector4 const&  common_colour,
        bool const  use_fog = false,
        std::string const&  id = ""
        );

batch  create_lines3d(
        std::vector< std::pair<vector3,vector3> > const&  lines,
        std::vector< vector4 > const&  colours_of_lines,
        bool const  use_fog = false,
        std::string const&  id = ""
        );

batch  create_lines3d(
        std::vector< std::pair<vector3,vector3> > const&  lines,
        vector4 const&  common_colour,
        bool const  use_fog = false,
        std::string const&  id = ""
        );


enum struct  GRID_MAIN_AXES_ORIENTATION_MARKER_TYPE : natural_8_bit
{
    NONE                        = 0,
    TRIANGLE                    = 1,
    RIGHT_ANGLES_TO_Z_AXIS      = 2,
    RIGHT_ANGLES_TO_ALL_AXES    = 3,
};

batch  create_grid(
        float_32_bit const  max_x_coordinate,
        float_32_bit const  max_y_coordinate,
        float_32_bit const  max_z_coordinate,
        float_32_bit const  step_along_x_axis,
        float_32_bit const  step_along_y_axis,
        std::array<float_32_bit, 4> const&  colour_for_x_lines,
        std::array<float_32_bit, 4> const&  colour_for_y_lines,
        std::array<float_32_bit, 4> const&  colour_for_highlighted_x_lines,
        std::array<float_32_bit, 4> const&  colour_for_highlighted_y_lines,
        std::array<float_32_bit, 4> const&  colour_for_central_x_line,
        std::array<float_32_bit, 4> const&  colour_for_central_y_line,
        std::array<float_32_bit, 4> const&  colour_for_central_z_line,
        natural_32_bit const  highlight_every,
        GRID_MAIN_AXES_ORIENTATION_MARKER_TYPE const  main_exes_orientation_marker_type,
        bool const  use_fog = false,
        std::string const&  id = ""
        );


batch  create_basis_vectors(bool const  use_fog = false);


batch  create_wireframe_box(
        vector3 const&  lo_corner,
        vector3 const&  hi_corner,
        vector4 const&  colour,
        bool const  use_fog = false,
        std::string const&  id = ""
        );


batch  create_wireframe_sphere(
        float_32_bit const  radius,
        natural_8_bit const  num_lines_per_quarter_of_circle,
        vector4 const&  colour,
        bool const  use_fog = false,
        std::string const&  id = ""
        );


batch  create_wireframe_perspective_frustum(
        float_32_bit const  near_plane,
        float_32_bit const  far_plane,
        float_32_bit const  left_plane,
        float_32_bit const  right_plane,
        float_32_bit const  top_plane,
        float_32_bit const  bottom_plane,
        vector4 const&  colour,
        bool const  use_fog = false,
        std::string const&  id = ""
        );


}

#endif
