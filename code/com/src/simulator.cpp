#include <com/simulator.hpp>
#include <gfx/draw.hpp>
#include <gfx/image.hpp>
#include <ai/cortex_mock.hpp>
#include <ai/sight_controller.hpp>
#include <osi/opengl.hpp>
#include <angeo/utility.hpp>
#include <utility/canonical_path.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>

namespace com { namespace detail {


inline bool  should_compute_contact_response_for_rigid_body_guids(object_guid const  rb_1_guid, object_guid const  rb_2_guid)
{
    return rb_1_guid != rb_2_guid && rb_1_guid != invalid_object_guid() && rb_2_guid != invalid_object_guid();
}


}}

namespace com {




simulator::simulation_configuration::simulation_configuration()
    : MAX_SIMULATION_TIME_DELTA(1.0f / 30.0f)
    , MAX_NUM_SUB_SIMULATION_STEPS(1U)

    , simulation_time_buffer(0.0f)
    , last_time_step(0.0f)

    , commit_state_changes_in_the_same_round(false)

    , paused(true)
    , num_rounds_to_pause(0U)
{}


simulator::render_configuration::render_configuration(osi::window_props const&  wnd_props, std::string const&  data_root_dir)
    // Global config
    : free_fly_config(gfx::default_free_fly_config(wnd_props.pixel_width_mm(), wnd_props.pixel_height_mm()))
    , font_props(
            [&data_root_dir]() -> gfx::font_mono_props {
                gfx::font_mono_props  props;
                gfx::load_font_mono_props(canonical_path(data_root_dir) / "shared" / "gfx" / "fonts" / "Liberation_Mono.txt", props);
                return props;
            }()        
            )
    , clear_colour{ 0.25f, 0.25f, 0.25f }
    , diffuse_colour{ 1.0f, 1.0f, 1.0f, 1.0f }
    , ambient_colour{ 0.5f, 0.5f, 0.5f }
    , specular_colour{ 1.0f, 1.0f, 1.0f, 2.0f }
    , directional_light_direction(normalised(-vector3(2.0f, 1.0f, 3.0f)))
    , directional_light_colour{ 1.0f, 1.0f, 1.0f }
    , fog_colour{ 0.25f, 0.25f, 0.25f, 2.0f }
    , fog_near(0.25f)
    , fog_far(1000.0f)
    , text_scale(1.0f)
    , text_shift{ 0.0f, -1.0f, 0.0f }
    , text_ambient_colour{ 1.0f, 0.0f, 1.0f }
    , fps_prefix("FPS:")
    , batch_grid(gfx::create_default_grid())
    , batch_frame(gfx::create_basis_vectors())
    , batch_sensory_collision_contact(gfx::create_arrow(0.1f, { 1.0f, 1.0f, 0.0f, 1.0f }))
    , batch_physics_collision_contact(gfx::create_arrow(0.1f, { 1.0f, 0.0f, 0.0f, 1.0f }))
    , batch_sight_raycast_contact(gfx::create_coord_cross(0.025f, { 0.5f, 0.7f, 1.0f, 1.0f }))
    , render_fps(true)
    , render_grid(true)
    , render_frames(false)
    , render_text(true)
    , render_in_wireframe(false)
    , render_scene_batches(true)
    , render_colliders_of_rigid_bodies(true)
    , render_colliders_of_fields(true)
    , render_colliders_of_sensors(true)
    , render_colliders_of_agents(true)
    , render_colliders_of_ray_casts(true)
    , render_collision_contacts(true)
    , render_sight_frustums(true)
    , render_sight_contacts(true)
    , render_sight_image(false)
    , sight_image_scale(5.0f)
    , colour_of_rigid_body_collider{ 0.75f, 0.75f, 1.0f, 1.0f }
    , colour_of_field_collider{ 1.0f, 0.5f, 0.25f, 1.0f }
    , colour_of_sensor_collider{ 0.0f, 0.85f, 0.85f, 1.0f }
    , colour_of_agent_collider{ 0.75f, 0.75f, 1.0f, 1.0f }
    , colour_of_ray_cast_collider{ 0.75f, 0.75f, 1.0f, 1.0f }
    , colour_of_collision_contact{ 1.0f, 0.5f, 0.5f, 1.0f }
    , colour_of_agent_perspective_frustum{0.5f, 0.5f, 0.5f, 1.0f}
    , disabled_collider_colour_multiplier(0.5f)
    , include_normals_to_batches_of_trinagle_mesh_colliders(true)
    , include_neigbour_lines_to_to_batches_of_trinagle_mesh_colliders(false)
    // Current round config
    , camera(
        gfx::camera_perspective::create(
                angeo::coordinate_system::create(
                        vector3(10.0f, 10.0f, 4.0f),
                        quaternion(0.293152988f, 0.245984003f, 0.593858004f, 0.707732975f)
                        ),
                0.25f,
                500.0f,
                wnd_props
                )
        )
    , matrix_from_world_to_camera()
    , matrix_from_camera_to_clipspace()
    , matrix_ortho_projection_for_text()
    , directional_light_direction_in_camera_space()
    , draw_state()
{
    camera->to_camera_space_matrix(matrix_from_world_to_camera);
    camera->projection_matrix(matrix_from_camera_to_clipspace);
    camera->projection_matrix_orthogonal(matrix_ortho_projection_for_text);
    directional_light_direction_in_camera_space = transform_vector(directional_light_direction, matrix_from_world_to_camera);
}


void  simulator::render_configuration::terminate()
{
    font_props.release();
    batch_grid.release();
    batch_frame.release();
    batch_sensory_collision_contact.release();
    batch_physics_collision_contact.release();
    draw_state.release();
}


struct  simulator::sight_image_render_data
{
    gfx::image_rgba_8888  img;
    gfx::batch  batch;
};


simulator::simulator(std::string const&  data_root_dir)
    : m_collision_scene_ptr(std::make_shared<angeo::collision_scene>())
    , m_rigid_body_simulator_ptr(std::make_shared<angeo::rigid_body_simulator>())
    , m_device_simulator_ptr(std::make_shared<com::device_simulator>())
    , m_ai_simulator_ptr(std::make_shared<ai::simulator>())

