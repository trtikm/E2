#include <gfx/draw.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>

namespace gfx { namespace detail { namespace current_draw {


static bool  s_are_buffers_ready = false;
static GLuint  s_id = 0U;
static natural_8_bit  s_num_components_per_primitive = 0U;
static natural_32_bit  s_num_primitives = 0U;
static natural_32_bit  s_num_instances = 0U;

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


void  set_num_instances(natural_32_bit const  num_instances)
{
    s_num_instances = num_instances;
}


}}}

namespace gfx {


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
        case 1U:
            if (s_num_instances == 0U)
                glDrawArrays(GL_POINTS,0U,s_num_primitives);
            else
                glDrawArraysInstanced(GL_POINTS, 0U, s_num_primitives, s_num_instances);
            break;
        case 2U:
            if (s_num_instances == 0U)
                glDrawArrays(GL_LINES,0U,s_num_primitives);
            else
                glDrawArraysInstanced(GL_LINES, 0U, s_num_primitives, s_num_instances);
            break;
        case 3U:
            if (s_num_instances == 0U)
                glDrawArrays(GL_TRIANGLES,0U,s_num_primitives);
            else
                glDrawArraysInstanced(GL_TRIANGLES, 0U, s_num_primitives, s_num_instances);
            break;
        default: UNREACHABLE();
        }
    else
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,s_id);
        switch (s_num_components_per_primitive)
        {
        case 1U:
            if (s_num_instances == 0U)
                glDrawElements(GL_POINTS,1U * s_num_primitives,GL_UNSIGNED_INT,nullptr);
            else
                glDrawElementsInstanced(GL_POINTS, 1U * s_num_primitives, GL_UNSIGNED_INT, nullptr, s_num_instances);
            break;
        case 2U:
            if (s_num_instances == 0U)
                glDrawElements(GL_LINES,2U * s_num_primitives,GL_UNSIGNED_INT,nullptr);
            else
                glDrawElementsInstanced(GL_LINES, 2U * s_num_primitives, GL_UNSIGNED_INT, nullptr, s_num_instances);
            break;
        case 3U:
            if (s_num_instances == 0U)
                glDrawElements(GL_TRIANGLES,3U * s_num_primitives,GL_UNSIGNED_INT,nullptr);
            else
                glDrawElementsInstanced(GL_TRIANGLES, 3U * s_num_primitives, GL_UNSIGNED_INT, nullptr, s_num_instances);
            break;
        default: UNREACHABLE();
        }
    }
}


