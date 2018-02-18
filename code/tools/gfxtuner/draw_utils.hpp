#ifndef E2_TOOL_GFXTUNER_DRAW_UTILS_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_DRAW_UTILS_HPP_INCLUDED

#   include <qtgl/draw.hpp>
#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <vector>


// NOTE on usage of 'render_batch' functions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// (a) Use of 'old_draw_state' parameter
//
// All 'render_batch' functions below accept a parameter:
//      qtgl::draw_state_ptr &  old_draw_state
// It should represent a draw state, which was passed to the preceeding
// call to any 'render_batch' function. That old draw state is compared
// with the current (i.e. the one inside the passed batch) in order to
// identify a minimal set of state changes which must be done to OpenGL.
// Before the first call to a 'render_batch' function in the current
// rendering frame create a fresh variable:
//      qtgl::draw_state_ptr draw_state; // yes, it will be nullptr initially.
// and pass it as 'old_draw_state' to all your calls to 'render_batch'
// functions. Understand, each call to 'render_batch' function automatically
// updates your variable to the lastly used draw_state, i.e. from
// the lastly rendered batch (or remains the same, if the rendering
// of the batch has failed).
// (You can of course always create and pass a fresh variable (with
// the nullptr value) to a 'render_batch' function, but it might lead
// to some unnecessary calls to OpenGL, which would set things
// which were already set in some of the previous calls.)
//
// (b) Use of 'apply_modelspace_of_batch' parameter
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


bool  render_batch(
        qtgl::batch const&  batch,
        std::vector<matrix44>&  transform_matrices,
        qtgl::draw_state_ptr&  old_draw_state,
        vector4 const&  diffuse_colour = { 0.0f, 0.0f, 0.0f, 0.0f },
        bool  apply_modelspace_of_batch = true
        );


bool  render_batch(
        qtgl::batch const&  batch,
        std::vector<matrix44> const&  transform_matrices,
        qtgl::draw_state_ptr&  old_draw_state,
        vector4 const&  diffuse_colour = { 0.0f, 0.0f, 0.0f, 0.0f },
        bool const  apply_modelspace_of_batch = true
        );


bool  render_batch(
        qtgl::batch const&  batch,
        matrix44 const&  transform_matrix,
        qtgl::draw_state_ptr&  old_draw_state,
        vector4 const&  diffuse_colour = { 0.0f, 0.0f, 0.0f, 0.0f },
        bool const  apply_modelspace_of_batch = false
        );


bool  render_batch(
        qtgl::batch const&  batch,
        matrix44 const&  view_projection_matrix,
        qtgl::draw_state_ptr&  old_draw_state,
        angeo::coordinate_system const&  coord_system,
        vector4 const&  diffuse_colour = { 0.0f, 0.0f, 0.0f, 0.0f }
        );


#endif
