#include <com/object_guid.hpp>
#include <utility/timeprof.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <unordered_map>

namespace com {


object_guid::object_guid(OBJECT_KIND const kind_, integer_32_bit const  index_)
    : kind(kind_)
    , index((index_type)index_)
{
    ASSUMPTION(index_ < std::numeric_limits<object_guid::index_type>::max());
}


OBJECT_KIND  as_object_kind(natural_8_bit  const  okind)
{
    ASSUMPTION(okind < get_num_object_kinds());
    return (OBJECT_KIND)okind;
}


std::string  to_string(OBJECT_KIND const  okind)
{
    switch (okind)
    {
    case OBJECT_KIND::FOLDER: return "FOLDER";
    case OBJECT_KIND::FRAME: return "FRAME";
    case OBJECT_KIND::BATCH: return "BATCH";
    case OBJECT_KIND::COLLIDER: return "COLLIDER";
    case OBJECT_KIND::RIGID_BODY: return "RIGID_BODY";
    case OBJECT_KIND::TIMER: return "TIMER";
    case OBJECT_KIND::SENSOR: return "SENSOR";
    case OBJECT_KIND::AGENT: return "AGENT";
    case OBJECT_KIND::NONE: return "NONE";
    default:
        UNREACHABLE();
    }
}


OBJECT_KIND  read_object_kind_from_string(std::string const&  name)
{
    static std::unordered_map<std::string, OBJECT_KIND> const  map{
        {"FOLDER", OBJECT_KIND::FOLDER},
        {"FRAME", OBJECT_KIND::FRAME},
        {"BATCH", OBJECT_KIND::BATCH},
        {"COLLIDER", OBJECT_KIND::COLLIDER},
        {"RIGID_BODY", OBJECT_KIND::RIGID_BODY},
        {"TIMER", OBJECT_KIND::TIMER},
        {"SENSOR", OBJECT_KIND::SENSOR},
        {"AGENT", OBJECT_KIND::AGENT},
        {"NONE", OBJECT_KIND::NONE}
    };
    auto const  it = map.find(name);
    ASSUMPTION(it != map.cend());
    return it->second;
}


}