    , m_context(simulation_context::create(
            m_collision_scene_ptr,
            m_rigid_body_simulator_ptr,
            m_device_simulator_ptr,
            m_ai_simulator_ptr,
            data_root_dir
            ))

    , m_simulation_config()
    , m_render_config(get_window_props(), m_context->get_data_root_dir())

    , m_FPS_num_rounds(0U)
    , m_FPS_time(0.0f)
    , m_FPS(0U)

    , m_text_cache()
    , m_collider_batches_cache()
    , m_agent_sight_frustum_batches_cache()
    , m_sight_image_render_data(nullptr)
{}


simulator::~simulator()
{
    m_ai_simulator_ptr.reset();
    m_device_simulator_ptr.reset();
    m_rigid_body_simulator_ptr.reset();
    m_collision_scene_ptr.reset();

    m_context.reset();
}


void  simulator::initialise()
{
    osi::simulator::initialise();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDisable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glDepthRangef(0.0f,1.0f);
}


void  simulator::terminate()
{
    context()->clear(true);
    render_config().terminate();
    osi::simulator::terminate();
}


void  simulator::change_camera_speed(float_32_bit const  multiplier)
{
    ASSUMPTION(multiplier > 0.0f);
    for (gfx::free_fly_action&  action : render_config().free_fly_config)
        if (action.do_rotation() == false
                && std::fabs(action.action_value() * multiplier) > 0.001f
                && std::fabs(action.action_value() * multiplier) < 1000.0f)
            action.set_action_value(action.action_value() * multiplier);
}


void  simulator::clear(bool const  also_caches)
{
    context()->clear(also_caches);
    clear_cache_of_collider_batches();
    clear_cache_of_agent_sight_batches();
    m_sight_image_render_data = nullptr;
}


void  simulator::round()
{
    TMPROF_BLOCK();

    screen_text_logger::instance().clear();

    on_begin_round();

        ++m_FPS_num_rounds;
        m_FPS_time += round_seconds();
        if (m_FPS_time >= 0.25L)
        {
            m_FPS = 4U * m_FPS_num_rounds;
            m_FPS_num_rounds = 0U;
            m_FPS_time -= 0.25L;
        }
        if (render_config().render_text && render_config().render_fps)
            SLOG(render_config().fps_prefix << FPS() << "\n");

        on_begin_simulation();
            context()->process_pending_late_requests();
            if (!simulation_config().paused)
            {
                simulate();

                if (simulation_config().num_rounds_to_pause > 0U)
                {
                    --simulation_config().num_rounds_to_pause;
                    if (simulation_config().num_rounds_to_pause == 0U)
                        simulation_config().paused = true;
                }
            }
            else { SLOG("PAUSED\n"); }
        on_end_simulation();

        on_begin_camera_update();
            camera_update();
        on_end_camera_update();

        on_begin_render();
            render();
        on_end_render();

    on_end_round();
}


void  simulator::simulate()
{
    TMPROF_BLOCK();

    simulation_config().simulation_time_buffer =
        std::min(simulation_config().simulation_time_buffer + round_seconds(),
                 (float_32_bit)simulation_config().MAX_NUM_SUB_SIMULATION_STEPS * simulation_config().MAX_SIMULATION_TIME_DELTA);

    do
    {
        if (!simulation_config().commit_state_changes_in_the_same_round)
            commit_state_changes();

        simulation_config().last_time_step = std::min(simulation_config().simulation_time_buffer,
                                                      simulation_config().MAX_SIMULATION_TIME_DELTA);
        simulation_config().simulation_time_buffer -= simulation_config().last_time_step;

        update_collision_contacts_and_constraints();

        device_simulator()->next_round((simulation_context const&)*context(), simulation_config().last_time_step);
        ai_simulator()->next_round(simulation_config().last_time_step, get_keyboard_props(), get_mouse_props(), get_window_props());
        custom_module_round();

        if (simulation_config().commit_state_changes_in_the_same_round)
            commit_state_changes();
    }
    while (simulation_config().simulation_time_buffer >= simulation_config().MAX_SIMULATION_TIME_DELTA);
}


void  simulator::update_collision_contacts_and_constraints()
{
    TMPROF_BLOCK();

    simulation_context&  ctx = *context();

    ctx.clear_collision_contacts();
    collision_scene()->compute_contacts_of_all_dynamic_objects(
            [this, &ctx](
                angeo::contact_id const&  cid,
                vector3 const&  contact_point,
                vector3 const&  unit_normal,
                float_32_bit  penetration_depth) -> bool {
                    angeo::collision_object_id const  coid_1 = angeo::get_object_id(angeo::get_first_collider_id(cid));
                    angeo::collision_object_id const  coid_2 = angeo::get_object_id(angeo::get_second_collider_id(cid));

                    object_guid const  collider_1_guid = ctx.to_collider_guid(coid_1);
                    object_guid const  collider_2_guid = ctx.to_collider_guid(coid_2);

                    ctx.insert_collision_contact({ collider_1_guid, collider_2_guid, contact_point, unit_normal, penetration_depth });

                    object_guid const  rb_1_guid = ctx.rigid_body_of_collider(collider_1_guid);
                    object_guid const  rb_2_guid = ctx.rigid_body_of_collider(collider_2_guid);

                    if (!detail::should_compute_contact_response_for_rigid_body_guids(rb_1_guid, rb_2_guid))
                        return true;

                    INVARIANT(ctx.is_rigid_body_moveable(rb_1_guid) || ctx.is_rigid_body_moveable(rb_2_guid));

                    angeo::COLLISION_MATERIAL_TYPE const  material_1 = ctx.collision_material_of(collider_1_guid);
                    angeo::COLLISION_MATERIAL_TYPE const  material_2 = ctx.collision_material_of(collider_2_guid);

                    bool const  use_friction =
                            material_1 != angeo::COLLISION_MATERIAL_TYPE::NO_FRINCTION_NO_BOUNCING &&
                            material_2 != angeo::COLLISION_MATERIAL_TYPE::NO_FRINCTION_NO_BOUNCING ;
                    angeo::rigid_body_simulator::contact_friction_constraints_info  friction_info;
                    if (use_friction)
                    {
                        friction_info.m_unit_tangent_plane_vectors.resize(2UL);
                        angeo::compute_tangent_space_of_unit_vector(
                                unit_normal,
                                friction_info.m_unit_tangent_plane_vectors.front(),
                                friction_info.m_unit_tangent_plane_vectors.back()
                                );
                        friction_info.m_suppress_negative_directions = false;
                        friction_info.m_max_tangent_relative_speed_for_static_friction = 0.001f;
                    }
                    std::vector<angeo::motion_constraint_system::constraint_id>  output_constraint_ids;
                    m_rigid_body_simulator_ptr->insert_contact_constraints(
                            ctx.from_rigid_body_guid(rb_1_guid),
                            ctx.from_rigid_body_guid(rb_2_guid),
                            cid,
                            contact_point,
                            unit_normal,
                            material_1,
                            material_2,
                            use_friction ? &friction_info : nullptr,
                            penetration_depth,
                            20.0f
                            );

                    return true;
                },
            true
            );
}


void  simulator::commit_state_changes()
{
    TMPROF_BLOCK();

    simulation_context&  ctx = *context();

    ctx.clear_invalidated_guids();

    ctx.process_rigid_bodies_with_invalidated_shape();
    ctx.process_pending_early_requests();

    rigid_body_simulator()->solve_constraint_system(simulation_config().last_time_step, simulation_config().last_time_step * 0.75f);
    rigid_body_simulator()->integrate_motion_of_rigid_bodies(simulation_config().last_time_step);
    rigid_body_simulator()->prepare_contact_cache_and_constraint_system_for_next_frame();

    for (auto  rb_it = ctx.moveable_rigid_bodies_begin(), rb_end = ctx.moveable_rigid_bodies_end(); rb_it != rb_end; ++rb_it)
        ctx.frame_relocate_relative_to_parent(
                ctx.frame_of_rigid_body(*rb_it),
                ctx.mass_centre_of_rigid_body(*rb_it),
                ctx.orientation_of_rigid_body(*rb_it)
                );

    for (auto  col_it = ctx.moveable_colliders_begin(), col_end = ctx.moveable_colliders_end(); col_it != col_end; ++col_it)
        ctx.relocate_collider(*col_it, ctx.frame_world_matrix(ctx.frame_of_collider(*col_it)));

    ctx.process_pending_requests();
}



void  simulator::camera_update()
{
    gfx::adjust(*render_config().camera, get_window_props());
    gfx::free_fly(*render_config().camera->coordinate_system(), render_config().free_fly_config,
                  round_seconds(), get_mouse_props(), get_keyboard_props());
    render_config().camera->to_camera_space_matrix(render_config().matrix_from_world_to_camera);
    render_config().camera->projection_matrix(render_config().matrix_from_camera_to_clipspace);
    render_config().directional_light_direction_in_camera_space =
            transform_vector(render_config().directional_light_direction, render_config().matrix_from_world_to_camera);
    render_config().camera->projection_matrix_orthogonal(render_config().matrix_ortho_projection_for_text);
}


void  simulator::render()
{
    TMPROF_BLOCK();

    simulation_context&  ctx = *context();
    render_configuration&  cfg = render_config();

    render_tasks_map  render_tasks_opaque;
    render_tasks_map  render_tasks_translucent;

    if (cfg.render_scene_batches)
        for (simulation_context::batch_guid_iterator  batch_it = ctx.batches_begin(), batch_end = ctx.batches_end();
             batch_it != batch_end; ++batch_it)
        {
            object_guid const  batch_guid = *batch_it;

            gfx::batch const  batch = ctx.from_batch_guid_to_batch(batch_guid);
            if (!batch.loaded_successfully())
                continue;

            // TODO: insert here a check whether the batch is inside camera's frustum or not.

            render_tasks_map&  tasks = batch.is_translucent() ? render_tasks_translucent : render_tasks_opaque;

            std::string const&  batch_id = ctx.from_batch_guid(batch_guid);
            auto  it = tasks.find(batch_id);
            if (it == tasks.end())
                it = tasks.insert({ batch_id, { batch, {} } }).first;

            for (object_guid  frame_guid : ctx.frames_of_batch(batch_guid))
                it->second.frame_guids.push_back(frame_guid);
        }

    // Here we start the actual rendering of batches collected above.

    glClearColor(cfg.clear_colour(0), cfg.clear_colour(1), cfg.clear_colour(2), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glViewport(0, 0, get_window_props().window_width(), get_window_props().window_height());
    glPolygonMode(GL_FRONT_AND_BACK, cfg.render_in_wireframe ? GL_LINE : GL_FILL);

    for (auto const&  id_and_task : render_tasks_opaque)
        render_task(id_and_task.second);
    for (auto const&  id_and_task : render_tasks_translucent)
        render_task(id_and_task.second);

    if (cfg.render_grid && cfg.batch_grid.loaded_successfully())
        render_grid();

    if (cfg.render_frames && cfg.batch_frame.loaded_successfully())
        render_frames();

    if (cfg.render_colliders_of_rigid_bodies
            || cfg.render_colliders_of_fields
            || cfg.render_colliders_of_sensors
            || cfg.render_colliders_of_agents
            || cfg.render_colliders_of_ray_casts
            )
        render_colliders();

    if (cfg.render_collision_contacts)
        render_collision_contacts();

    if (cfg.render_sight_frustums)
        render_sight_frustums();
    if (cfg.render_sight_contacts)
        render_sight_contacts();
    if (cfg.render_sight_image)
        render_sight_image();

    if (cfg.render_text)
        render_text();
}


void  simulator::render_task(render_task_info const&  task)
{
    if (task.frame_guids.empty() && task.world_matrices.empty())
        return;

    simulation_context&  ctx = *context();
    render_configuration&  cfg = render_config();

    bool const  use_instancing =
            task.frame_guids.size() + task.world_matrices.size() > 1UL &&
            !task.batch.is_attached_to_skeleton() &&
            task.batch.has_instancing_data()
            ;

    if (!gfx::make_current(task.batch, cfg.draw_state, use_instancing))
        return;

    gfx::fragment_shader_uniform_data_provider const  fs_uniform_data_provider(
            cfg.diffuse_colour,
            cfg.ambient_colour,
            cfg.specular_colour,
            cfg.directional_light_direction_in_camera_space,
            cfg.directional_light_colour,
            cfg.fog_colour,
            cfg.fog_near,
            cfg.fog_far
            );

    if (use_instancing)
    {
        gfx::vertex_shader_instanced_data_provider  instanced_data_provider(task.batch);
        for (object_guid  frame_guid : task.frame_guids)
            instanced_data_provider.insert_from_model_to_camera_matrix(
                    cfg.matrix_from_world_to_camera * ctx.frame_world_matrix(frame_guid)
                    );
        for (matrix44 const&  world_matrix : task.world_matrices)
            instanced_data_provider.insert_from_model_to_camera_matrix(cfg.matrix_from_world_to_camera * world_matrix);
        gfx::render_batch(
                task.batch,
                instanced_data_provider,
                gfx::vertex_shader_uniform_data_provider(
                        task.batch,
                        {},
                        cfg.matrix_from_camera_to_clipspace,
                        cfg.diffuse_colour,
                        cfg.ambient_colour,
                        cfg.specular_colour,
                        cfg.directional_light_direction_in_camera_space,
                        cfg.directional_light_colour,
                        cfg.fog_colour,
                        cfg.fog_near,
                        cfg.fog_far
                        ),
                fs_uniform_data_provider
                );    
        cfg.draw_state = task.batch.get_draw_state();
    }
    else if (task.batch.is_attached_to_skeleton())
    {
        std::vector<matrix44> const&  to_bone_matrices = ctx.matrices_to_pose_bones_of_batch(ctx.to_batch_guid(task.batch));
        INVARIANT(
                !to_bone_matrices.empty() &&
                task.frame_guids.empty() != task.world_matrices.empty() &&
                task.frame_guids.size() + task.world_matrices.size() >= to_bone_matrices.size() &&
                (task.frame_guids.size() + task.world_matrices.size()) % to_bone_matrices.size() == 0U
                );
        std::vector<matrix44>  frames_of_bones;
        frames_of_bones.reserve(to_bone_matrices.size());
        natural_32_bit const  n = (natural_32_bit)task.frame_guids.size();
        natural_32_bit const  m = (natural_32_bit)to_bone_matrices.size();
        for (natural_32_bit  i = 0U; i != n; )
        {
            frames_of_bones.clear();
            for (natural_32_bit  j = 0U; j != m; ++j, ++i)
                frames_of_bones.push_back(
                        cfg.matrix_from_world_to_camera *
                        (task.frame_guids.empty() ? task.world_matrices.at(i) :
                                                    ctx.frame_world_matrix(task.frame_guids.at(i))) *
                        to_bone_matrices.at(j)
                        );
            gfx::render_batch(
                    task.batch,
                    gfx::vertex_shader_uniform_data_provider(
                            task.batch,
                            frames_of_bones,
                            cfg.matrix_from_camera_to_clipspace,
                            cfg.diffuse_colour,
                            cfg.ambient_colour,
                            cfg.specular_colour,
                            cfg.directional_light_direction_in_camera_space,
                            cfg.directional_light_colour,
                            cfg.fog_colour,
                            cfg.fog_near,
                            cfg.fog_far
                            ),
                    fs_uniform_data_provider
                    );
            cfg.draw_state = task.batch.get_draw_state();
        }
    }
    else
    {
        auto const  render = [this, &task, &cfg, &fs_uniform_data_provider](matrix44 const&  world_matrix) {
            gfx::render_batch(
                    task.batch,
                    gfx::vertex_shader_uniform_data_provider(
                            task.batch,
                            { cfg.matrix_from_world_to_camera * world_matrix },
                            cfg.matrix_from_camera_to_clipspace,
                            cfg.diffuse_colour,
                            cfg.ambient_colour,
                            cfg.specular_colour,
                            cfg.directional_light_direction_in_camera_space,
                            cfg.directional_light_colour,
                            cfg.fog_colour,
                            cfg.fog_near,
                            cfg.fog_far
                            ),
                    fs_uniform_data_provider
                    );
            cfg.draw_state = task.batch.get_draw_state();
        };
        for (object_guid  frame_guid : task.frame_guids)
            render(ctx.frame_world_matrix(frame_guid));
        for (matrix44 const&  world_matrix : task.world_matrices)
            render(world_matrix);
    }
}


void  simulator::render_grid()
{
    simulation_context&  ctx = *context();
    render_configuration&  cfg = render_config();

    if (!gfx::make_current(cfg.batch_grid, cfg.draw_state))
        return;

    gfx::render_batch(
        cfg.batch_grid,
        gfx::vertex_shader_uniform_data_provider(
            cfg.batch_grid,
            { cfg.matrix_from_world_to_camera },
            cfg.matrix_from_camera_to_clipspace,
            cfg.diffuse_colour,
            cfg.ambient_colour,
            cfg.specular_colour,
            cfg.directional_light_direction_in_camera_space,
            cfg.directional_light_colour,
            cfg.fog_colour,
            cfg.fog_near,
            cfg.fog_far
            )
        );

    cfg.draw_state = cfg.batch_grid.get_draw_state();
}


void  simulator::render_frames()
{
    simulation_context&  ctx = *context();

    if (ctx.frames_begin() == ctx.frames_end())
        return;

    render_configuration&  cfg = render_config();

    if (!gfx::make_current(cfg.batch_frame, cfg.draw_state, true))
        return;

    gfx::vertex_shader_instanced_data_provider  instanced_data_provider(cfg.batch_frame);
    for (simulation_context::frame_guid_iterator  frame_it = ctx.frames_begin(), frame_end = ctx.frames_end();
         frame_it != frame_end; ++frame_it)
        instanced_data_provider.insert_from_model_to_camera_matrix(
                cfg.matrix_from_world_to_camera * ctx.frame_world_matrix(*frame_it)
                );
    gfx::render_batch(
        cfg.batch_frame,
        instanced_data_provider,
        gfx::vertex_shader_uniform_data_provider(
            cfg.batch_frame,
            {},
            cfg.matrix_from_camera_to_clipspace,
            cfg.diffuse_colour,
            cfg.ambient_colour,
            cfg.specular_colour,
            cfg.directional_light_direction_in_camera_space,
            cfg.directional_light_colour,
            cfg.fog_colour,
            cfg.fog_near,
            cfg.fog_far
            )
        );

    cfg.draw_state = cfg.batch_frame.get_draw_state();
}


void  simulator::render_colliders()
{
    std::unordered_map<object_guid, cached_collider_batch_state>  collider_batches_cache;

    simulation_context&  ctx = *context();
    render_configuration&  cfg = render_config();

    render_tasks_map  tasks;
    for (simulation_context::collider_guid_iterator  collider_it = ctx.colliders_begin(), collider_end = ctx.colliders_end();
         collider_it != collider_end; ++collider_it)
    {
        object_guid const  collider_guid = *collider_it;

        std::unordered_map<object_guid, cached_collider_batch_state>::iterator  batch_it;
        {
            batch_it = m_collider_batches_cache.find(collider_guid);
            if (batch_it == m_collider_batches_cache.end())
                batch_it = collider_batches_cache.end();
            else
            {
                if (ctx.invalidated_guids().count(collider_guid) != 0UL)
                {
                    m_collider_batches_cache.erase(batch_it);
                    batch_it = collider_batches_cache.end();
                }
                else
                    batch_it = collider_batches_cache.insert(*batch_it).first;
            }
        }

        switch (ctx.collision_class_of(collider_guid))
        {
        case angeo::COLLISION_CLASS::STATIC_OBJECT:
        case angeo::COLLISION_CLASS::COMMON_MOVEABLE_OBJECT:
        case angeo::COLLISION_CLASS::HEAVY_MOVEABLE_OBJECT:
            if (!cfg.render_colliders_of_rigid_bodies)
                continue;
            break;
        case angeo::COLLISION_CLASS::AGENT_MOTION_OBJECT:
            if (!cfg.render_colliders_of_agents)
                continue;
            break;
        case angeo::COLLISION_CLASS::FIELD_AREA:
            if (!cfg.render_colliders_of_fields)
                continue;
            break;
        case angeo::COLLISION_CLASS::SENSOR_DEDICATED:
        case angeo::COLLISION_CLASS::SENSOR_WIDE_RANGE:
        case angeo::COLLISION_CLASS::SENSOR_NARROW_RANGE:
            if (!cfg.render_colliders_of_sensors)
                continue;
            break;
        case angeo::COLLISION_CLASS::RAY_CAST_TARGET:
            if (!cfg.render_colliders_of_ray_casts)
                continue;
            break;
        default: UNREACHABLE(); break;
        }

        gfx::batch  batch;
        {
            bool const  is_enabled = ctx.is_collider_enabled(collider_guid);
            natural_32_bit const  batch_index = is_enabled ? 0U : 1U;
            if (batch_it == collider_batches_cache.end() || batch_it->second.at(batch_index).empty())
            {
                batch = create_batch_for_collider(collider_guid, is_enabled);
                collider_batches_cache[collider_guid].at(batch_index) = batch;
            }
            else
                batch = batch_it->second.at(batch_index);
        }

        std::string const  batch_id = batch.uid();
        auto  it = tasks.find(batch_id);
        if (it == tasks.end())
            it = tasks.insert({ batch_id, { batch, {} } }).first;
        it->second.frame_guids.push_back(ctx.frame_of_collider(collider_guid));
    }

    m_collider_batches_cache.swap(collider_batches_cache);

    // And here we do the actual rendering of prepared render tasks.

    GLint  backup_polygon_mode[2];
    glGetIntegerv(GL_POLYGON_MODE, &backup_polygon_mode[0]);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    for (auto const&  id_and_task : tasks)
        render_task(id_and_task.second);

    glPolygonMode(GL_FRONT_AND_BACK, backup_polygon_mode[0]);
}


void  simulator::render_collision_contacts()
{
    simulation_context&  ctx = *context();
    render_task_info  sensory_task{ render_config().batch_sensory_collision_contact, {}, {} };
    render_task_info  physics_task{ render_config().batch_physics_collision_contact, {}, {} };
    sensory_task.world_matrices.reserve(ctx.num_collision_contacts());
    physics_task.world_matrices.reserve(ctx.num_collision_contacts());
    for (simulation_context::collision_contacts_iterator  it = ctx.collision_contacts_begin(), end = ctx.collision_contacts_end();
            it != end; ++it)
    {
        collision_contact const&  cc = *it;

        render_task_info*  task_ptr;
        {
            object_guid const  rb_1_guid = ctx.rigid_body_of_collider(cc.first_collider());
            object_guid const  rb_2_guid = ctx.rigid_body_of_collider(cc.second_collider());
            task_ptr = detail::should_compute_contact_response_for_rigid_body_guids(rb_1_guid, rb_2_guid) ?
                            &physics_task : & sensory_task;
        }

        vector3  X, Y, Z = cc.unit_normal(cc.second_collider());
        angeo::compute_tangent_space_of_unit_vector(Z, X, Y);
        task_ptr->world_matrices.push_back({});
        compose_from_base_matrix(cc.contact_point(), X, Y, Z, task_ptr->world_matrices.back());
    }
    render_task(sensory_task);
    render_task(physics_task);
}


void  simulator::render_sight_frustums()
{
    simulation_context&  ctx = *context();

    render_tasks_map  m_frustum_tasks;
    for (simulation_context::agent_guid_iterator  agent_it = ctx.agents_begin(), end = ctx.agents_end(); agent_it != end; ++agent_it)
    {
        ai::sight_controller const&  sight = m_ai_simulator_ptr->get_agent(ctx.from_agent_guid(*agent_it)).get_sight_controller();
        gfx::camera_perspective_ptr  camera_ptr = sight.get_camera();
        if (camera_ptr == nullptr)
            continue;

        auto  frustum_it = m_agent_sight_frustum_batches_cache.find(*agent_it);
        if (frustum_it == m_agent_sight_frustum_batches_cache.end())
            frustum_it = m_agent_sight_frustum_batches_cache.insert({
                    *agent_it,
                    gfx::create_wireframe_perspective_frustum(
                                        -camera_ptr->near_plane(),
                                        -camera_ptr->far_plane(),
                                        camera_ptr->left(),
                                        camera_ptr->right(),
                                        camera_ptr->top(),
                                        camera_ptr->bottom(),
                                        render_config().colour_of_agent_perspective_frustum,
                                        true
                                        )
                    }).first;

        matrix44  W;
        angeo::from_base_matrix(*camera_ptr->coordinate_system(), W);
        render_task_info&  frustum_task = m_frustum_tasks[frustum_it->second.uid()];
        if (frustum_task.batch.empty())
            frustum_task.batch = frustum_it->second;
        frustum_task.world_matrices.push_back(W);
    }
    for (auto const&  key_and_task : m_frustum_tasks)
        render_task(key_and_task.second);
}


void  simulator::render_sight_contacts()
{
    simulation_context&  ctx = *context();

    render_task_info  task_sight_contacts{ render_config().batch_sight_raycast_contact, {}, {} };
    for (simulation_context::agent_guid_iterator  agent_it = ctx.agents_begin(), end = ctx.agents_end(); agent_it != end; ++agent_it)
    {
        ai::sight_controller const&  sight = m_ai_simulator_ptr->get_agent(ctx.from_agent_guid(*agent_it)).get_sight_controller();
        gfx::camera_perspective_ptr  camera = sight.get_camera();
        if (camera == nullptr)
            continue;

        ai::sight_controller::ray_casts_in_time const&  ray_casts = sight.get_ray_casts_in_time();
        task_sight_contacts.world_matrices.reserve(task_sight_contacts.world_matrices.size() + ray_casts.size());
        for (auto  ray_it = ray_casts.begin(), end = ray_casts.end(); ray_it != end; ++ray_it)
        {
            vector3 const  contact_point =
                    ray_it->second.ray_origin_in_world_space +
                    ray_it->second.parameter_to_coid * ray_it->second.ray_direction_in_world_space
                    ;
            matrix44  W;
            compose_from_base_matrix(contact_point, matrix33_identity(), W);
            task_sight_contacts.world_matrices.push_back(W);
        }
    }
    render_task(task_sight_contacts);
}


void  simulator::render_sight_image()
{
    simulation_context&  ctx = *context();

    ai::agent const*  mocked_agent = nullptr;
    for (simulation_context::agent_guid_iterator  agent_it = ctx.agents_begin(), end = ctx.agents_end(); agent_it != end; ++agent_it)
    {
        ai::agent const&  agent = m_ai_simulator_ptr->get_agent(ctx.from_agent_guid(*agent_it));
        if (dynamic_cast<ai::cortex_mock const*>(&agent.get_cortex()) != nullptr)
        {
            mocked_agent = &agent;
            break;
        }
    }

    if (mocked_agent == nullptr)
        return;

    ai::sight_controller const&  sight = mocked_agent->get_sight_controller();

    if (m_sight_image_render_data == nullptr)
    {
        m_sight_image_render_data = std::make_shared<sight_image_render_data>();
        m_sight_image_render_data->img.width = sight.get_ray_cast_config().num_cells_along_x_axis;
        m_sight_image_render_data->img.height = sight.get_ray_cast_config().num_cells_along_y_axis;
        natural_32_bit const  n = m_sight_image_render_data->img.width * m_sight_image_render_data->img.height * 4U;
        m_sight_image_render_data->img.data.resize(n, 255U);
        m_sight_image_render_data->batch = gfx::create_sprite(m_sight_image_render_data->img);
    }

    ai::sight_controller::ray_casts_image const&  depth_image = sight.get_depth_image();
    for (natural_32_bit  i = 0U, j = 0U, n = (natural_32_bit)depth_image.size(); i != n; ++i, ++j)
    {
        natural_8_bit const  value =
                (natural_8_bit)std::max(0U, std::min(255U,(natural_32_bit)std::roundf(255.0f * depth_image.at(i))));
        m_sight_image_render_data->img.data.at(j) = value;
        ++j;
        m_sight_image_render_data->img.data.at(j) = value;
        ++j;
        m_sight_image_render_data->img.data.at(j) = value;
        ++j;
    }

    gfx::update_sprite(m_sight_image_render_data->batch, m_sight_image_render_data->img);
    if (gfx::make_current(m_sight_image_render_data->batch, render_config().draw_state, false))
    {
        gfx::render_sprite_batch(
                m_sight_image_render_data->batch,
                5U,
                5U,
                get_window_props().window_width(),
                get_window_props().window_height(),
                std::max(0.01f, render_config().sight_image_scale)
                );
        render_config().draw_state = m_sight_image_render_data->batch.get_draw_state();
    }
}


void  simulator::render_text()
{
    std::string const&  text = screen_text_logger::instance().text();
    if (text.empty())
        return;

    render_configuration&  cfg = render_config();

    vector3 const  pos{
        cfg.camera->left() + cfg.text_scale * cfg.text_shift(0) * cfg.font_props.char_width,
        cfg.camera->top() + cfg.text_scale * cfg.text_shift(1)* cfg.font_props.char_height,
        -cfg.camera->near_plane()
    };

    if (text != m_text_cache.first)
    {
        m_text_cache.first = text;
        m_text_cache.second = gfx::create_text(text, cfg.font_props, (cfg.camera->right() - pos(0)) / cfg.text_scale);
    }

    if (gfx::make_current(m_text_cache.second, cfg.draw_state))
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        gfx::render_batch(
            m_text_cache.second,
            pos,
            cfg.text_scale,
            cfg.matrix_ortho_projection_for_text,
            cfg.text_ambient_colour
            );
        cfg.draw_state = m_text_cache.second.get_draw_state();
    }
}


gfx::batch  simulator::create_batch_for_collider(object_guid const  collider_guid, bool const  is_enabled)
{
    simulation_context&  ctx = *context();
    render_configuration&  cfg = render_config();

    vector4  colour;
    switch (ctx.collision_class_of(collider_guid))
    {
    case angeo::COLLISION_CLASS::STATIC_OBJECT:
    case angeo::COLLISION_CLASS::COMMON_MOVEABLE_OBJECT:
    case angeo::COLLISION_CLASS::HEAVY_MOVEABLE_OBJECT:
        colour = cfg.colour_of_rigid_body_collider;
        break;
    case angeo::COLLISION_CLASS::AGENT_MOTION_OBJECT:
        colour = cfg.colour_of_agent_collider;
        break;
    case angeo::COLLISION_CLASS::FIELD_AREA:
        colour = cfg.colour_of_field_collider;
        break;
    case angeo::COLLISION_CLASS::SENSOR_DEDICATED:
    case angeo::COLLISION_CLASS::SENSOR_WIDE_RANGE:
    case angeo::COLLISION_CLASS::SENSOR_NARROW_RANGE:
        colour = cfg.colour_of_sensor_collider;
        break;
    case angeo::COLLISION_CLASS::RAY_CAST_TARGET:
        colour = cfg.colour_of_ray_cast_collider;
        break;
    default: UNREACHABLE(); break;
    }

    if (!is_enabled)
        colour = expand34(cfg.disabled_collider_colour_multiplier * contract43(colour), 1.0f);

    switch (ctx.collider_shape_type(collider_guid))
    {
    case angeo::COLLISION_SHAPE_TYPE::BOX:
        return gfx::create_wireframe_box(ctx.collider_box_half_sizes_along_axes(collider_guid),
                                         colour, gfx::FOG_TYPE::NONE);
    case angeo::COLLISION_SHAPE_TYPE::CAPSULE:
        return gfx::create_wireframe_capsule(ctx.collider_capsule_half_distance_between_end_points(collider_guid),
                                             ctx.collider_capsule_thickness_from_central_line(collider_guid),
                                             5U, colour, gfx::FOG_TYPE::NONE);
    case angeo::COLLISION_SHAPE_TYPE::SPHERE:
        return gfx::create_wireframe_sphere(ctx.collider_sphere_radius(collider_guid), 5U, colour, gfx::FOG_TYPE::NONE);
    case angeo::COLLISION_SHAPE_TYPE::TRIANGLE:
        {
            matrix44 const  to_node_matrix = inverse44(ctx.frame_world_matrix(ctx.frame_of_collider(collider_guid)));

            std::vector<std::pair<vector3,vector3> >  lines;
            std::vector<vector4>  colours_of_lines;
            vector4 const  ignored_edge_colour = expand34(0.5f * contract43(colour), 1.0f);
            for (angeo::collision_object_id const  coid : ctx.from_collider_guid(collider_guid))
            {
                auto const&  getter = m_collision_scene_ptr->get_triangle_points_getter(coid);
                natural_32_bit const  triangle_index = m_collision_scene_ptr->get_triangle_index(coid);
                natural_8_bit const  edge_ignore_mask = m_collision_scene_ptr->get_trinagle_edges_ignore_mask(coid);

                std::array<vector3, 3U> const  P = {
                    getter(triangle_index, 0U),
                    getter(triangle_index, 1U),
                    getter(triangle_index, 2U)
                };

                lines.push_back({ P[0], P[1] });
                colours_of_lines.push_back((edge_ignore_mask & 1U) == 0U ? colour : ignored_edge_colour);
                lines.push_back({ P[1], P[2] });
                colours_of_lines.push_back((edge_ignore_mask & 2U) == 0U ? colour : ignored_edge_colour);
                lines.push_back({ P[2], P[0] });
                colours_of_lines.push_back((edge_ignore_mask & 4U) == 0U ? colour : ignored_edge_colour);

                vector3 const  center = (1.0f / 3.0f) * (P[0] + P[1] + P[2]);

                if (cfg.include_normals_to_batches_of_trinagle_mesh_colliders)
                    lines.push_back({
                            center,
                            center + 0.25f * transform_vector(m_collision_scene_ptr->get_triangle_unit_normal_in_world_space(coid),
                                                             to_node_matrix)
                            });
                colours_of_lines.push_back(colour);

                if (cfg.include_neigbour_lines_to_to_batches_of_trinagle_mesh_colliders)
                    for (natural_32_bit i = 0U; i != 3U; ++i)
                        if (m_collision_scene_ptr->get_trinagle_neighbour_over_edge(coid, i) != coid)
                        {
                            vector3 const  A = 0.5f * (P[i] + P[(i + 1U) % 3U]);
                            vector3  B;
                            if (false)
                                B = A + 0.25f * (center - A);
                            else
                            {
                                // This is only for debugging purposes (when suspicious about neighbours somewhere).
                                angeo::collision_object_id const  neighbour_coid =
                                        m_collision_scene_ptr->get_trinagle_neighbour_over_edge(coid, i);
                                auto const& neighbour_getter =
                                        m_collision_scene_ptr->get_triangle_points_getter(neighbour_coid);
                                natural_32_bit const  neighbour_triangle_index =
                                        m_collision_scene_ptr->get_triangle_index(neighbour_coid);

                                std::array<vector3, 3U> const  neighbour_P = {
                                    neighbour_getter(neighbour_triangle_index, 0U),
                                    neighbour_getter(neighbour_triangle_index, 1U),
                                    neighbour_getter(neighbour_triangle_index, 2U)
                                };
                                vector3 const  neighbour_center =
                                    (1.0f / 3.0f) * (neighbour_P[0] + neighbour_P[1] + neighbour_P[2]);
                                B = A + 0.25f * (neighbour_center - A);
                            }
                            lines.push_back({ A, B });
                            colours_of_lines.push_back(colour);
                        }
            }
            return gfx::create_lines3d(lines, colours_of_lines);
        }
    default: UNREACHABLE(); break;
    }
}


}
