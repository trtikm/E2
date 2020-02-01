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


}
