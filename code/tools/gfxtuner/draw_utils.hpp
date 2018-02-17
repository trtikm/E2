#ifndef E2_TOOL_GFXTUNER_DRAW_UTILS_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_DRAW_UTILS_HPP_INCLUDED

#   include <qtgl/draw.hpp>
#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>


void  render_batch(
        qtgl::batch const&  batch,
        std::vector<matrix44>&  transform_matrices,
        vector4 const&  diffuse_colour = { 0.0f, 0.0f, 0.0f, 0.0f },
        bool  apply_modelspace_of_batch = true
        );


void  render_batch(
        qtgl::batch const&  batch,
        std::vector<matrix44> const&  transform_matrices,
        vector4 const&  diffuse_colour = { 0.0f, 0.0f, 0.0f, 0.0f },
        bool  apply_modelspace_of_batch = true
        );


void  render_batch(
        qtgl::batch const&  batch,
        matrix44 const&  transform_matrix,
        vector4 const&  diffuse_colour = { 0.0f, 0.0f, 0.0f, 0.0f },
        bool  apply_modelspace_of_batch = false
        );


void  render_batch(
        qtgl::batch const&  batch,
        matrix44 const&  view_projection_matrix,
        angeo::coordinate_system const&  coord_system,
        vector4 const&  diffuse_colour = { 0.0f, 0.0f, 0.0f, 0.0f }
        );


#endif
