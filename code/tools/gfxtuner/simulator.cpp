#include <gfxtuner/simulator.hpp>
#include <gfxtuner/simulator_notifications.hpp>
#include <gfxtuner/program_options.hpp>
#include <gfxtuner/draw_utils.hpp>
#include <qtgl/glapi.hpp>
#include <qtgl/draw.hpp>
#include <qtgl/batch_generators.hpp>
#include <qtgl/texture_generators.hpp>
#include <qtgl/camera_utils.hpp>
#include <utility/timeprof.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/random.hpp>
#include <utility/canonical_path.hpp>
#include <utility/msgstream.hpp>

#include <vector>
#include <map>
#include <algorithm>


simulator::simulator()
    : qtgl::real_time_simulator()

    , m_camera(
            qtgl::camera_perspective::create(
                    angeo::coordinate_system::create(
                            vector3(10.0f, 10.0f, 4.0f),
                            quaternion(0.293152988f, 0.245984003f, 0.593858004f, 0.707732975f)
                            ),
                    0.25f,
                    500.0f,
                    window_props()
                    )
            )
    , m_free_fly_config
            {
                {
                    false,
                    false,
                    2U,
                    2U,
                    -15.0f,
                    qtgl::free_fly_controler::keyboard_key_pressed(qtgl::KEY_W()),
                },
                {
                    false,
                    false,
                    2U,
                    2U,
                    15.0f,
                    qtgl::free_fly_controler::keyboard_key_pressed(qtgl::KEY_S()),
                },
                {
                    false,
                    false,
                    0U,
                    2U,
                    -15.0f,
                    qtgl::free_fly_controler::keyboard_key_pressed(qtgl::KEY_A()),
                },
                {
                    false,
                    false,
                    0U,
                    2U,
                    15.0f,
                    qtgl::free_fly_controler::keyboard_key_pressed(qtgl::KEY_D()),
                },
                {
                    false,
                    false,
                    1U,
                    2U,
                    -15.0f,
                    qtgl::free_fly_controler::keyboard_key_pressed(qtgl::KEY_Q()),
                },
                {
                    false,
                    false,
                    1U,
                    2U,
                    15.0f,
                    qtgl::free_fly_controler::keyboard_key_pressed(qtgl::KEY_E()),
                },
                {
                    true,
                    true,
                    2U,
                    0U,
                    -(10.0f * PI()) * (window_props().pixel_width_in_milimeters() / 1000.0f),
                    qtgl::free_fly_controler::mouse_button_pressed(qtgl::MIDDLE_MOUSE_BUTTON()),
                },
                {
                    true,
                    false,
                    0U,
                    1U,
                    -(10.0f * PI()) * (window_props().pixel_height_in_milimeters() / 1000.0f),
                    qtgl::free_fly_controler::mouse_button_pressed(qtgl::MIDDLE_MOUSE_BUTTON()),
                },
            }
    , m_batch_grid{ 
            qtgl::create_grid(
                    50.0f,
                    50.0f,
                    50.0f,
                    1.0f,
                    1.0f,
                    { 0.4f, 0.4f, 0.4f },
                    { 0.4f, 0.4f, 0.4f },
                    { 0.5f, 0.5f, 0.5f },
                    { 0.5f, 0.5f, 0.5f },
                    { 1.0f, 0.0f, 0.0f },
                    { 0.0f, 1.0f, 0.0f },
                    { 0.0f, 0.0f, 1.0f },
                    10U,
                    true,
                    get_program_options()->dataRoot()
                    )
            }
    , m_do_show_grid(true)

    , m_ske_test_batch{
            qtgl::batch::create(canonical_path(
                    boost::filesystem::path{get_program_options()->dataRoot()} /
                    "shared/gfx/models/ske_box/ske_box.txt"
                    ))
            }
    , m_ske_test_modelspace{
            canonical_path(
                boost::filesystem::path{get_program_options()->dataRoot()} /
                "shared/gfx/meshes/ske_box/ske_box/coord_systems.txt"
                )
            }
    , m_ske_test_keyframes{
            {canonical_path(
                boost::filesystem::path{get_program_options()->dataRoot()} /
                "shared/gfx/animation/ske_box/ske_box/ske_box_animation/keyframe_0.txt"
                )},
            {canonical_path(
                boost::filesystem::path{get_program_options()->dataRoot()} /
                "shared/gfx/animation/ske_box/ske_box/ske_box_animation/keyframe_1.txt"
                )},
            {canonical_path(
                boost::filesystem::path{get_program_options()->dataRoot()} /
                "shared/gfx/animation/ske_box/ske_box/ske_box_animation/keyframe_2.txt"
                )},
            {canonical_path(
                boost::filesystem::path{get_program_options()->dataRoot()} /
                "shared/gfx/animation/ske_box/ske_box/ske_box_animation/keyframe_3.txt"
                )},
            }
    , m_ske_test_time(0.0f)

    , m_paused(true)
    , m_do_single_step(false)
{}

