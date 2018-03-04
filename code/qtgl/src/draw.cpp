#include <qtgl/draw.hpp>
#include <qtgl/modelspace.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>

namespace qtgl { namespace detail { namespace current_draw {


static bool  s_are_buffers_ready = false;
static GLuint  s_id = 0U;
static natural_8_bit  s_num_components_per_primitive = 0U;
static natural_32_bit  s_num_primitives = 0U;

void  set_are_buffers_ready(bool const  are_buffers_ready)
{
    s_are_buffers_ready = are_buffers_ready;
}

void  set_index_buffer_id(GLuint const id)
{
    s_id = id;
}

void  set_num_components_per_primitive(natural_8_bit const  num_components_per_primitive)
{
    ASSUMPTION(num_components_per_primitive == 2U ||
               num_components_per_primitive == 3U ||
               num_components_per_primitive == 4U );
    s_num_components_per_primitive = num_components_per_primitive;
}

void  set_num_primitives(natural_32_bit const  num_primitives)
{
    ASSUMPTION(num_primitives > 0U);
    s_num_primitives = num_primitives;
}


}}}

namespace qtgl {


void  draw()
{
    TMPROF_BLOCK();

    using namespace detail::current_draw;

    if (!s_are_buffers_ready)
        return;

    ASSUMPTION(s_num_primitives > 0U);

    if (s_id == 0U)
        switch (s_num_components_per_primitive)
        {
        case 1U: glapi().glDrawArrays(GL_POINTS,0U,s_num_primitives); break;
        case 2U: glapi().glDrawArrays(GL_LINES,0U,s_num_primitives); break;
        case 3U: glapi().glDrawArrays(GL_TRIANGLES,0U,s_num_primitives); break;
        default: UNREACHABLE();
        }
    else
    {
        glapi().glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,s_id);
        switch (s_num_components_per_primitive)
        {
        case 1U: glapi().glDrawElements(GL_POINTS,1U * s_num_primitives,GL_UNSIGNED_INT,nullptr); break;
        case 2U: glapi().glDrawElements(GL_LINES,2U * s_num_primitives,GL_UNSIGNED_INT,nullptr); break;
        case 3U: glapi().glDrawElements(GL_TRIANGLES,3U * s_num_primitives,GL_UNSIGNED_INT,nullptr); break;
        default: UNREACHABLE();
        }
    }
}


void  render_batch(
        batch const&  batch,
        std::vector<matrix44>&  transform_matrices,
        vector4 const&  diffuse_colour,
        bool  apply_modelspace_of_batch
        )
{
    TMPROF_BLOCK();

    vertex_shader  vertex_shader = batch.get_shaders_binding().get_vertex_shader();
    for (vertex_shader_uniform_symbolic_name const uniform : vertex_shader.get_symbolic_names_of_used_uniforms())
        switch (uniform)
        {
        case vertex_shader_uniform_symbolic_name::COLOUR_ALPHA:
            break;
        case vertex_shader_uniform_symbolic_name::DIFFUSE_COLOUR:
            vertex_shader.set_uniform_variable(uniform,diffuse_colour);
            break;
        case vertex_shader_uniform_symbolic_name::TRANSFORM_MATRIX_TRANSPOSED:
            ASSUMPTION(!transform_matrices.empty());
            if (apply_modelspace_of_batch && batch.get_modelspace().loaded_successfully())
            {
                apply_modelspace_to_frame_of_keyframe_animation(
                        batch.get_modelspace(),
                        transform_matrices
                        );
                apply_modelspace_of_batch = false;
            }
            if (transform_matrices.size() == 1UL)
                vertex_shader.set_uniform_variable(uniform,transform_matrices.front());
            else
                vertex_shader.set_uniform_variable(uniform, transform_matrices);
            break;
        case vertex_shader_uniform_symbolic_name::NUM_MATRICES_PER_VERTEX:
            ASSUMPTION(transform_matrices.size() >= batch.get_buffers_binding().num_matrices_per_vertex());
            ASSUMPTION(transform_matrices.size() != 1UL || batch.get_buffers_binding().num_matrices_per_vertex() == 1U);
            vertex_shader.set_uniform_variable(uniform, batch.get_buffers_binding().num_matrices_per_vertex());
            break;
        }

    draw();
}


void  render_batch(
        batch const&  batch,
        std::vector<matrix44> const&  transform_matrices,
        vector4 const&  diffuse_colour,
        bool const  apply_modelspace_of_batch
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
        modelspace const  modelspace,
        std::size_t const  value_when_nullptr = 1UL)
{
    return modelspace.loaded_successfully() ? modelspace.get_coord_systems().size() : value_when_nullptr;
}


void  render_batch(
        batch const&  batch,
        matrix44 const&  transform_matrix,
        vector4 const&  diffuse_colour,
        bool const  apply_modelspace_of_batch
        )
{
    std::vector<matrix44>  temp(get_modelspace_size(batch.get_modelspace()), transform_matrix );
    render_batch(batch, temp, diffuse_colour, apply_modelspace_of_batch);
}


void  render_batch(
        batch const&  batch,
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


}
