#ifndef COM_SIMULATOR_HPP_INCLUDED
#   define COM_SIMULATOR_HPP_INCLUDED

#   include <com/simulation_context.hpp>
#   include <osi/simulator.hpp>
#   include <gfx/camera.hpp>
#   include <gfx/free_fly.hpp>
#   include <gfx/effects_config.hpp>
#   include <angeo/tensor_math.hpp>
#   include <angeo/collision_scene.hpp>
#   include <angeo/rigid_body_simulator.hpp>
//#   include <ai/simulator.hpp>
#   include <string>
#   include <memory>

namespace com {


struct  simulator : public osi::simulator
{
    struct  simulation_configuration
    {
        simulation_configuration();
        bool  paused;
        natural_32_bit  num_rounds_to_pause;
    };

    struct  render_configuration
    {
        render_configuration(osi::window_props const&  wnd_props, std::string const&  data_root_dir);
        void  terminate();
        // Global config - fields are only initialised in the constructor and then never changed in this class.
        //                 Feel free to modify these field, ideally in the callback 'on_begin_round()'
        gfx::free_fly_config  free_fly_config;
        gfx::effects_config  effects_config;
        gfx::font_mono_props  font_props;
        vector3  clear_colour;
        vector4  diffuse_colour;
        vector3  ambient_colour;
        vector4  specular_colour;
        vector3  directional_light_direction;
        vector3  directional_light_colour;
        vector4  fog_colour;
        float  fog_near;
        float  fog_far;
        float_32_bit  text_scale;
        vector3  text_shift;
        vector3  text_ambient_colour;
        std::string  fps_prefix;
        gfx::batch  batch_grid;
        gfx::batch  batch_frame;
        bool  render_fps;
        bool  render_grid;
        bool  render_frames;
        bool  render_text;
        bool  render_in_wireframe;
        bool  render_scene_batches;
        bool  render_colliders_of_rigid_bodies;
        bool  render_colliders_of_sensors;
        bool  render_colliders_of_activators;
        bool  render_colliders_of_agents;
        bool  render_colliders_of_ray_casts;
        bool  render_collision_contacts;
        // Current round config - changes from round to round. So, your changes affect only the current round.
        //                        They are reset in the next round. Perform your changes in the callback 'on_begin_round()'.
        gfx::camera_perspective_ptr  camera;
        matrix44  matrix_from_world_to_camera;
        matrix44  matrix_from_camera_to_clipspace;
        matrix44  matrix_ortho_projection_for_text;
        vector3  directional_light_direction_in_camera_space;
        gfx::draw_state  draw_state;
    };

    simulator(std::string const&  data_root_dir = "../data");
    ~simulator() override;

    void  initialise() override;
    void  terminate() override;

    void  round() override;

    virtual void  on_begin_round() {}

        virtual void  on_begin_simulation() {}
        virtual void  on_end_simulation() {}

        virtual void  on_begin_camera_update() {}
        virtual void  on_end_camera_update() {}

        virtual void  on_begin_render() {}
        virtual void  on_end_render() {}

    virtual void  on_end_round() {}

    std::shared_ptr<angeo::collision_scene>  collision_scene() { return m_collision_scene_ptr; }
    std::shared_ptr<angeo::collision_scene const>  collision_scene() const { return m_collision_scene_ptr; }

    std::shared_ptr<angeo::rigid_body_simulator>  rigid_body_simulator() { return m_rigid_body_simulator_ptr; }
    std::shared_ptr<angeo::rigid_body_simulator const>  rigid_body_simulator() const { return m_rigid_body_simulator_ptr; }

    std::shared_ptr<ai::simulator>  ai_simulator() { return m_ai_simulator_ptr; }
    std::shared_ptr<ai::simulator const>  ai_simulator() const { return m_ai_simulator_ptr; }

    simulation_context_ptr  context() { return m_context; }
    simulation_context_const_ptr  context() const { return m_context; }

    simulation_configuration&  simulation_config() { return m_simulation_config; }
    simulation_configuration const&  simulation_config() const { return m_simulation_config; }

    render_configuration&  render_config() { return m_render_config; }
    render_configuration const&  render_config() const { return m_render_config; }

    natural_32_bit  FPS() const { return m_FPS; }

private:

    void  simulate();

    void  camera_update();

    void  render();
    void  render_task(std::vector<object_guid> const&  batch_guids);
    void  render_grid();
    void  render_frames();
    void  render_text();

    std::shared_ptr<angeo::collision_scene>  m_collision_scene_ptr;
    std::shared_ptr<angeo::rigid_body_simulator>  m_rigid_body_simulator_ptr;
    std::shared_ptr<ai::simulator>  m_ai_simulator_ptr;

    simulation_context_ptr  m_context;

    simulation_configuration  m_simulation_config;
    render_configuration  m_render_config;

    natural_32_bit  m_FPS_num_rounds;
    float_32_bit  m_FPS_time;
    natural_32_bit  m_FPS;

    std::pair<std::string, gfx::batch>  m_text_cache;
};


}

#endif
