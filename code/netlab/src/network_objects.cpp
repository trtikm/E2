#include <netlab/network_objects.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>

namespace netlab {


spiker::spiker()
    : m_last_update_id(0UL)
{}


ship::ship()
    : m_position(0.0f,0.0f,0.0f)
    , m_velocity(0.0f,0.0f,0.0f)
{}


}
