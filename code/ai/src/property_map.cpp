#include <ai/property_map.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


property_map::property_type_and_value::property_type_and_value(integer_32_bit const  value)
    : type(PROPERTY_TYPE::INT)
    , value_int(value)
    , value_float(0.0f)
    , value_string()
{}

property_map::property_type_and_value::property_type_and_value(float_32_bit const  value)
    : type(PROPERTY_TYPE::FLOAT)
    , value_int(0)
    , value_float(value)
    , value_string()
{}

property_map::property_type_and_value::property_type_and_value(std::string const&  value)
    : type(PROPERTY_TYPE::STRING)
    , value_int(0)
    , value_float(0.0f)
    , value_string(value)
{}


integer_32_bit  property_map::property_type_and_value::get_int() const
{
    ASSUMPTION(get_type() == PROPERTY_TYPE::INT);
    return value_int;
}

float_32_bit  property_map::property_type_and_value::get_float() const
{
    ASSUMPTION(get_type() == PROPERTY_TYPE::FLOAT);
    return value_float;
}

std::string const&  property_map::property_type_and_value::get_string() const
{
    ASSUMPTION(get_type() == PROPERTY_TYPE::STRING);
    return value_string;
}


void  property_map::property_type_and_value::set_int(integer_32_bit const  value)
{
    ASSUMPTION(get_type() == PROPERTY_TYPE::INT);
    value_int = value;
}

void  property_map::property_type_and_value::set_float(float_32_bit const  value)
{
    ASSUMPTION(get_type() == PROPERTY_TYPE::FLOAT);
    value_float = value;
}

void  property_map::property_type_and_value::set_string(std::string const&  value)
{
    ASSUMPTION(get_type() == PROPERTY_TYPE::STRING);
    value_string = value;
}


boost::property_tree::ptree  as_ptree(property_map const&  map)
{
    boost::property_tree::ptree  result;
    for (auto const& elem : map)
    {
        boost::property_tree::ptree  prop;
        switch (elem.second.get_type())
        {
        case property_map::PROPERTY_TYPE::INT:
            prop.put("type", "INT");
            prop.put("value", std::to_string(elem.second.get_int()));
            break;
        case property_map::PROPERTY_TYPE::FLOAT:
            prop.put("type", "FLOAT");
            prop.put("value", std::to_string(elem.second.get_float()));
            break;
        case property_map::PROPERTY_TYPE::STRING:
            prop.put("type", "STRING");
            prop.put("value", elem.second.get_string());
            break;
        default: UNREACHABLE(); break;
        }
        result.put_child(elem.first, prop);
    }
    return result;
}


property_map  as_property_map(boost::property_tree::ptree const&  tree)
{
    property_map::map_type  result;
    for (auto it = tree.begin(); it != tree.end(); ++it)
    {
        std::string const  type = it->second.get<std::string>("type");
        if (type == "INT")
            result.insert({ it->first, property_map::property_type_and_value(it->second.get<integer_32_bit>("value")) });
        if (type == "FLOAT")
            result.insert({ it->first, property_map::property_type_and_value(it->second.get<float_32_bit>("value")) });
        if (type == "STRING")
            result.insert({ it->first, property_map::property_type_and_value(it->second.get<std::string>("value")) });
        else { UNREACHABLE(); }
    }
    return result;
}


}
