#include <ai/sensor.hpp>
#include <ai/simulator.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


sensor::config_variable_type_and_value::config_variable_type_and_value(integer_32_bit const  value)
    : type(CONFIG_VARIABLE_TYPE::INT)
    , value_int(value)
    , value_float(0.0f)
    , value_string()
{}

sensor::config_variable_type_and_value::config_variable_type_and_value(float_32_bit const  value)
    : type(CONFIG_VARIABLE_TYPE::FLOAT)
    , value_int(0)
    , value_float(value)
    , value_string()
{}

sensor::config_variable_type_and_value::config_variable_type_and_value(std::string const&  value)
    : type(CONFIG_VARIABLE_TYPE::STRING)
    , value_int(0)
    , value_float(0.0f)
    , value_string(value)
{}


integer_32_bit  sensor::config_variable_type_and_value::get_int() const
{
    ASSUMPTION(get_type() == CONFIG_VARIABLE_TYPE::INT);
    return value_int;
}

float_32_bit  sensor::config_variable_type_and_value::get_float() const
{
    ASSUMPTION(get_type() == CONFIG_VARIABLE_TYPE::FLOAT);
    return value_float;
}

std::string const&  sensor::config_variable_type_and_value::get_string() const
{
    ASSUMPTION(get_type() == CONFIG_VARIABLE_TYPE::STRING);
    return value_string;
}


void  sensor::config_variable_type_and_value::set_int(integer_32_bit const  value)
{
    ASSUMPTION(get_type() == CONFIG_VARIABLE_TYPE::INT);
    value_int = value;
}

void  sensor::config_variable_type_and_value::set_float(float_32_bit const  value)
{
    ASSUMPTION(get_type() == CONFIG_VARIABLE_TYPE::FLOAT);
    value_float = value;
}

void  sensor::config_variable_type_and_value::set_string(std::string const&  value)
{
    ASSUMPTION(get_type() == CONFIG_VARIABLE_TYPE::STRING);
    value_string = value;
}


std::unordered_map<SENSOR_KIND, sensor::config> const&  sensor::default_configs()
{
    static std::unordered_map<SENSOR_KIND, config> const  cfg {
        {
            SENSOR_KIND::TIMER, {
                { "period_in_seconds", config_variable_type_and_value(1.0f) },
                { "consumed_in_seconds", config_variable_type_and_value(0.0f) }
                }
        }
    };
    return cfg;
}


sensor::sensor(simulator* const  simulator_, SENSOR_KIND const  kind_, object_id const&  owner_id_, config const&  cfg_)
    : m_simulator(simulator_)
    , m_kind(kind_)
    , m_owner_id(owner_id_)
    , m_cfg(cfg_)
{
    ASSUMPTION(m_simulator != nullptr && m_owner_id.valid() &&
        [](SENSOR_KIND const  kind, config const&  cfg) -> bool {
            auto const&  default_cfg = sensor::default_configs().at(kind);
            if (cfg.size() != default_cfg.size())
                return false;
            for (auto const& elem : default_cfg)
            {
                auto const  it = cfg.find(elem.first);
                if (it == cfg.cend())
                    return false;
                if (it->second.get_type() != elem.second.get_type())
                    return false;
            }
            return true;
        }(m_kind, m_cfg));
}


void  sensor::next_round(float_32_bit const  time_step_in_seconds)
{
    if (get_kind() == SENSOR_KIND::TIMER)
    {
        config_variable_type_and_value const&  period = m_cfg.at("period_in_seconds");
        config_variable_type_and_value&  consumed = m_cfg.at("consumed_in_seconds");
        consumed.set_float(consumed.get_float() + time_step_in_seconds);
        while (consumed.get_float() >= period.get_float())
        {
            consumed.set_float(consumed.get_float() - period.get_float());
            // TODO: send event to owner!
        }
    }
}


}
