#ifndef E2_TOOL_GFXTUNER_SIMULATOR_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_SIMULATOR_HPP_INCLUDED

#   include <qtgl/real_time_simulator.hpp>
#   include <qtgl/camera.hpp>
#   include <qtgl/free_fly.hpp>
#   include <qtgl/draw.hpp>
#   include <qtgl/batch.hpp>
#   include <qtgl/modelspace.hpp>
#   include <qtgl/keyframe.hpp>
#   include <angeo/tensor_math.hpp>
#   include <vector>

struct simulator : public qtgl::real_time_simulator
{
    simulator();
    ~simulator();

    void next_round(
            float_64_bit const  seconds_from_previous_call,
            bool const  is_this_pure_redraw_request
            ) override;

    void  set_clear_color(vector3 const&  colour) { qtgl::glapi().glClearColor(colour(0), colour(1), colour(2), 1.0f); }
    void  set_show_grid_state(bool const  state) { m_do_show_grid = state; }
    vector3 const&  get_camera_position() const { return m_camera->coordinate_system()->origin(); }
    quaternion const&  get_camera_orientation() const { return m_camera->coordinate_system()->orientation(); }
    void  set_camera_position(vector3 const&  position) { m_camera->coordinate_system()->set_origin(position); }
    void  set_camera_orientation(quaternion const&  orientation) { m_camera->coordinate_system()->set_orientation(orientation); }
    void  set_camera_far_plane(float_32_bit const  far_plane) { m_camera->set_far_plane(far_plane); }
    void  set_camera_speed(float_32_bit const  speed);

    bool  paused() const noexcept { return m_paused; }

private:

    void  perform_simulation_step(float_64_bit const  time_to_simulate_in_seconds);
    void  render_simulation_state(matrix44 const&  view_projection_matrix, qtgl::draw_state_ptr  draw_state);

    /// Data providing feedback loop between a human user and 3D scene in the tool
    qtgl::camera_perspective_ptr  m_camera;
    qtgl::free_fly_config  m_free_fly_config;
    qtgl::batch_ptr  m_batch_grid;
    bool  m_do_show_grid;

    qtgl::batch_ptr  m_ske_test_batch;
    qtgl::modelspace  m_ske_test_modelspace;
    std::vector<qtgl::keyframe>  m_ske_test_keyframes;
    float_32_bit  m_ske_test_time;

    /// Data related to simulation
    bool  m_paused;
    bool  m_do_single_step;
};


#endif
