#include <ai/agent.hpp>
#include <ai/cortex_mock.hpp>
#include <ai/cortex_random.hpp>
#include <ai/cortex_robot.hpp>
#include <angeo/collision_class.hpp>
#include <com/simulation_context.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


static sight_controller::camera_config  make_camera_config()
{
    return sight_controller::camera_config{
                    PI() / 4.0f,    // horizontal_fov_angle
                    PI() / 4.0f,    // vertical_fov_angle
                    0.05f,          // near_plane
                    10.0f,          // far_plane
                    0.0f            // origin_z_shift
                    };
}


static sight_controller::ray_cast_config  make_ray_cast_config(AGENT_KIND const  agent_kind, scene_binding_ptr const  binding)
{
    auto const  cell_distribution = [](float_32_bit const  x) -> float_32_bit {
        return x * x;
    };

    auto const  collider_filter = [binding](com::object_guid const collider_guid, angeo::COLLISION_CLASS const  cc) -> bool {
        switch (cc)
        {
        case angeo::COLLISION_CLASS::AGENT_MOTION_OBJECT:
        case angeo::COLLISION_CLASS::FIELD_AREA:
            return false;
        case angeo::COLLISION_CLASS::SENSOR_DEDICATED:
        case angeo::COLLISION_CLASS::SENSOR_NARROW_RANGE:
        case angeo::COLLISION_CLASS::SENSOR_WIDE_RANGE: {
            com::object_guid const  owner_guid = binding->context->owner_of_collider(collider_guid);
            return binding->context->is_valid_sensor_guid(owner_guid) && binding->context->is_sensor_enabled(owner_guid);
            }
        default:
            return true;
        }
    };

    auto const  depth_image_func = [](float_32_bit const  x) -> float_32_bit {
        return x;
    }; 

    static natural_16_bit constexpr  num_cells_x = 32U;
    static natural_16_bit constexpr  num_cells_y = 32U;
    static natural_32_bit constexpr  num_raycasts = 2U * num_cells_x * num_cells_y;
    static float_32_bit constexpr  raycast_life_sec = 0.1f;

    switch (agent_kind)
    {
    case AGENT_KIND::MOCK:
    case AGENT_KIND::ROBOT:
        return sight_controller::ray_cast_config{
                num_raycasts,       // num_raycasts_per_second
                raycast_life_sec,   // max_ray_cast_info_life_time_in_seconds
                num_cells_x,        // num_cells_along_x_axis; must be a power of 2.
                num_cells_y,        // num_cells_along_y_axis; must be a power of 2.
                cell_distribution,  // distribution_of_cells_in_camera_space
                collider_filter,    // collider_filter
                depth_image_func    // depth_image_func
                };
    case AGENT_KIND::STATIONARY:
    case AGENT_KIND::RANDOM:
        return sight_controller::ray_cast_config{0U};
    default: { UNREACHABLE(); return sight_controller::ray_cast_config(); }
    }
}


std::shared_ptr<cortex>  make_cortex(AGENT_KIND const  agent_kind)
{
    switch (agent_kind)
    {
    case AGENT_KIND::MOCK:
        return std::make_shared<cortex_mock>();
    case AGENT_KIND::STATIONARY:
        return std::make_shared<cortex>();
    case AGENT_KIND::RANDOM:
        return std::make_shared<cortex_random>();
    case AGENT_KIND::ROBOT:
        return std::make_shared<cortex_robot>();
    default: { UNREACHABLE(); break; }
    }
}


agent::agent(
        AGENT_KIND const  agent_kind,
        skeletal_motion_templates const  motion_templates,
        scene_binding_ptr const  binding
        )
    : m_agent_kind(agent_kind)
    , m_motion_templates(motion_templates)
    , m_binding(binding)
    , m_action_controller(motion_templates, binding)
    , m_sight_controller(
            make_camera_config(),
            make_ray_cast_config(agent_kind, binding),
            m_motion_templates,
            m_binding
            )
    , m_cortex(make_cortex(agent_kind))
{}


void  agent::next_round(
        float_32_bit const  time_step_in_seconds,
        osi::keyboard_props const&  keyboard,
        osi::mouse_props const&  mouse,
        osi::window_props const&  window
        )
{
    TMPROF_BLOCK();

    m_sight_controller.next_round(time_step_in_seconds, m_action_controller.get_motion_object_collider_guids());
    m_cortex->next_round(time_step_in_seconds, keyboard, mouse, window);
    m_action_controller.next_round(time_step_in_seconds, m_cortex->get_motion_desire_props());
}


}