void  render_batch(
        batch const  batch_,
        vertex_shader_instanced_data_provider const&  vertex_instanced_data_provider,
        vertex_shader_uniform_data_provider_base const&  vertex_uniform_provider,
        fragment_shader_uniform_data_provider_base const&  fragment_uniform_provider,
        fragment_shader_output_texture_provider_base const&  fragment_output_textures
        )
{
    TMPROF_BLOCK();

    if (vertex_instanced_data_provider.make_current() == false)
        return;
    detail::current_draw::set_num_instances(vertex_instanced_data_provider.get_num_instances());

    {
        vertex_shader  shader = vertex_instanced_data_provider.get_num_instances() > 0U ?
                                        batch_.get_instancing_data_ptr()->m_shaders_binding.get_vertex_shader() :
                                        batch_.get_shaders_binding().get_vertex_shader();
        for (VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME const uniform : shader.get_symbolic_names_of_used_uniforms())
            switch (uniform)
            {
            case VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::MATRIX_FROM_MODEL_TO_CAMERA:
                shader.set_uniform_variable(uniform, vertex_uniform_provider.get_MATRIX_FROM_MODEL_TO_CAMERA());
                break;
            case VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::MATRIX_FROM_CAMERA_TO_CLIPSPACE:
                shader.set_uniform_variable(uniform, vertex_uniform_provider.get_MATRIX_FROM_CAMERA_TO_CLIPSPACE());
                break;
            case VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::MATRICES_FROM_MODEL_TO_CAMERA:
                shader.set_uniform_variable(uniform, vertex_uniform_provider.get_MATRICES_FROM_MODEL_TO_CAMERA());
                break;
            case VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::NUM_MATRICES_PER_VERTEX:
                shader.set_uniform_variable(uniform, vertex_uniform_provider.get_NUM_MATRICES_PER_VERTEX());
                break;
            case VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::AMBIENT_COLOUR:
                shader.set_uniform_variable(uniform, vertex_uniform_provider.get_AMBIENT_COLOUR());
                break;
            case VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::DIFFUSE_COLOUR:
                shader.set_uniform_variable(uniform, vertex_uniform_provider.get_DIFFUSE_COLOUR());
                break;
            case VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::SPECULAR_COLOUR:
                shader.set_uniform_variable(uniform, vertex_uniform_provider.get_SPECULAR_COLOUR());
                break;
            case VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::DIRECTIONAL_LIGHT_DIRECTION:
                shader.set_uniform_variable(uniform, vertex_uniform_provider.get_DIRECTIONAL_LIGHT_DIRECTION());
                break;
            case VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::DIRECTIONAL_LIGHT_COLOUR:
                shader.set_uniform_variable(uniform, vertex_uniform_provider.get_DIRECTIONAL_LIGHT_COLOUR());
                break;
            case VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::FOG_COLOUR:
                shader.set_uniform_variable(uniform, vertex_uniform_provider.get_FOG_COLOUR());
                break;
            case VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::FOG_NEAR:
                shader.set_uniform_variable(uniform, vertex_uniform_provider.get_FOG_NEAR());
                break;
            case VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::FOG_FAR:
                shader.set_uniform_variable(uniform, vertex_uniform_provider.get_FOG_FAR());
                break;
            default:
                UNREACHABLE();
                break;
            }
    }

    {
        fragment_shader  shader = batch_.get_shaders_binding().get_fragment_shader();
        for (FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME const uniform : shader.get_symbolic_names_of_used_uniforms())
            switch (uniform)
            {
            case FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::TEXTURE_SAMPLER_DIFFUSE:
            case FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::TEXTURE_SAMPLER_SPECULAR:
            case FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::TEXTURE_SAMPLER_NORMAL:
                shader.set_uniform_variable(uniform, (integer_32_bit)value(uniform));
                break;
            case FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::AMBIENT_COLOUR:
                shader.set_uniform_variable(uniform, fragment_uniform_provider.get_AMBIENT_COLOUR());
                break;
            case FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::DIFFUSE_COLOUR:
                shader.set_uniform_variable(uniform, fragment_uniform_provider.get_DIFFUSE_COLOUR());
                break;
            case FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::SPECULAR_COLOUR:
                shader.set_uniform_variable(uniform, fragment_uniform_provider.get_SPECULAR_COLOUR());
                break;
            case FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::DIRECTIONAL_LIGHT_DIRECTION:
                shader.set_uniform_variable(uniform, fragment_uniform_provider.get_DIRECTIONAL_LIGHT_DIRECTION());
                break;
            case FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::DIRECTIONAL_LIGHT_COLOUR:
                shader.set_uniform_variable(uniform, fragment_uniform_provider.get_DIRECTIONAL_LIGHT_COLOUR());
                break;
            case FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::FOG_COLOUR:
                shader.set_uniform_variable(uniform, fragment_uniform_provider.get_FOG_COLOUR());
                break;
            case FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::FOG_NEAR:
                shader.set_uniform_variable(uniform, fragment_uniform_provider.get_FOG_NEAR());
                break;
            case FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::FOG_FAR:
                shader.set_uniform_variable(uniform, fragment_uniform_provider.get_FOG_FAR());
                break;
            case FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::ALPHA_TEST_CONSTANT:
                shader.set_uniform_variable(uniform, batch_.get_available_resources().skins().at(batch_.get_skin_name()).alpha_testing().alpha_test_constant());
                break;
            default:
                UNREACHABLE();
                break;
            }
        for (FRAGMENT_SHADER_OUTPUT_BINDING_LOCATION const location : shader.get_output_buffer_bindings())
            switch (location)
            {
            case FRAGMENT_SHADER_OUTPUT_BINDING_LOCATION::BINDING_OUT_COLOUR:
                // Nothing to set here (a texture sampler cannot be set any data).
                break;
            case FRAGMENT_SHADER_OUTPUT_BINDING_LOCATION::BINDING_OUT_TEXTURE_POSITION:
            case FRAGMENT_SHADER_OUTPUT_BINDING_LOCATION::BINDING_OUT_TEXTURE_NORMAL:
            case FRAGMENT_SHADER_OUTPUT_BINDING_LOCATION::BINDING_OUT_TEXTURE_DIFFUSE:
            case FRAGMENT_SHADER_OUTPUT_BINDING_LOCATION::BINDING_OUT_TEXTURE_SPECULAR:
                NOT_IMPLEMENTED_YET();
                break;
            default:
                UNREACHABLE();
                break;
            }
    }

    draw();
}


void  render_batch(
        batch const  batch_,
        matrix44 const&  matrix_from_model_to_camera,
        matrix44 const&  matrix_from_camera_to_clipspace,
        vector4 const&  diffuse_colour
        )
{
    render_batch(
        batch_,
        vertex_shader_instanced_data_provider(),
        vertex_shader_uniform_data_provider(batch_, { matrix_from_model_to_camera }, matrix_from_camera_to_clipspace, diffuse_colour)
        );
}


