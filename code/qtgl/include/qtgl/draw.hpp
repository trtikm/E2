#ifndef QTGL_DRAW_HPP_INCLUDED
#   define QTGL_DRAW_HPP_INCLUDED

#   include <qtgl/glapi.hpp>
#   include <qtgl/buffer.hpp>
#   include <qtgl/shader.hpp>
#   include <qtgl/texture.hpp>
#   include <qtgl/modelspace.hpp>
#   include <qtgl/skeleton_alignment.hpp>
#   include <qtgl/camera.hpp>
#   include <qtgl/draw_state.hpp>
#   include <qtgl/batch.hpp>
#   include <qtgl/effects_config.hpp>
#   include <qtgl/shader_data_linkers.hpp>
#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <vector>

namespace qtgl {


// This function performs the actual drawing by calling
// glDrawArrays or glDrawElements of OpenGL. It is usualy
// not necessary to call it explicitly. Functions below
// do so automatically in right moments.
void  draw();


// The function provides rendering of a single setup/configuration
// of OpenGL to render primitives, like a set of triangles with proper
// shaders and textures. Any such OpenGL setup is called a 'batch' 
// in this library, see 'struct batch'. In fact, a 'batch' is NOT
// a complete setup of OpenGL. There are missing, for example, data
// to be fed into constant registers of shaders and outpput textures.
// These data are thus be passed to 'render_batch' function
// together with a desired 'batch' instance.
void  render_batch(
        batch const  batch_,
        vertex_shader_instanced_data_provider const&  vertex_instanced_data_provider,
        vertex_shader_uniform_data_provider_base const&  vertex_uniform_provider,
        fragment_shader_uniform_data_provider_base const&  fragment_uniform_provider = fragment_shader_uniform_data_provider(),
        fragment_shader_output_texture_provider_base const&  fragment_output_textures = fragment_shader_output_texture_provider()
        );


// And next follow few helper functions for simpler calling of render_batch.


void  render_batch(
        batch const  batch_,
        matrix44 const&  matrix_from_model_to_camera,
        matrix44 const&  matrix_from_camera_to_clipspace,
        vector4 const&  diffuse_colour = { 0.0f, 0.0f, 0.0f, 1.0f }
        );


void  render_batch(
        batch const  batch_,
        matrix44 const&  matrix_from_model_to_camera,
        matrix44 const&  matrix_from_camera_to_clipspace,
        vector3 const&  ambient_light_colour
        );


void  render_batch(
        batch const  batch_,
        matrix44 const&  matrix_from_world_to_camera,
        matrix44 const&  matrix_from_camera_to_clipspace,
        angeo::coordinate_system const&  coord_system,
        vector4 const&  diffuse_colour = { 0.0f, 0.0f, 0.0f, 1.0f }
        );


void  render_batch(
        batch const  batch_,
        matrix44 const&  matrix_from_model_to_camera,
        vertex_shader_uniform_data_provider_base const&  vertex_uniform_provider,
        fragment_shader_uniform_data_provider_base const&  fragment_uniform_provider = fragment_shader_uniform_data_provider()
        );


void  render_batch(
        batch const  batch_,
        vertex_shader_uniform_data_provider_base const&  vertex_uniform_provider,
        fragment_shader_uniform_data_provider_base const&  fragment_uniform_provider = fragment_shader_uniform_data_provider()
        );


void  render_batch_instances(
        batch const  batch_,
        matrix44 const&  matrix_from_camera_to_clipspace,
        vertex_shader_instanced_data_provider const&  instanced_data_provider
        );


}

#endif
