#ifndef AI_SENSOR_ACTION_HPP_INCLUDED
#   define AI_SENSOR_ACTION_HPP_INCLUDED

#   include <ai/sensor_kind.hpp>
#   include <ai/scene.hpp>
#   include <ai/property_map.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <unordered_map>
#   include <string>
#   include <vector>

namespace ai {


enum struct  SENSOR_ACTION_KIND : natural_8_bit
{
    END_OF_LIFE = 0,
};

std::string const&  description(SENSOR_ACTION_KIND const  kind);

inline natural_8_bit  as_number(SENSOR_ACTION_KIND const  kind) noexcept
{
    return *reinterpret_cast<natural_8_bit const*>(&kind);
}

inline SENSOR_ACTION_KIND  as_sensor_action_kind(natural_8_bit const  index)
{
    return (SENSOR_ACTION_KIND)index;
}

inline constexpr natural_32_bit  num_sensor_action_kinds() { return (natural_32_bit)SENSOR_ACTION_KIND::END_OF_LIFE + 1U; }

std::string const&  as_string(SENSOR_ACTION_KIND const  kind);
SENSOR_ACTION_KIND  as_sensor_action_kind(std::string const&  name);


struct  sensor_action
{
    SENSOR_ACTION_KIND  kind;
    property_map  props;
};

using  from_sensor_event_to_sensor_action_map = std::unordered_map<
        SENSOR_KIND,
        std::unordered_map<
                scene::node_id,     // Of the sensor sending an event to this device.
                std::vector<sensor_action>
                >
        >;

std::unordered_map<SENSOR_ACTION_KIND, property_map> const&  default_sensor_action_props();


}

#endif