void  render_batch(
        batch const  batch_,
        matrix44 const&  matrix_from_model_to_camera,
        matrix44 const&  matrix_from_camera_to_clipspace,
        vector3 const&  ambient_light_colour
        )
{
    render_batch(
        batch_,
        vertex_shader_instanced_data_provider(),
        vertex_shader_uniform_data_provider(
                batch_,
                { matrix_from_model_to_camera }, matrix_from_camera_to_clipspace,
                { 0.0f, 0.0f, 0.0f, 1.0f },
                ambient_light_colour
                )
        );
}


void  render_batch(
        batch const  batch_,
        matrix44 const&  matrix_from_world_to_camera,
        matrix44 const&  matrix_from_camera_to_clipspace,
        angeo::coordinate_system const&  coord_system,
        vector4 const&  diffuse_colour
        )
{
    matrix44  matrix_from_model_to_camera;
    {
        matrix44  matrix_from_model_to_world;
        angeo::from_base_matrix(coord_system, matrix_from_model_to_world);
        matrix_from_model_to_camera = matrix_from_world_to_camera * matrix_from_model_to_world;
    }
    render_batch(batch_, matrix_from_model_to_camera, matrix_from_camera_to_clipspace, diffuse_colour);
}


void  render_batch(
        batch const  batch_,
        matrix44 const&  matrix_from_model_to_camera,
        vertex_shader_uniform_data_provider_base const&  vertex_uniform_provider,
        fragment_shader_uniform_data_provider_base const&  fragment_uniform_provider
        )
{
    ASSUMPTION(batch_.has_instancing_data());

    vertex_shader_instanced_data_provider  instanced_data_provider(batch_);
    instanced_data_provider.insert_from_model_to_camera_matrix(matrix_from_model_to_camera);

    render_batch(
        batch_,
        instanced_data_provider,
        vertex_uniform_provider,
        fragment_uniform_provider
        );
}


void  render_batch(
        batch const  batch_,
        vertex_shader_uniform_data_provider_base const&  vertex_uniform_provider,
        fragment_shader_uniform_data_provider_base const&  fragment_uniform_provider
        )
{
    render_batch(
        batch_,
        vertex_shader_instanced_data_provider(),
        vertex_uniform_provider,
        fragment_uniform_provider
        );
}


void  render_batch_instances(
        batch const  batch_,
        matrix44 const&  matrix_from_camera_to_clipspace,
        vertex_shader_instanced_data_provider const&  instanced_data_provider
        )
{
    render_batch(
        batch_,
        instanced_data_provider,
        vertex_shader_uniform_data_provider(batch_, {}, matrix_from_camera_to_clipspace)
        );
}


void  render_batch(
        batch const  batch_,
        vector3 const&  position_in_camera_space,
        float_32_bit const  scale,
        matrix44 const&  matrix_from_camera_to_clipspace,
        vector3 const&  ambient_light_colour
        )
{
    matrix44 mat_pos = matrix44_identity();
    mat_pos(0, 3) = position_in_camera_space(0);
    mat_pos(1, 3) = position_in_camera_space(1);
    mat_pos(2, 3) = position_in_camera_space(2);

    matrix44 mat_scale = matrix44_identity();
    mat_scale(0, 0) = mat_scale(1, 1) = scale;

    render_batch(
        batch_,
        vertex_shader_instanced_data_provider(),
        vertex_shader_uniform_data_provider(
                batch_,
                { mat_pos * mat_scale },
                matrix_from_camera_to_clipspace,
                { 0.0f, 0.0f, 0.0f, 1.0f },
                ambient_light_colour
                )
        );
}


extern texture  get_sprite_texture(batch const  sprite_batch);


void  render_sprite_batch(
        batch const  sprite_batch_,
        natural_32_bit const  x_screen_pos,
        natural_32_bit const  y_screen_pos,
        natural_32_bit const  screen_width_in_pixels,
        natural_32_bit const  screen_height_in_pixels,
        float_32_bit const  scale
        )
{
    matrix44  matrix_from_world_to_camera;
    {
        texture const  tex = get_sprite_texture(sprite_batch_);

        float_32_bit const sx = scale * ((float_32_bit)tex.width()) / ((float_32_bit)screen_width_in_pixels);
        float_32_bit const sy = scale * ((float_32_bit)tex.height()) / ((float_32_bit)screen_height_in_pixels);
        float_32_bit const px = 2.0f * ((float_32_bit)x_screen_pos) / ((float_32_bit)screen_width_in_pixels) - 1.0f + sx;
        float_32_bit const py = 2.0f * ((float_32_bit)y_screen_pos) / ((float_32_bit)screen_height_in_pixels) - 1.0f + sy;

        matrix_from_world_to_camera = matrix44_identity();
        matrix_from_world_to_camera(0, 0) = sx;
        matrix_from_world_to_camera(0, 3) = px;
        matrix_from_world_to_camera(1, 1) = sy;
        matrix_from_world_to_camera(1, 3) = py;
    }
    render_batch(
        sprite_batch_,
        vertex_shader_uniform_data_provider(sprite_batch_, { matrix_from_world_to_camera }, matrix44_identity()),
        fragment_shader_uniform_data_provider()
        );
}


