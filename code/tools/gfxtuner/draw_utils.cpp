#include <qtgl/modelspace.hpp>
#include <gfxtuner/draw_utils.hpp>
#include <utility/assumptions.hpp>


void  render_batch(
        qtgl::batch const&  batch,
        std::vector<matrix44>&  transform_matrices,
        vector4 const&  diffuse_colour,
        bool  apply_modelspace_of_batch
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
            ASSUMPTION(!transform_matrices.empty());
            if (apply_modelspace_of_batch && batch.get_modelspace().operator bool())
            {
                qtgl::apply_modelspace_to_frame_of_keyframe_animation(*batch.get_modelspace(), transform_matrices);
                apply_modelspace_of_batch = false;
            }
            if (transform_matrices.size() == 1UL)
                qtgl::set_uniform_variable(batch.shaders_binding()->uniform_variable_accessor(), uniform, transform_matrices.front());
            else
                qtgl::set_uniform_variable(batch.shaders_binding()->uniform_variable_accessor(), uniform, transform_matrices);
            break;
        case qtgl::vertex_shader_uniform_symbolic_name::NUM_MATRICES_PER_VERTEX:
            ASSUMPTION(transform_matrices.size() >= batch.num_matrices_per_vertex());
            ASSUMPTION(transform_matrices.size() != 1UL || batch.num_matrices_per_vertex() == 1U);
            qtgl::set_uniform_variable(batch.shaders_binding()->uniform_variable_accessor(), uniform, batch.num_matrices_per_vertex());
            break;
        }

    qtgl::draw();
}


void  render_batch(
        qtgl::batch const&  batch,
        std::vector<matrix44> const&  transform_matrices,
        vector4 const&  diffuse_colour,
        bool  apply_modelspace_of_batch
        )
{
    if (apply_modelspace_of_batch)
    {
        std::vector<matrix44>  temp(transform_matrices);
        render_batch(batch, temp, diffuse_colour, apply_modelspace_of_batch);
    }
    else
        render_batch(
                batch,
                const_cast<std::vector<matrix44>&>(transform_matrices), // but, won't be modified.
                diffuse_colour,
                false
                );
}


static std::size_t  get_modelspace_size(
        qtgl::modelspace_ptr const  modelspace_ptr,
        std::size_t const  value_when_nullptr = 1UL)
{
    return modelspace_ptr == nullptr ? value_when_nullptr : modelspace_ptr->get_coord_systems().size();
}


void  render_batch(
        qtgl::batch const&  batch,
        matrix44 const&  transform_matrix,
        vector4 const&  diffuse_colour,
        bool  apply_modelspace_of_batch
        )
{
    std::vector<matrix44>  temp(get_modelspace_size(batch.get_modelspace()), transform_matrix );
    render_batch(batch, temp, diffuse_colour, apply_modelspace_of_batch);
}


void  render_batch(
        qtgl::batch const&  batch,
        matrix44 const&  view_projection_matrix,
        angeo::coordinate_system const&  coord_system,
        vector4 const&  diffuse_colour
        )
{
    matrix44  world_transformation;
    angeo::from_base_matrix(coord_system, world_transformation);
    std::vector<matrix44>  temp(get_modelspace_size(batch.get_modelspace()),
                                view_projection_matrix * world_transformation );
    render_batch(batch, temp, diffuse_colour, false);
}