simulator::~simulator()
{}


void  simulator::set_camera_speed(float_32_bit const  speed)
{
    ASSUMPTION(speed > 0.001f);
    m_free_fly_config.at(0UL).set_action_value(-speed);
    m_free_fly_config.at(1UL).set_action_value(speed);
    m_free_fly_config.at(2UL).set_action_value(-speed);
    m_free_fly_config.at(3UL).set_action_value(speed);
    m_free_fly_config.at(4UL).set_action_value(-speed);
    m_free_fly_config.at(5UL).set_action_value(speed);
}


void  simulator::next_round(float_64_bit const  seconds_from_previous_call,
                            bool const  is_this_pure_redraw_request)
{
    TMPROF_BLOCK();

    if (!is_this_pure_redraw_request)
    {
        qtgl::adjust(*m_camera,window_props());
        auto const translated_rotated =
            qtgl::free_fly(*m_camera->coordinate_system(), m_free_fly_config,
                           seconds_from_previous_call, mouse_props(), keyboard_props());
        if (translated_rotated.first)
            call_listeners(simulator_notifications::camera_position_updated());
        if (translated_rotated.second)
            call_listeners(simulator_notifications::camera_orientation_updated());

        if (keyboard_props().was_just_released(qtgl::KEY_SPACE()))
        {
            if (paused())
            {
                m_paused = !m_paused;
                call_listeners(simulator_notifications::paused());
            }
            m_do_single_step = true;
        }

        if (!m_do_single_step && keyboard_props().was_just_released(qtgl::KEY_PAUSE()))
        {
            m_paused = !m_paused;
            call_listeners(simulator_notifications::paused());
        }

        if (!paused())
            perform_simulation_step(seconds_from_previous_call);

        if (m_do_single_step)
        {
            m_paused = true;
            call_listeners(simulator_notifications::paused());
            m_do_single_step = false;
        }
    }

    qtgl::glapi().glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    qtgl::glapi().glViewport(0, 0, window_props().width_in_pixels(), window_props().height_in_pixels());

    matrix44  view_projection_matrix;
    qtgl::view_projection_matrix(*m_camera,view_projection_matrix);

    qtgl::draw_state_ptr  draw_state;
    if (m_do_show_grid)
        if (qtgl::make_current(*m_batch_grid, *m_batch_grid->draw_state()))
        {
            INVARIANT(m_batch_grid->shaders_binding().operator bool());
            render_batch(*m_batch_grid,view_projection_matrix);
            draw_state = m_batch_grid->draw_state();
        }

    render_simulation_state(view_projection_matrix,draw_state);

    qtgl::swap_buffers();
}


void  simulator::perform_simulation_step(float_64_bit const  time_to_simulate_in_seconds)
{
    TMPROF_BLOCK();

    if (m_ske_test_modelspace.data() == nullptr)
        return;
    for (natural_64_bit  i = 0ULL; i != m_ske_test_keyframes.size(); ++i)
    {
        if (m_ske_test_keyframes.at(i).data() == nullptr)
            return;
        if (m_ske_test_keyframes.at(i).data()->coord_systems().size() !=
            m_ske_test_modelspace.data()->coord_systems().size())
            return;
    }
    std::sort(
        m_ske_test_keyframes.begin(),m_ske_test_keyframes.end(),
        [](qtgl::keyframe const& left, qtgl::keyframe const& right) -> bool {
            return left.data()->time_point() < right.data()->time_point();
        });

    m_ske_test_time += time_to_simulate_in_seconds;
    float_32_bit const  animation_length =
            m_ske_test_keyframes.back().data()->time_point() - m_ske_test_keyframes.front().data()->time_point();
    while (m_ske_test_time >= animation_length)
        m_ske_test_time -= animation_length;
}


