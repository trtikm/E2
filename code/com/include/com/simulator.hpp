#ifndef COM_SIMULATOR_HPP_INCLUDED
#   define COM_SIMULATOR_HPP_INCLUDED

#   include <com/simulation_context.hpp>
#   include <osi/simulator.hpp>
#   include <gfx/camera.hpp>
#   include <gfx/free_fly.hpp>
#   include <angeo/tensor_math.hpp>
#   include <angeo/collision_scene.hpp>
#   include <angeo/rigid_body_simulator.hpp>
#   include <com/device_simulator.hpp>
#   include <ai/simulator.hpp>
#   include <unordered_map>
#   include <vector>
#   include <string>
#   include <utility>
#   include <memory>

namespace com {


struct  simulator : public osi::simulator
{
    struct  simulation_configuration
    {
        simulation_configuration();

        float_32_bit  MAX_SIMULATION_TIME_DELTA;
        natural_8_bit  MAX_NUM_SUB_SIMULATION_STEPS;

        float_32_bit  simulation_time_buffer;
        float_32_bit  last_time_step;

        bool  commit_state_changes_in_the_same_round;

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
        gfx::batch  batch_sensory_collision_contact;
        gfx::batch  batch_physics_collision_contact;
        gfx::batch  batch_sight_raycast_contact;
        bool  render_fps;
        bool  render_grid;
        bool  render_frames;
        bool  render_text;
        bool  render_in_wireframe;
        bool  render_scene_batches;
        bool  render_colliders_of_rigid_bodies;
        bool  render_colliders_of_fields;
        bool  render_colliders_of_sensors;
        bool  render_colliders_of_agents;
        bool  render_colliders_of_ray_casts;
        bool  render_collision_contacts;
        bool  render_sight_frustums;
        bool  render_sight_contacts;
        bool  render_sight_image;
        float_32_bit  sight_image_scale;
        vector4  colour_of_rigid_body_collider;
        vector4  colour_of_field_collider;
        vector4  colour_of_sensor_collider;
        vector4  colour_of_agent_collider;
        vector4  colour_of_ray_cast_collider;
        vector4  colour_of_collision_contact;
        vector4  colour_of_agent_perspective_frustum;
        float_32_bit  disabled_collider_colour_multiplier;
        bool  include_normals_to_batches_of_trinagle_mesh_colliders;
        bool  include_neigbour_lines_to_to_batches_of_trinagle_mesh_colliders;
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

    void  change_camera_speed(float_32_bit const  multiplier);

    void  clear_cache_of_collider_batches() { m_collider_batches_cache.clear(); }
    void  clear_cache_of_agent_sight_batches() { m_agent_sight_frustum_batches_cache.clear(); }
    void  clear(bool const  also_caches);

    void  round() override;

    virtual void  on_begin_round() {}

        virtual void  on_begin_simulation() {}
        virtual void  custom_module_round() {}
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

    std::shared_ptr<com::device_simulator>  device_simulator() { return m_device_simulator_ptr; }
    std::shared_ptr<com::device_simulator const>  device_simulator() const { return m_device_simulator_ptr; }

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

    struct  render_task_info
    {
        gfx::batch  batch;
        std::vector<object_guid>  frame_guids;
        std::vector<matrix44>  world_matrices;
    };
    using  render_tasks_map = std::unordered_map<std::string, render_task_info>;

    void  simulate();
    void  update_collision_contacts_and_constraints();
    void  commit_state_changes();
    void  update_collider_locations_of_relocated_frames();

    void  camera_update();

    void  render();
    void  render_task(render_task_info const&  task);
    void  render_grid();
    void  render_frames();
    void  render_colliders();
    void  render_collision_contacts();
    void  render_sight_frustums();
    void  render_sight_contacts();
    void  render_sight_image();
    void  render_text();

    gfx::batch  create_batch_for_collider(object_guid const  collider_guid, bool const  is_enabled);

    std::shared_ptr<angeo::collision_scene>  m_collision_scene_ptr;
    std::shared_ptr<angeo::rigid_body_simulator>  m_rigid_body_simulator_ptr;
    std::shared_ptr<com::device_simulator>  m_device_simulator_ptr;
    std::shared_ptr<ai::simulator>  m_ai_simulator_ptr;

    simulation_context_ptr  m_context;

    simulation_configuration  m_simulation_config;
    render_configuration  m_render_config;

    natural_32_bit  m_FPS_num_rounds;
    float_32_bit  m_FPS_time;
    natural_32_bit  m_FPS;

    std::pair<std::string, gfx::batch>  m_text_cache;
    using  cached_collider_batch_state = std::array<gfx::batch, 2U>;
    std::unordered_map<object_guid, cached_collider_batch_state>  m_collider_batches_cache;
    std::unordered_map<object_guid, gfx::batch>  m_agent_sight_frustum_batches_cache;
    struct  sight_image_render_data;
    std::shared_ptr<sight_image_render_data>  m_sight_image_render_data;
};


}

#endif