void  render_batch(
        batch const  batch_,
        std::vector<matrix44> const&  world_matrices_,
        std::vector<matrix44> const&  to_bone_space_matrices_,  // Not used for batches without skeleton.
        matrix44 const&  matrix_from_world_to_camera_,
        matrix44 const&  matrix_from_camera_to_clipspace_,
        vector4 const&  diffuse_colour_,
        vector3 const&  ambient_colour_,
        vector4 const&  specular_colour_,
        vector3 const&  directional_light_direction_, // In world space or in camera space based on the boolean flag below.
        vector3 const&  directional_light_colour_,
        bool const  is_directional_light_direction_in_camera_space_, // When false, then it is assumed in world space.
        vector4 const&  fog_colour_,
        float const  fog_near_,
        float const  fog_far_,
        draw_state*  draw_state_ptr_
        )
{
    if (world_matrices_.empty())
        return;

    bool const  use_instancing = world_matrices_.size() > 1UL && !batch_.is_attached_to_skeleton() && batch_.has_instancing_data();

    if (!make_current(batch_, draw_state_ptr_ == nullptr ? draw_state() : *draw_state_ptr_, use_instancing))
        return;

    vertex_shader_uniform_data_provider  vs_uniform_data_provider(
            batch_,
            {},
            matrix_from_camera_to_clipspace_,
            diffuse_colour_,
            ambient_colour_,
            specular_colour_,
            is_directional_light_direction_in_camera_space_ ?
                    directional_light_direction_ : transform_vector(directional_light_direction_, matrix_from_world_to_camera_),
            directional_light_colour_,
            fog_colour_,
            fog_near_,
            fog_far_
            );

    std::vector<matrix44>&  vs_matrices = vs_uniform_data_provider.MATRICES_FROM_MODEL_TO_CAMERA_ref();

    fragment_shader_uniform_data_provider const  fs_uniform_data_provider(
            vs_uniform_data_provider.get_DIFFUSE_COLOUR(),
            vs_uniform_data_provider.get_AMBIENT_COLOUR(),
            vs_uniform_data_provider.get_SPECULAR_COLOUR(),
            vs_uniform_data_provider.get_DIRECTIONAL_LIGHT_DIRECTION(),
            vs_uniform_data_provider.get_DIRECTIONAL_LIGHT_COLOUR(),
            vs_uniform_data_provider.get_FOG_COLOUR(),
            vs_uniform_data_provider.get_FOG_NEAR(),
            vs_uniform_data_provider.get_FOG_FAR()
            );

    if (use_instancing)
    {
        vertex_shader_instanced_data_provider  instanced_data_provider(batch_);
        for (matrix44 const&  world_matrix : world_matrices_)
            instanced_data_provider.insert_from_model_to_camera_matrix(matrix_from_world_to_camera_ * world_matrix);
        gfx::render_batch(batch_, instanced_data_provider, vs_uniform_data_provider, fs_uniform_data_provider);
        if (draw_state_ptr_ != nullptr)
            *draw_state_ptr_ = batch_.get_draw_state();
    }
    else if (batch_.is_attached_to_skeleton())
    {
        INVARIANT(
                !to_bone_space_matrices_.empty() &&
                world_matrices_.size() >= to_bone_space_matrices_.size() &&
                world_matrices_.size() % to_bone_space_matrices_.size() == 0U
                );
        vs_matrices.reserve(to_bone_space_matrices_.size());

        natural_32_bit const  n = (natural_32_bit)world_matrices_.size();
        natural_32_bit const  m = (natural_32_bit)to_bone_space_matrices_.size();
        for (natural_32_bit  i = 0U; i != n; )
        {
            vs_matrices.clear();
            for (natural_32_bit  j = 0U; j != m; ++j, ++i)
                vs_matrices.push_back(matrix_from_world_to_camera_ * world_matrices_.at(i) * to_bone_space_matrices_.at(j));
            gfx::render_batch(batch_, vs_uniform_data_provider, fs_uniform_data_provider);
            if (draw_state_ptr_ != nullptr)
                *draw_state_ptr_ = batch_.get_draw_state();
        }
    }
    else
    {
        vs_matrices.push_back({});
        for (matrix44 const&  world_matrix : world_matrices_)
        {
            vs_matrices.front() = matrix_from_world_to_camera_ * world_matrix;
            render_batch(batch_, vs_uniform_data_provider, fs_uniform_data_provider);
            if (draw_state_ptr_ != nullptr)
                *draw_state_ptr_ = batch_.get_draw_state();
        }
    }
}


}
