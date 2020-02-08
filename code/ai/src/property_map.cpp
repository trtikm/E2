#include <ai/property_map.hpp>
#include <utility/assumptions.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


property_map  property_map::clone() const
{
    property_map  result;
    for (auto const&  elem : get_map())
        result.set(elem);
    return result;
}


void  property_map::set(property_name const&  name, property_type_and_value const&  value)
{
    switch (value.get_type())
    {
    case PROPERTY_TYPE::BOOL: set_bool(name, as<bool_value>(value).value()); break;
    case PROPERTY_TYPE::INT: set_int(name, as<int_value>(value).value()); break;
    case PROPERTY_TYPE::FLOAT: set_float(name, as<float_value>(value).value()); break;
    case PROPERTY_TYPE::STRING: set_string(name, as<string_value>(value).value()); break;
    default: UNREACHABLE(); break;
    }
}


}


std::string  as_string(ai::property_map::property_type_and_value const&  prop)
{
    switch (prop.get_type())
    {
    case ai::property_map::PROPERTY_TYPE::BOOL:
        return std::to_string(ai::property_map::as<ai::property_map::bool_value>(prop).value());
    case ai::property_map::PROPERTY_TYPE::INT:
        return std::to_string(ai::property_map::as<ai::property_map::int_value>(prop).value());
    case ai::property_map::PROPERTY_TYPE::FLOAT:
        return std::to_string(ai::property_map::as<ai::property_map::float_value>(prop).value());
    case ai::property_map::PROPERTY_TYPE::STRING:
        return ai::property_map::as<ai::property_map::string_value>(prop).value();
    default: UNREACHABLE(); break;
    }
}


ai::property_map::property_value_ptr  as_property_map_value(
        ai::property_map::PROPERTY_TYPE const  type,
        std::string const&  value_text
        )
{
    switch (type)
    {
    case ai::property_map::PROPERTY_TYPE::BOOL:
        return ai::property_map::make_bool(
                    value_text == "true" ||
                    value_text == "TRUE" ||
                    value_text == "True" ||
                    value_text == "tt" ||
                    (integer_32_bit)std::atoi(value_text.c_str()) != 0
                    );
    case ai::property_map::PROPERTY_TYPE::INT:
        return ai::property_map::make_int((integer_32_bit)std::atoi(value_text.c_str()));
    case ai::property_map::PROPERTY_TYPE::FLOAT:
        return ai::property_map::make_float((float_32_bit)std::atof(value_text.c_str()));
    case ai::property_map::PROPERTY_TYPE::STRING:
        return ai::property_map::make_string(value_text);
    default: UNREACHABLE(); break;
    }
}


boost::property_tree::ptree  as_ptree(ai::property_map const&  map)
{
    boost::property_tree::ptree  result;
    for (auto const& elem : map)
    {
        boost::property_tree::ptree  prop;
        switch (elem.second->get_type())
        {
        case ai::property_map::PROPERTY_TYPE::BOOL: prop.put("type", "BOOL"); break;
        case ai::property_map::PROPERTY_TYPE::INT: prop.put("type", "INT"); break;
        case ai::property_map::PROPERTY_TYPE::FLOAT: prop.put("type", "FLOAT"); break;
        case ai::property_map::PROPERTY_TYPE::STRING: prop.put("type", "STRING"); break;
        default: UNREACHABLE(); break;
        }
        prop.put("value", as_string(*elem.second));
        result.put_child(elem.first, prop);
    }
    return result;
}


ai::property_map  as_property_map(boost::property_tree::ptree const&  tree)
{
    ai::property_map  result;
    for (auto it = tree.begin(); it != tree.end(); ++it)
    {
        std::string const  type = it->second.get<std::string>("type");
        if (type == "BOOL")
        {
            std::string const  value_text = it->second.get<std::string>("value");
            result.set_bool(
                    it->first,
                    value_text == "true"
                        || value_text == "TRUE"
                        || value_text == "True"
                        || value_text == "tt"
                        || (integer_32_bit)std::atoi(value_text.c_str()) != 0
                    );
        }
        else if (type == "INT")
            result.set_int(it->first, it->second.get<integer_32_bit>("value"));
        else if (type == "FLOAT")
            result.set_float(it->first, it->second.get<float_32_bit>("value"));
        else if (type == "STRING")
            result.set_string(it->first, it->second.get<std::string>("value"));
        else { UNREACHABLE(); }
    }
    return result;
}
