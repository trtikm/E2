#ifndef AI_SENSOR_HPP_INCLUDED
#   define AI_SENSOR_HPP_INCLUDED

#   include <ai/sensor_kind.hpp>
#   include <utility/basic_numeric_types.hpp>

namespace ai {


struct sensor
{
    explicit sensor(SENSOR_KIND const  kind_);

    SENSOR_KIND  get_kind() const { return m_kind; }

private:
    SENSOR_KIND  m_kind;
};


}

#endif
