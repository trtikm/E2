#include <ai/sensor.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


sensor::sensor(SENSOR_KIND const  kind_)
    : m_kind(kind_)
{
}


}
