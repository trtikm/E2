#include <ai/sensor.hpp>
#include <ai/simulator.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


std::unordered_map<SENSOR_KIND, property_map> const&  sensor::default_configs()
{
    static std::unordered_map<SENSOR_KIND, property_map> const  cfg{
        {
            SENSOR_KIND::TIMER, property_map({
                { "period_in_seconds", property_map::property_type_and_value(1.0f) },
                { "consumed_in_seconds", property_map::property_type_and_value(0.0f) }
                })
        }
    };
    return cfg;
}


sensor::sensor(simulator* const  simulator_,
               SENSOR_KIND const  kind_,
               scene::node_id const&  self_nid_,
               object_id const&  owner_id_,
               std::shared_ptr<property_map> const  cfg_
               )
    : m_simulator(simulator_)
    , m_kind(kind_)
    , m_self_nid(self_nid_)
    , m_owner_id(owner_id_)
    , m_cfg(cfg_)
{
    ASSUMPTION(m_simulator != nullptr && m_owner_id.valid() &&
        [](SENSOR_KIND const  kind, property_map const&  cfg) -> bool {
            auto const&  default_cfg = sensor::default_configs().at(kind);
            if (cfg.size() != default_cfg.size())
                return false;
            for (auto const&  elem : default_cfg)
            {
                auto const  it = cfg.find(elem.first);
                if (it == cfg.end())
                    return false;
                if (it->second.get_type() != elem.second.get_type())
                    return false;
            }
            return true;
        }(m_kind, *m_cfg));
}


void  sensor::next_round(float_32_bit const  time_step_in_seconds)
{
    if (get_kind() == SENSOR_KIND::TIMER)
    {
        property_map::property_type_and_value const&  period = m_cfg->at("period_in_seconds");
        property_map::property_type_and_value&  consumed = m_cfg->at("consumed_in_seconds");
        consumed.set_float(consumed.get_float() + time_step_in_seconds);
        while (consumed.get_float() >= period.get_float())
        {
            consumed.set_float(consumed.get_float() - period.get_float());
            m_simulator->on_sensor_event(*this);
        }
    }
}


}
