#include <ai/agent.hpp>
#include <ai/cortex_mock.hpp>
#include <ai/cortex_random.hpp>
#include <ai/cortex_default.hpp>
#include <ai/cortex_robot.hpp>
#include <ai/cortex_netlab.hpp>
#include <ai/utils_ptree.hpp>
#include <angeo/collision_class.hpp>
#include <com/simulation_context.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


std::shared_ptr<cortex>  make_cortex(agent const*  myself_, boost::property_tree::ptree const&  config)
{
    std::string const  type = get_value<std::string>("type", config);

    if (type == "mock")
        return std::make_shared<cortex_mock>(myself_);

    bool const  allow_mock = get_value<bool>("allow_mock", config);

    if (type == "default")
        return std::make_shared<cortex_default>(myself_, allow_mock);
    else if (type == "random")
        return std::make_shared<cortex_random>(myself_, allow_mock);
    else if (type == "robot")
        return std::make_shared<cortex_robot>(myself_, allow_mock);
    else if (type == "netlab")
        return std::make_shared<cortex_netlab>(myself_, allow_mock);

    UNREACHABLE();
}


agent::agent(
        agent_config const  config,
        skeletal_motion_templates const  motion_templates,
        navsystem_const_ptr const  navsystem_,
        scene_binding_ptr const  binding
        )
    : m_system_state()
    , m_system_variables(load_agent_system_variables())
    , m_state_variables(load_agent_state_variables(config))

    , m_motion_templates(motion_templates)
    , m_binding(binding)

    , m_action_controller(config, this)
    , m_sight_controller(
            sight_controller::camera_config(get_ptree("camera", config.sight())),
            sight_controller::ray_cast_config(get_ptree("ray_casting", config.sight()), binding->context),
            m_motion_templates,
            m_binding
            )
    , m_cortex(make_cortex(this, config.cortex()))
    , m_navsystem(navsystem_)
{
    ASSUMPTION([this]() ->bool {
        for (auto const& var_and_ignored : get_system_variables())
            if (get_state_variables().count(var_and_ignored.first) != 0UL)
                return false;
        return true;
        }());
    ASSUMPTION(m_navsystem != nullptr);

    m_action_controller.initialise();
}


void  agent::next_round(float_32_bit const  time_step_in_seconds, cortex::mock_input_props const* const  mock_input_ptr)
{
    TMPROF_BLOCK();

    m_sight_controller.next_round(time_step_in_seconds);
    m_cortex->next_round(time_step_in_seconds, mock_input_ptr);
    update_system_state();
    update_system_variables(system_variables_ref(), get_system_state());
    m_action_controller.next_round(time_step_in_seconds);

//if (auto const  ptr = std::dynamic_pointer_cast<cortex_robot>(m_cortex)) if (ptr->use_mock())
//{
//for (auto const& var_and_value : get_system_variables()) SLOG(var_and_value.first << ": " << var_and_value.second << "\n");
//for (auto const& var_and_state : get_state_variables()) SLOG(var_and_state.first << ": " << var_and_state.second.get_value() << "\n");
//}

}


void  agent::update_system_state()
{
    system_state_ref() = {
            get_binding()->context->frame_explicit_coord_system_in_world_space(get_binding()->frame_guid_of_motion_object),
            get_sight_controller().get_camera() == nullptr ?
                    angeo::get_world_coord_system_explicit() :
                    *get_sight_controller().get_camera()->coordinate_system()
            };
}


}
