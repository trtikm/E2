#include <ai/navigation.hpp>
#include <utility/timeprof.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <unordered_map>

namespace ai {


NAVOBJ_KIND  as_navobj_kind(natural_8_bit  const  kind)
{
    ASSUMPTION(kind < get_num_navobj_kinds());
    return (NAVOBJ_KIND)kind;
}


std::string  to_string(NAVOBJ_KIND const  kind)
{
    switch (kind)
    {
    case NAVOBJ_KIND::NAVCOMPONENT: return "NAVCOMPONENT";
    case NAVOBJ_KIND::WAYPOINT2D: return "WAYPOINT2D";
    case NAVOBJ_KIND::WAYPOINT3D: return "WAYPOINT3D";
    case NAVOBJ_KIND::WAYLINK: return "WAYLINK";
    case NAVOBJ_KIND::NONE: return "NONE";
    default:
        UNREACHABLE();
    }
}


NAVOBJ_KIND  read_navobj_kind_from_string(std::string const&  name)
{
    static std::unordered_map<std::string, NAVOBJ_KIND> const  map{
        {"NAVCOMPONENT", NAVOBJ_KIND::NAVCOMPONENT},
        {"WAYPOINT2D", NAVOBJ_KIND::WAYPOINT2D},
        {"WAYPOINT3D", NAVOBJ_KIND::WAYPOINT3D},
        {"WAYLINK", NAVOBJ_KIND::WAYLINK},
        {"NONE", NAVOBJ_KIND::NONE}
    };
    auto const  it = map.find(name);
    ASSUMPTION(it != map.cend());
    return it->second;
}


}

namespace ai {


navsystem::navsystem(simulation_context_const_ptr const  context_)
    : m_components()
    , m_navlinks()
    , m_context(context_)
{
    ASSUMPTION(m_context != nullptr);
}


}

namespace ai {


naveditor::naveditor(navsystem_ptr const  navsystem_)
    : m_navsystem(navsystem_)
    , m_context(m_navsystem->m_context)
{}


}
