#include <com/simulator.hpp>
#include <gfx/draw.hpp>
#include <osi/opengl.hpp>
#include <angeo/utility.hpp>
#include <utility/canonical_path.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>

namespace com {


simulator::render_configuration::render_configuration(osi::window_props const&  wnd_props)
    // Global config
    : free_fly_config
        {
            {
                false,
                false,
                2U,
                2U,
                -15.0f,
                gfx::free_fly_controler::keyboard_key_pressed(osi::KEY_W()),
            },
            {
                false,
                false,
                2U,
                2U,
                15.0f,
                gfx::free_fly_controler::keyboard_key_pressed(osi::KEY_S()),
            },
            {
                false,
                false,
                0U,
                2U,
                -15.0f,
                gfx::free_fly_controler::keyboard_key_pressed(osi::KEY_A()),
            },
            {
                false,
                false,
                0U,
                2U,
                15.0f,
                gfx::free_fly_controler::keyboard_key_pressed(osi::KEY_D()),
            },
            {
                false,
                false,
                1U,
                2U,
                -15.0f,
                gfx::free_fly_controler::keyboard_key_pressed(osi::KEY_Q()),
            },
            {
                false,
                false,
                1U,
                2U,
                15.0f,
                gfx::free_fly_controler::keyboard_key_pressed(osi::KEY_E()),
            },
            {
                true,
                true,
                2U,
                0U,
                -(10.0f * PI()) * (wnd_props.pixel_width_mm() / 1000.0f),
                gfx::free_fly_controler::mouse_button_pressed(osi::MIDDLE_MOUSE_BUTTON()),
            },
            {
                true,
                false,
                0U,
                1U,
                -(10.0f * PI()) * (wnd_props.pixel_height_mm() / 1000.0f),
                gfx::free_fly_controler::mouse_button_pressed(osi::MIDDLE_MOUSE_BUTTON()),
            },
        }
    , effects_config
        {
            nullptr,
            gfx::effects_config::light_types{
                gfx::LIGHT_TYPE::AMBIENT,
                gfx::LIGHT_TYPE::DIRECTIONAL,
                },
            gfx::effects_config::lighting_data_types{
                { gfx::LIGHTING_DATA_TYPE::DIRECTION, gfx::SHADER_DATA_INPUT_TYPE::UNIFORM },
                { gfx::LIGHTING_DATA_TYPE::NORMAL, gfx::SHADER_DATA_INPUT_TYPE::TEXTURE },
                { gfx::LIGHTING_DATA_TYPE::DIFFUSE, gfx::SHADER_DATA_INPUT_TYPE::TEXTURE },
                //{ gfx::LIGHTING_DATA_TYPE::DIFFUSE, gfx::SHADER_DATA_INPUT_TYPE::UNIFORM },
                { gfx::LIGHTING_DATA_TYPE::SPECULAR, gfx::SHADER_DATA_INPUT_TYPE::TEXTURE }
                },
            gfx::SHADER_PROGRAM_TYPE::VERTEX,
            gfx::effects_config::shader_output_types{
                gfx::SHADER_DATA_OUTPUT_TYPE::DEFAULT
                },
            gfx::FOG_TYPE::NONE,
            gfx::SHADER_PROGRAM_TYPE::VERTEX
        }
    , font_props(
            []() -> gfx::font_mono_props {
                gfx::font_mono_props  props;
                gfx::load_font_mono_props(
                    canonical_path("..") / "data" / "shared" / "gfx" / "fonts" / "Liberation_Mono.txt",
                    props
                    );
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
    , render_fps(true)
    , render_text(true)
    , render_in_wireframe(false)
    , render_class_common_object(true)
    , render_class_collider_of_rigid_body(true)
    , render_class_collider_of_sensor(true)
    , render_class_collider_of_activator(true)
    , render_class_collider_of_agent(true)
    , render_class_collision_contact(true)
    , render_class_ray_cast(true)
    , render_class_helper(true)
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


simulator::simulator()
    : m_collision_scene_ptr(std::make_shared<angeo::collision_scene>())
    , m_rigid_body_simulator_ptr(std::make_shared<angeo::rigid_body_simulator>())
    , m_ai_simulator_ptr()

    , m_context(simulation_context::create(m_collision_scene_ptr, m_rigid_body_simulator_ptr, m_ai_simulator_ptr))

    , m_paused(true)
    , m_render_config(get_window_props())

    , m_FPS_num_rounds(0U)
    , m_FPS_time(0.0f)
    , m_FPS(0U)

    , m_text_cache()
{}


simulator::~simulator()
{
    m_ai_simulator_ptr.reset();
    m_rigid_body_simulator_ptr.reset();
    m_collision_scene_ptr.reset();

    m_context.reset();
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
            SLOG(render_config().fps_prefix << FPS());

        on_begin_simulation();
            if (!is_paused())
                simulate();
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

    simulation_context&  ctx = *context();

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

                    object_guid const  owner_1_guid = ctx.owner_of_collider(collider_1_guid);
                    object_guid const  owner_2_guid = ctx.owner_of_collider(collider_2_guid);

                    if (owner_1_guid.kind != OBJECT_KIND::RIGID_BODY || owner_2_guid.kind != OBJECT_KIND::RIGID_BODY)
                        return true;

                    INVARIANT(ctx.is_rigid_body_moveable(owner_1_guid) || ctx.is_rigid_body_moveable(owner_2_guid));

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
                            ctx.from_rigid_body_guid(owner_1_guid),
                            ctx.from_rigid_body_guid(owner_2_guid),
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

    //ai_simulator()->next_round(round_seconds(), get_keyboard_props(), get_mouse_props(), get_window_props());

    rigid_body_simulator()->solve_constraint_system(round_seconds(), round_seconds() * 0.75f);
    rigid_body_simulator()->integrate_motion_of_rigid_bodies(round_seconds());
    rigid_body_simulator()->prepare_contact_cache_and_constraint_system_for_next_frame();

    for (auto  rb_it = ctx.moveable_rigid_bodies_begin(), rb_end = ctx.moveable_rigid_bodies_end(); rb_it != rb_end; ++rb_it)
        ctx.frame_relocate_relative_to_parent(
                ctx.frame_of_rigid_body(*rb_it),
                ctx.mass_centre_of_rigid_body(*rb_it),
                ctx.orientation_of_rigid_body(*rb_it)
                );

    for (auto  col_it = ctx.moveable_colliders_begin(), col_end = ctx.moveable_colliders_end(); col_it != col_end; ++col_it)
        ctx.relocate_collider(*col_it, ctx.frame_world_matrix(ctx.frame_of_collider(*col_it)));
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

    using  render_tasks_map = std::unordered_map<std::string, std::vector<object_guid> >;

    render_tasks_map  from_ids_to_guids_opaque;
    render_tasks_map  from_ids_to_guids_translucent;
    for (simulation_context::batch_guid_iterator  batch_it = ctx.batches_begin(), batch_end = ctx.batches_end();
         batch_it != batch_end; ++batch_it)
    {
        object_guid const  batch_guid = *batch_it;

        switch (ctx.batch_class(batch_guid))
        {
        case BATCH_CLASS::COMMON_OBJECT: if (cfg.render_class_common_object) break; else continue;
        case BATCH_CLASS::COLLIDER_OF_RIGID_BODY: if (cfg.render_class_collider_of_rigid_body) break; else continue;
        case BATCH_CLASS::COLLIDER_OF_SENSOR: if (cfg.render_class_collider_of_sensor) break; else continue;
        case BATCH_CLASS::COLLIDER_OF_ACTIVATOR: if (cfg.render_class_collider_of_activator) break; else continue;
        case BATCH_CLASS::COLLIDER_OF_AGENT: if (cfg.render_class_collider_of_agent) break; else continue;
        case BATCH_CLASS::COLLISION_CONTACT: if (cfg.render_class_collision_contact) break; else continue;
        case BATCH_CLASS::RAY_CAST: if (cfg.render_class_ray_cast) break; else continue;
        case BATCH_CLASS::HELPER: if (cfg.render_class_helper) break; else continue;
        default: UNREACHABLE(); continue;
        }

        gfx::batch const  batch = ctx.from_batch_guid_to_batch(batch_guid);
        if (!batch.loaded_successfully())
            continue;

        // TODO: insert here a check whether the batch is inside camera's frustum or not.

        render_tasks_map&  tasks = batch.is_translucent() ? from_ids_to_guids_translucent : from_ids_to_guids_opaque;

        tasks[ctx.from_batch_guid(batch_guid)].push_back(batch_guid);
    }

    // Here we start the actual rendering of batches collected above.

    glClearColor(cfg.clear_colour(0), cfg.clear_colour(1), cfg.clear_colour(2), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glViewport(0, 0, get_window_props().window_width(), get_window_props().window_height());
    glPolygonMode(GL_FRONT_AND_BACK, cfg.render_in_wireframe ? GL_LINE : GL_FILL);

    for (auto const&  id_and_guids : from_ids_to_guids_opaque)
        render_task(id_and_guids.second);
    for (auto const&  id_and_guids : from_ids_to_guids_translucent)
        render_task(id_and_guids.second);

    if (cfg.render_text)
        render_text();
}


void  simulator::render_task(std::vector<object_guid> const&  batch_guids)
{
    if (batch_guids.empty())
        return;

    simulation_context&  ctx = *context();
    render_configuration&  cfg = render_config();

    gfx::batch const  batch = ctx.from_batch_guid_to_batch(batch_guids.front());

    bool const  use_instancing =
            batch_guids.size() > 1UL &&    
            batch.get_available_resources().skeletal() == nullptr &&
            batch.has_instancing_data()
            ;

    if (!gfx::make_current(batch, cfg.draw_state, use_instancing))
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
        gfx::vertex_shader_instanced_data_provider  instanced_data_provider(batch);
        for (object_guid  batch_guid : batch_guids)
            instanced_data_provider.insert_from_model_to_camera_matrix(
                    cfg.matrix_from_world_to_camera * ctx.frame_world_matrix(ctx.frames_of_batch(batch_guid).front())
                    );
        gfx::render_batch(
                batch,
                instanced_data_provider,
                gfx::vertex_shader_uniform_data_provider(
                        batch,
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
        cfg.draw_state = batch.get_draw_state();
        return;
    }

    for (object_guid  batch_guid : batch_guids)
    {
        //ai::skeletal_motion_templates  motion_templates;
        //{
        //    scn::agent const* const  agent_ptr = scn::get_agent(*node_ptr);
        //    if (agent_ptr != nullptr && agent_ptr->get_props().m_skeleton_props != nullptr)
        //        motion_templates = agent_ptr->get_props().m_skeleton_props->skeletal_motion_templates;
        //}
        //if (!motion_templates.loaded_successfully())
        if (true)
            gfx::render_batch(
                    batch,
                    gfx::vertex_shader_uniform_data_provider(
                            batch,
                            { cfg.matrix_from_world_to_camera * ctx.frame_world_matrix(ctx.frames_of_batch(batch_guid).front()) },
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
        else
        {
            std::vector<matrix44>  frame;
            {
                NOT_IMPLEMENTED_YET();
                //matrix44 alignment_matrix;
                //angeo::from_base_matrix(batch_and_nodes.first.get_skeleton_alignment().get_skeleton_alignment(), alignment_matrix);

                //std::vector<matrix44>  to_bone_matrices;
                //{
                //    to_bone_matrices.resize(motion_templates.pose_frames().size());
                //    for (natural_32_bit  bone = 0U; bone != motion_templates.pose_frames().size(); ++bone)
                //    {
                //        angeo::to_base_matrix(motion_templates.pose_frames().at(bone), to_bone_matrices.at(bone));
                //        if (motion_templates.parents().at(bone) >= 0)
                //            to_bone_matrices.at(bone) *= to_bone_matrices.at(motion_templates.parents().at(bone));
                //    }
                //}

                //detail::skeleton_enumerate_nodes_of_bones(
                //        node_ptr,
                //        motion_templates,
                //        [&frame, &matrix_from_world_to_camera, motion_templates, &alignment_matrix, &to_bone_matrices, node_ptr](
                //            natural_32_bit const bone, scn::scene_node_ptr const  bone_node_ptr, bool const  has_parent) -> bool
                //            {

                //                ASSUMPTION(bone_node_ptr != nullptr);
                //                frame.push_back(
                //                        matrix_from_world_to_camera *
                //                        bone_node_ptr->get_world_matrix() *
                //                        to_bone_matrices.at(bone) *
                //                        alignment_matrix
                //                        );
                //                return true;
                //            }
                //        );
            }
            gfx::render_batch(
                    batch,
                    gfx::vertex_shader_uniform_data_provider(
                            batch,
                            frame,
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

        }
        cfg.draw_state = batch.get_draw_state();
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


}
