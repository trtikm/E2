#include <ai/sensor_action.hpp>
#include <utility/invariants.hpp>
#include <unordered_map>
#include <functional>

namespace ai {


static std::unordered_map<natural_8_bit, std::pair<std::string, std::string> > const  from_index_to_name_and_description = {
    { as_number(SENSOR_ACTION_KIND::BEGIN_OF_LIFE), { "BEGIN_OF_LIFE", "Imports a template scene with agent, device, or sensor into the current one." } },
    { as_number(SENSOR_ACTION_KIND::END_OF_LIFE), { "END_OF_LIFE", "Erase self from the scene." } },
};

static std::unordered_map<std::string, SENSOR_ACTION_KIND> const  from_name_to_kind = []() {
    std::unordered_map<std::string, SENSOR_ACTION_KIND>  result;
    for (auto const&  elem : from_index_to_name_and_description)
        result.insert({ elem.second.first, as_sensor_action_kind(elem.first) });
    return result;
}();


std::string const&  description(SENSOR_ACTION_KIND const  kind)
{
    return from_index_to_name_and_description.at(as_number(kind)).second;
}


std::string const&  as_string(SENSOR_ACTION_KIND const  kind)
{
    return from_index_to_name_and_description.at(as_number(kind)).first;
}


SENSOR_ACTION_KIND  as_sensor_action_kind(std::string const&  name)
{
    return from_name_to_kind.at(name);
}


std::unordered_map<SENSOR_ACTION_KIND, property_map> const&  default_sensor_action_props()
{
    static std::unordered_map<SENSOR_ACTION_KIND, property_map>  props {
        { SENSOR_ACTION_KIND::BEGIN_OF_LIFE, property_map({
                { "scene_id", property_map::property_type_and_value("shared/import/TODO") },
                { "parent_nid", property_map::property_type_and_value("TODO") },
                { "frame_reference_nid", property_map::property_type_and_value("TODO") },
                })},
        { SENSOR_ACTION_KIND::END_OF_LIFE, property_map{} },
    };
    return props;
}


boost::property_tree::ptree  as_ptree(from_sensor_record_to_sensor_action_map const&  map, scene::node_id const&  root)
{
    boost::property_tree::ptree  result;
    for (auto const&  elem : map)
    {
        boost::property_tree::ptree  actions;
        for (auto const& action : elem.second)
        {
            boost::property_tree::ptree  prop;
            prop.put("kind", as_string(action.kind));
            prop.put_child("props", as_ptree(action.props));
            actions.push_back({"", prop});
        }
        result.put_child(
                ::as_string({
                        elem.first.get_node_id().copy(common_prefix_size(elem.first.get_node_id(), root)),
                        elem.first.get_folder_name(),
                        elem.first.get_record_name()
                        }),
                actions);
    }
    return result;
}


from_sensor_record_to_sensor_action_map  as_sensor_action_map(
        boost::property_tree::ptree const&  tree,
        scene::node_id const&  root
        )
{
    from_sensor_record_to_sensor_action_map  result;
    for (auto  it = tree.begin(); it != tree.end(); ++it)
    {
        scene::record_id const  relative_id = ::as_scene_record_id(it->first);
        scene::record_id const  id(root / relative_id.get_node_id(), relative_id.get_folder_name(), relative_id.get_record_name());
        std::vector<sensor_action>  actions;
        for (auto  action_it = it->second.begin(); action_it != it->second.end(); ++action_it)
        {
            actions.push_back({
                as_sensor_action_kind(action_it->second.get<std::string>("kind")),
                as_property_map(action_it->second.get_child("props"))
                });
        }
        result.insert({ id, actions });
    }
    return result;
}


}
