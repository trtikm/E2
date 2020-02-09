#ifndef AI_SENSOR_ACTION_HPP_INCLUDED
#   define AI_SENSOR_ACTION_HPP_INCLUDED

#   include <ai/scene.hpp>
#   include <ai/property_map.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <unordered_map>
#   include <unordered_set>
#   include <string>
#   include <vector>

namespace ai {


enum struct  SENSOR_ACTION_KIND : natural_8_bit
{
    BEGIN_OF_LIFE       = 0,
    END_OF_LIFE         = 1,
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


struct  sensor_action
{
    SENSOR_ACTION_KIND  kind;
    property_map  props;
};

using  from_sensor_record_to_sensor_action_map = std::unordered_map<scene::record_id, std::vector<sensor_action> >;

std::unordered_map<SENSOR_ACTION_KIND, property_map::default_config_records_map> const&  default_sensor_action_configs();


}


std::string const&  as_string(ai::SENSOR_ACTION_KIND const  kind);
ai::SENSOR_ACTION_KIND  as_sensor_action_kind(std::string const& name);


boost::property_tree::ptree  as_ptree(ai::from_sensor_record_to_sensor_action_map const& map, ai::scene::node_id const& root);
ai::from_sensor_record_to_sensor_action_map  as_sensor_action_map(boost::property_tree::ptree const& tree, ai::scene::node_id const& root);


#endif
