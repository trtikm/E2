#include <gfxtuner/draw_utils.hpp>


void  render_batch(
    qtgl::batch const&  batch,
    matrix44 const&  transform_matrix,
    vector4 const&  diffuse_colour
    )
{
    for (qtgl::vertex_shader_uniform_symbolic_name const uniform : batch.symbolic_names_of_used_uniforms())
        switch (uniform)
        {
        case qtgl::vertex_shader_uniform_symbolic_name::COLOUR_ALPHA:
            break;
        case qtgl::vertex_shader_uniform_symbolic_name::DIFFUSE_COLOUR:
            qtgl::set_uniform_variable(batch.shaders_binding()->uniform_variable_accessor(), uniform, diffuse_colour);
            break;
        case qtgl::vertex_shader_uniform_symbolic_name::TRANSFORM_MATRIX_TRANSPOSED:
            qtgl::set_uniform_variable(batch.shaders_binding()->uniform_variable_accessor(), uniform, transform_matrix);
            break;
        }

    qtgl::draw();
}


void  render_batch(
    qtgl::batch const&  batch,
    std::vector<matrix44> const&  transform_matrices,
    vector4 const&  diffuse_colour
    )
{
    for (qtgl::vertex_shader_uniform_symbolic_name const uniform : batch.symbolic_names_of_used_uniforms())
        switch (uniform)
        {
        case qtgl::vertex_shader_uniform_symbolic_name::COLOUR_ALPHA:
            break;
        case qtgl::vertex_shader_uniform_symbolic_name::DIFFUSE_COLOUR:
            qtgl::set_uniform_variable(batch.shaders_binding()->uniform_variable_accessor(), uniform, diffuse_colour);
            break;
        case qtgl::vertex_shader_uniform_symbolic_name::TRANSFORM_MATRIX_TRANSPOSED:
            qtgl::set_uniform_variable(batch.shaders_binding()->uniform_variable_accessor(), uniform, transform_matrices);
            break;
        }

    qtgl::draw();
}


void  render_batch(
    qtgl::batch const&  batch,
    matrix44 const&  view_projection_matrix,
    angeo::coordinate_system const&  coord_system,
    vector4 const&  diffuse_colour
    )
{
    matrix44  world_transformation;
    angeo::transformation_matrix(coord_system, world_transformation);
    matrix44 const  transform_matrix = view_projection_matrix * world_transformation;
    render_batch(batch,transform_matrix,diffuse_colour);
}
