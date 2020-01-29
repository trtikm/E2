#ifndef AI_SENSOR_HPP_INCLUDED
#   define AI_SENSOR_HPP_INCLUDED

#   include <ai/sensor_kind.hpp>
#   include <ai/object_id.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <unordered_map>
#   include <string>

namespace ai {


struct  simulator;


struct sensor
{
    using  config_variable_name = std::string;

    enum CONFIG_VARIABLE_TYPE
    {
        INT = 0,
        FLOAT = 0,
        STRING = 0,
    };

    struct config_variable_type_and_value
    {
        config_variable_type_and_value(integer_32_bit const  value);
        config_variable_type_and_value(float_32_bit const  value);
        config_variable_type_and_value(std::string const& value);

        CONFIG_VARIABLE_TYPE  get_type() const { return type; }

        integer_32_bit  get_int() const;
        float_32_bit  get_float() const;
        std::string const& get_string() const;

        void  set_int(integer_32_bit const  value);
        void  set_float(float_32_bit const  value);
        void  set_string(std::string const&  value);

    private:
        CONFIG_VARIABLE_TYPE type;
        integer_32_bit  value_int;
        float_32_bit  value_float;
        std::string  value_string;
    };

    using  config = std::unordered_map<config_variable_name, config_variable_type_and_value>;
    static std::unordered_map<SENSOR_KIND, config> const&  default_configs();

    sensor(simulator* const  simulator_, SENSOR_KIND const  kind_, object_id const&  owner_id_, config const&  cfg_);

    SENSOR_KIND  get_kind() const { return m_kind; }
    object_id const&  get_owner_id() const { return m_owner_id; }
    config const&  get_config() const { return m_cfg; }

    void  next_round(float_32_bit const  time_step_in_seconds);

private:
    simulator*  m_simulator;
    SENSOR_KIND  m_kind;
    object_id  m_owner_id;
    config  m_cfg;
};


}

#endif
