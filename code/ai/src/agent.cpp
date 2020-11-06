#include <ai/agent.hpp>
#include <ai/cortex_mock.hpp>
#include <ai/cortex_random.hpp>
#include <ai/cortex_default.hpp>
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


static sight_controller::ray_cast_config  make_ray_cast_config(agent_config const  config, scene_binding_ptr const  binding)
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

    //return sight_controller::ray_cast_config{
    //        true,               // do_directed_ray_casts
    //        num_raycasts,       // num_random_ray_casts_per_second
    //        raycast_life_sec,   // max_ray_cast_info_life_time_in_seconds
    //        num_cells_x,        // num_cells_along_x_axis; must be a power of 2.
    //        num_cells_y,        // num_cells_along_y_axis; must be a power of 2.
    //        cell_distribution,  // distribution_of_cells_in_camera_space
    //        collider_filter,    // collider_filter
    //        depth_image_func    // depth_image_func
    //        };
    return sight_controller::ray_cast_config(true, 0U);
}


std::shared_ptr<cortex>  make_cortex(agent_config const  config)
{
    if (config.use_cortex_mock())
        return std::make_shared<cortex_mock>();
    return std::make_shared<cortex>();
}


agent::agent(
        agent_config const  config,
        skeletal_motion_templates const  motion_templates,
        scene_binding_ptr const  binding
        )
    : m_state_variables(load_agent_state_variables(config))
    , m_motion_templates(motion_templates)
    , m_binding(binding)
    , m_action_controller(config, this)
    , m_sight_controller(
            make_camera_config(),
            make_ray_cast_config(config, binding),
            m_motion_templates,
            m_binding
            )
    , m_cortex(make_cortex(config))
{
    m_action_controller.initialise();
}


void  agent::next_round(float_32_bit const  time_step_in_seconds, cortex::mock_input_props const* const  mock_input_ptr)
{
    TMPROF_BLOCK();

    m_sight_controller.next_round(time_step_in_seconds);
    m_cortex->next_round(time_step_in_seconds, mock_input_ptr);
    m_action_controller.next_round(time_step_in_seconds);
}


}
