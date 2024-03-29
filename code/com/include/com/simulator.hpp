#ifndef COM_SIMULATOR_HPP_INCLUDED
#   define COM_SIMULATOR_HPP_INCLUDED

#   include <com/simulation_context.hpp>
#   include <com/console.hpp>
#   include <osi/simulator.hpp>
#   include <gfx/camera.hpp>
#   include <gfx/free_fly.hpp>
#   include <gfx/gui/text_box.hpp>
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

        bool  paused;
        natural_32_bit  num_rounds_to_pause;
    };

    enum struct  VIEWPORT_TYPE { SCENE = 0, CONSOLE = 1, OUTPUT = 2 };

    enum struct CAMERA_CONTROLLER_TYPE : natural_8_bit { CAMERA_IS_LOCKED, FREE_FLY, CUSTOM_CONTROL };

    struct  render_configuration
    {
        render_configuration(gfx::viewport const&  vp, std::string const&  data_root_dir);
        void  terminate();
        // Global config - fields are only initialised in the constructor and then never changed in this class.
        //                 Feel free to modify these field, ideally in the callback 'on_begin_round()'
        gfx::free_fly_config  free_fly_config;
        CAMERA_CONTROLLER_TYPE  camera_controller_type;
        std::shared_ptr<gfx::font_mono_props>  font_props;
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
        gfx::batch  batch_sight_raycast_contact_directed;
        gfx::batch  batch_sight_raycast_contact_random;
        bool  render_fps;
        bool  render_grid;
        bool  render_frames;
        bool  render_skeleton_frames;
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
        bool  render_sight_contacts_directed;
        bool  render_sight_contacts_random;
        bool  render_sight_image;
        bool  render_agent_action_transition_contratints;
        bool  render_ai_navigation_data;
        bool  show_console;
        float_32_bit  console_viewport_left_param; // A parameter in (0,1) to the left side of the console viewport
                                                   // from the left side of the whole app window.
        bool  show_output;
        float_32_bit  output_viewport_top_param; // A parameter in (0,1) to the top side of the output viewport
                                                 // from the top side of the whole app window.
        float_32_bit  sight_image_scale;
        vector4  colour_of_rigid_body_collider;
        vector4  colour_of_field_collider;
        vector4  colour_of_sensor_collider;
        vector4  colour_of_agent_collider;
        vector4  colour_of_ray_cast_collider;
        vector4  colour_of_collision_contact;
        vector4  colour_of_agent_perspective_frustum;
        vector4  colour_of_agent_action_transition_contratints;
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
    void  clear_cache_of_ai_debug_render() { m_ai_debug_draw_data.clear(); }
    void  clear(bool const  also_caches);

    void  round() override;

    virtual void  on_begin_round() {}

        virtual void  on_begin_simulation() {}
        virtual void  custom_module_round() {}
        virtual void  on_end_simulation() {}

        virtual void  on_begin_camera_update() {}
        virtual void  custom_camera_update() {}
        virtual void  on_end_camera_update() {}

        virtual void  on_begin_render() {}
        virtual bool  do_render_batch(object_guid const  batch_guid) const { return true; }
        virtual void  custom_render() {}
        virtual void  on_end_render() {}

    virtual void  on_end_round() {}

    //std::shared_ptr<angeo::collision_scene>  collision_scene() { return m_collision_scenes_ptr->front(); }
    //std::shared_ptr<angeo::collision_scene const>  collision_scene() const { return m_collision_scenes_ptr->front(); }

    std::shared_ptr<angeo::collision_scene>  collision_scene(natural_8_bit const  idx) { return m_collision_scenes_ptr->at(idx); }
    std::shared_ptr<angeo::collision_scene const>  collision_scene(natural_8_bit const  idx) const { return m_collision_scenes_ptr->at(idx); }

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

    VIEWPORT_TYPE  active_viewport_type() const { return m_active_viewport; }
    std::shared_ptr<gfx::viewport const>  get_viewport_ptr(VIEWPORT_TYPE const  vp_type) const { return m_viewports.at((std::size_t)vp_type); }
    gfx::viewport const&  get_viewport(VIEWPORT_TYPE const  vp_type) const { return *get_viewport_ptr(vp_type); }

    void  render_batch(gfx::batch  batch, std::vector<matrix44> const&  world_matrices);

    object_guid  find_collider_under_mouse() const;
    std::string  paste_object_path_to_command_line_of_console(
            object_guid  guid,
            bool const  prefer_owner_guid = false,
            bool const  replace_text_after_cursor = false,
            bool const  move_cursor = true
            );

private:

    struct  render_task_info
    {
        gfx::batch  batch;
        std::vector<matrix44>  world_matrices;
    };
    using  render_tasks_map = std::unordered_map<std::string, render_task_info>;

    void  simulate();
    void  update_collision_contacts_and_constraints();
    void  update_collider_locations_of_relocated_frames();

    void  update_viewports();
    gfx::viewport&  viewport_ref(VIEWPORT_TYPE const  vp_type) { return *m_viewports.at((std::size_t)vp_type); }
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
    void  render_agent_action_transition_contratints();
    void  render_ai_navigation_data();
    void  render_text();

    gfx::batch  create_batch_for_collider(object_guid const  collider_guid, bool const  is_enabled);

    void  update_console();
    void  render_console();

    std::shared_ptr<std::vector<std::shared_ptr<angeo::collision_scene> > >  m_collision_scenes_ptr;
    std::shared_ptr<angeo::rigid_body_simulator>  m_rigid_body_simulator_ptr;
    std::shared_ptr<com::device_simulator>  m_device_simulator_ptr;
    std::shared_ptr<ai::simulator>  m_ai_simulator_ptr;

    simulation_context_ptr  m_context;

    std::vector<std::shared_ptr<gfx::viewport> >  m_viewports;
    VIEWPORT_TYPE  m_active_viewport;

    simulation_configuration  m_simulation_config;
    render_configuration  m_render_config;

    struct  console_props {
        console_props(
                std::shared_ptr<gfx::font_mono_props> const  font,
                std::shared_ptr<gfx::viewport> const  viewport
                )
            : console()
            , text_box(font, viewport)
        {
            text_box.set_text(console.text());
            text_box.set_character_escaping(false);
        }
        console  console;
        gfx::gui::text_box  text_box;
    }  m_console;

    gfx::gui::text_box  m_output_text_box;

    natural_32_bit  m_FPS_num_rounds;
    float_32_bit  m_FPS_time;
    natural_32_bit  m_FPS;

    struct  text_cache {
        bool  operator==(text_cache const&  other) const { return width == other.width && text == other.text && scale == other.scale; }
        bool  operator!=(text_cache const&  other) const { return !(*this == other); }
        std::string  text;
        gfx::batch  batch;
        float_32_bit  width;
        float_32_bit  scale;
    } m_text_cache;

    //////////////////////////////////////////////////////////////////////////////////////////
    // DEBUG DRAW DATA OF LIBRARIES
    //////////////////////////////////////////////////////////////////////////////////////////

    using  cached_collider_batch_state = std::array<gfx::batch, 2U>;
    std::unordered_map<object_guid, cached_collider_batch_state>  m_collider_batches_cache;

    struct  ai_debug_draw_data
    {
        ai_debug_draw_data();
        void  clear();
        void  on_navcomponent_updated(ai::navobj_guid const  component_guid);

        std::unordered_map<object_guid, gfx::batch>  m_agent_sight_frustum_batches_cache;
        std::unordered_map<std::string, gfx::batch>  m_agent_action_transition_contratints_cache;
        struct  sight_image_render_data;
        std::shared_ptr<sight_image_render_data>  m_sight_image_render_data;
        gfx::batch  m_waypoint2d_static_batch;
        gfx::batch  m_waypoint2d_dynamic_batch;
        gfx::batch  m_waypoint3d_static_batch;
        gfx::batch  m_waypoint3d_dynamic_batch;
        std::unordered_map<ai::navobj_guid, gfx::batch>  m_navcomponent_waylink_batches_cache;
        gfx::batch  m_navlinks_batch;
    } m_ai_debug_draw_data;

};


}

#endif
