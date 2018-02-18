#ifndef QTGL_DRAW_HPP_INCLUDED
#   define QTGL_DRAW_HPP_INCLUDED

#   include <qtgl/glapi.hpp>
#   include <qtgl/buffer.hpp>
#   include <qtgl/shader.hpp>
#   include <qtgl/texture.hpp>
#   include <qtgl/camera.hpp>
#   include <qtgl/draw_state.hpp>
#   include <qtgl/batch.hpp>
#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <vector>

namespace qtgl {


// This function performs the actual drawing by calling
// glDrawArrays or glDrawElements of OpenGL. It is usualy
// not necessary to call it explicitly. Functions below
// do so automatically in right moments.
void  draw();


// 'render_batch' functions
// ~~~~~~~~~~~~~~~~~~~~~~~~
//
// These functions provide rendering of a single setup/configuration
// of OpenGL to render primitives, like a set of triangles with proper
// shaders and textures. Any such OpenGL setup is called '(render) batch' 
// in this library, see 'struct batch'. In fact, a 'batch' is NOT
// a complete setup of OpenGL. There are missing data, which are supposed
// to be fed into constant registers of shaders, like transformation
// matrices. These data must thus be passed to 'render_batch' functions
// together with a desired 'batch' instance. Here are descriptions of
// some of the other parameters.
//
// (*) Use of 'apply_modelspace_of_batch' parameter
//
// The parameter indicates whether the called 'render_batch' function
// should apply the model-space transformation of the passed batch
// or not. If the batch does not have a model-space defined (i.e.
// there is no keyframe animation defined for it), or if you already
// applied the model-space transformations of the batch to the passed
// matrix/matrices by yourself, or if no keyframe transformation was
// applied to the passed matrices, then pass 'false' to the parameter.
// Otherwise pass 'true'. The default values for the parameter match
// typical usage. So, initially do not specify the parameter (use the
// default value). Only in case of wrong rendered output check, if the
// default value is vaild in your case.


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
        bool const  apply_modelspace_of_batch = true
        );


void  render_batch(
        qtgl::batch const&  batch,
        matrix44 const&  transform_matrix,
        vector4 const&  diffuse_colour = { 0.0f, 0.0f, 0.0f, 0.0f },
        bool const  apply_modelspace_of_batch = false
        );


void  render_batch(
        qtgl::batch const&  batch,
        matrix44 const&  view_projection_matrix,
        angeo::coordinate_system const&  coord_system,
        vector4 const&  diffuse_colour = { 0.0f, 0.0f, 0.0f, 0.0f }
        );


}

#endif
