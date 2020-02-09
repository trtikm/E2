#include <ai/sensor.hpp>
#include <ai/simulator.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


std::unordered_map<SENSOR_KIND, property_map::default_config_records_map> const&  default_sensor_configs()
{
    static natural_32_bit  edit_order = 0U;
    static std::unordered_map<SENSOR_KIND, property_map::default_config_records_map> const  cfg {
        { SENSOR_KIND::TIMER, {
                { "period_in_seconds", { property_map::make_float(1.0f), true,
                        "A time period for repetitive sending the TIMER event to the owner.\n"
                        "If the period is <= 0.0001f, then the event is sent every time step.",
                        edit_order++} },
                { "consumed_in_seconds", { property_map::make_float(0.0f), true,
                        "Time already consumed from the current period.",
                        edit_order++} }
                } }
    };
    return cfg;
}


sensor::sensor(simulator* const  simulator_,
               SENSOR_KIND const  kind_,
               scene::record_id const&  self_rid_,
               object_id const&  owner_id_,
               std::shared_ptr<property_map> const  cfg_
               )
    : m_simulator(simulator_)
    , m_kind(kind_)
    , m_self_rid(self_rid_)
    , m_owner_id(owner_id_)
    , m_cfg(cfg_)
{
    ASSUMPTION(
        m_simulator != nullptr && m_owner_id.valid() &&
        [](SENSOR_KIND const  kind, property_map const&  cfg) -> bool {
            auto const&  default_cfg = default_sensor_configs().at(kind);
            if (cfg.size() != default_cfg.size())
                return false;
            for (auto const&  elem : default_cfg)
            {
                auto const  it = cfg.find(elem.first);
                if (it == cfg.end())
                {
                    if (elem.second.is_mandatory)
                        return false;
                }
                else if (it->second->get_type() != elem.second.value->get_type())
                    return false;
            }
            return true;
        }(m_kind, *m_cfg));
}


void  sensor::next_round(float_32_bit const  time_step_in_seconds)
{
    if (get_kind() == SENSOR_KIND::TIMER)
    {
        float_32_bit const  period = m_cfg->get_float("period_in_seconds");
        if (period <= 0.0001f)
            m_simulator->on_sensor_event(*this);
        else
        {
            float_32_bit&  consumed = m_cfg->get_float_ref("consumed_in_seconds");
            consumed += time_step_in_seconds;
            while (consumed >= period)
            {
                consumed -= period;
                m_simulator->on_sensor_event(*this);
            }
        }
    }
}


}
