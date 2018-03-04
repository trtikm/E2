#ifndef SIMULATOR_HPP_INCLUDED
#   define SIMULATOR_HPP_INCLUDED

#   include <qtgl/real_time_simulator.hpp>
#   include <qtgl/camera.hpp>
#   include <qtgl/free_fly.hpp>
#   include <qtgl/draw.hpp>
#   include <qtgl/batch.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <angeo/tensor_math.hpp>
#   include <vector>
#   include <memory>

struct simulator : public qtgl::real_time_simulator
{
    simulator(vector3 const&  initial_clear_colour);
    ~simulator();
    void next_round(float_64_bit const  miliseconds_from_previous_call,
                    bool const  is_this_pure_redraw_request);

    void  set_clear_color(vector3 const&  colour) { qtgl::glapi().glClearColor(colour(0), colour(1), colour(2), 1.0f); }

    vector3 const&  get_camera_position() const { return m_camera->coordinate_system()->origin(); }
    quaternion const&  get_camera_orientation() const { return m_camera->coordinate_system()->orientation(); }

    void  set_camera_position(vector3 const&  position) { m_camera->coordinate_system()->set_origin(position); }
    void  set_camera_orientation(quaternion const&  orientation) { m_camera->coordinate_system()->set_orientation(orientation); }

    void  insert_batch(boost::filesystem::path const&  batch_pathname);
    void  erase_batch(boost::filesystem::path const&  batch_pathname);

private:
    qtgl::camera_perspective_ptr  m_camera;
    qtgl::free_fly_config  m_free_fly_config;

    angeo::coordinate_system_ptr  m_grid_space;
    qtgl::buffer  m_grid_vertex_buffer;
    qtgl::buffer  m_grid_colour_buffer;
    qtgl::buffers_binding  m_grid_buffers_binding;
    qtgl::shaders_binding  m_grid_shaders_binding;
    qtgl::draw_state_ptr  m_grid_draw_state;

    angeo::coordinate_system_ptr  m_batch_space;
    std::vector<qtgl::batch>  m_batches;
};

typedef std::shared_ptr<simulator>  simulator_ptr;


namespace notifications {


inline std::string  camera_position_updated() { return "CAMERA_POSITION_UPDATED"; }
inline std::string  camera_orientation_updated() { return "CAMERA_ORIENTATION_UPDATED"; }


}

#endif