void  simulator::render_simulation_state(matrix44 const&  view_projection_matrix, qtgl::draw_state_ptr  draw_state)
{
    TMPROF_BLOCK();

    if (m_ske_test_modelspace.data() == nullptr)
        return;
    for (natural_64_bit  i = 0ULL; i != m_ske_test_keyframes.size(); ++i)
    {
        if (m_ske_test_keyframes.at(i).data() == nullptr)
            return;
        if (m_ske_test_keyframes.at(i).data()->coord_systems().size() !=
            m_ske_test_modelspace.data()->coord_systems().size())
            return;
    }
    std::sort(
        m_ske_test_keyframes.begin(),m_ske_test_keyframes.end(),
        [](qtgl::keyframe const& left, qtgl::keyframe const& right) -> bool {
            return left.data()->time_point() < right.data()->time_point();
        });

    if (m_ske_test_batch != nullptr && qtgl::make_current(*m_ske_test_batch, draw_state))
    {
        INVARIANT(m_ske_test_batch->shaders_binding().operator bool());
        std::vector<matrix44> transform_matrices(m_ske_test_modelspace.data()->coord_systems().size(),view_projection_matrix);
        {
            float_32_bit const  time_point = m_ske_test_keyframes.front().data()->time_point() + m_ske_test_time;
            natural_64_bit  keyframe_index = 0ULL;
            while (keyframe_index + 2ULL < m_ske_test_keyframes.size() &&
                   time_point >= m_ske_test_keyframes.at(keyframe_index + 1ULL).data()->time_point())
                ++keyframe_index;
            INVARIANT(keyframe_index + 1ULL < m_ske_test_keyframes.size());
            INVARIANT(time_point >= m_ske_test_keyframes.at(keyframe_index).data()->time_point());
            INVARIANT(time_point < m_ske_test_keyframes.at(keyframe_index + 1ULL).data()->time_point());

            float_32_bit  interpolation_param;
            {
                float_32_bit const  dt =
                        m_ske_test_keyframes.at(keyframe_index + 1ULL).data()->time_point() -
                        m_ske_test_keyframes.at(keyframe_index).data()->time_point()
                        ;
                if (dt < 0.001f)
                    interpolation_param = 0.0f;
                else
                    interpolation_param = (time_point - m_ske_test_keyframes.at(keyframe_index).data()->time_point()) / dt;
                interpolation_param = std::max(-1.0f,std::min(interpolation_param,1.0f));
            }

//keyframe_index = 0ULL;
//interpolation_param = 0.0f;

            for (natural_64_bit  i = 0; i != m_ske_test_modelspace.data()->coord_systems().size(); ++i)
            {
                matrix44 M;
                {
                    angeo::coordinate_system  S;
                    S = m_ske_test_keyframes.at(keyframe_index).data()->coord_systems().at(i);
                    angeo::interpolate_spherical(
                                m_ske_test_keyframes.at(keyframe_index).data()->coord_systems().at(i),
                                m_ske_test_keyframes.at(keyframe_index + 1ULL).data()->coord_systems().at(i),
                                interpolation_param,
                                S
                                );
                    angeo::from_base_matrix(S,M);
                }
                transform_matrices.at(i) *= M;
            }

            for (natural_64_bit  i = 0; i != m_ske_test_modelspace.data()->coord_systems().size(); ++i)
            {
                matrix44 M;
                angeo::to_base_matrix(m_ske_test_modelspace.data()->coord_systems().at(i),M);
                transform_matrices.at(i) *= M;
            }
        }
        render_batch(*m_ske_test_batch,transform_matrices,{1.0f,0.0f,0.0f,0.0f});
        draw_state = m_ske_test_batch->draw_state();
    }
}
