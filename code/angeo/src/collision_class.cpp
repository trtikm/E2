#include <angeo/collision_class.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <unordered_map>

namespace angeo {


COLLISION_CLASS  as_collision_class(natural_8_bit  const  cc)
{
    ASSUMPTION(cc < get_num_collision_classes());
    return (COLLISION_CLASS)cc;
}


char const* to_string(COLLISION_CLASS const  cc)
{
    switch (cc)
    {
    case COLLISION_CLASS::STATIC_OBJECT: return "STATIC_OBJECT";
    case COLLISION_CLASS::COMMON_MOVEABLE_OBJECT: return "COMMON_MOVEABLE_OBJECT";
    case COLLISION_CLASS::HEAVY_MOVEABLE_OBJECT: return "HEAVY_MOVEABLE_OBJECT";
    case COLLISION_CLASS::AGENT_MOTION_OBJECT: return "AGENT_MOTION_OBJECT";
    case COLLISION_CLASS::FIELD_AREA: return "FIELD_AREA";
    case COLLISION_CLASS::SENSOR_DEDICATED: return "SENSOR_DEDICATED";
    case COLLISION_CLASS::SENSOR_WIDE_RANGE: return "SENSOR_WIDE_RANGE";
    case COLLISION_CLASS::SENSOR_NARROW_RANGE: return "SENSOR_NARROW_RANGE";
    case COLLISION_CLASS::RAY_CAST_TARGET: return "RAY_CAST_TARGET";
    default:
        UNREACHABLE();
    }
}


COLLISION_CLASS read_collison_class_from_string(std::string const& name)
{
    static std::unordered_map<std::string, COLLISION_CLASS> const  map{
        {"STATIC_OBJECT", COLLISION_CLASS::STATIC_OBJECT},
        {"COMMON_MOVEABLE_OBJECT", COLLISION_CLASS::COMMON_MOVEABLE_OBJECT},
        {"HEAVY_MOVEABLE_OBJECT", COLLISION_CLASS::HEAVY_MOVEABLE_OBJECT},
        {"AGENT_MOTION_OBJECT", COLLISION_CLASS::AGENT_MOTION_OBJECT},
        {"FIELD_AREA", COLLISION_CLASS::FIELD_AREA},
        {"SENSOR_DEDICATED", COLLISION_CLASS::SENSOR_DEDICATED},
        {"SENSOR_WIDE_RANGE", COLLISION_CLASS::SENSOR_WIDE_RANGE},
        {"SENSOR_NARROW_RANGE", COLLISION_CLASS::SENSOR_NARROW_RANGE},
        {"RAY_CAST_TARGET", COLLISION_CLASS::RAY_CAST_TARGET},
    };
    auto const  it = map.find(name);
    ASSUMPTION((natural_8_bit)map.size() == get_num_collision_classes() && it != map.cend());
    return it->second;
}


bool  are_colliding(COLLISION_CLASS const  cc1, COLLISION_CLASS const  cc2)
{
    static const bool  table[get_num_collision_classes()][get_num_collision_classes()] = {
        // This must be a SYMETRIC matrix!
        //STATIC|COMMON|HEAVY |AGENT |FIELD |S.DED |S.WIDE|S.NARR|RAY
        { false, true,  false, true,  false, false, false, false, false }, // STATIC_OBJECT
        { true,  true,  true,  true,  true,  false, true,  false, false }, // COMMON_MOVEABLE_OBJECT
        { false, true,  false, true,  false, true,  true,  false, false }, // HEAVY_MOVEABLE_OBJECT
        { true,  true,  true,  true,  true,  false, true,  false, false }, // AGENT_MOTION_OBJECT
        { false, true,  false, true,  false, false, false, false, false }, // FIELD_AREA
        { false, false, true,  false, false, true,  false, false, false }, // SENSOR_DEDICATED
        { false, true,  true,  true,  false, false, true,  true,  true  }, // SENSOR_WIDE_RANGE
        { false, false, false, false, false, false, true,  true,  true  }, // SENSOR_NARROW_RANGE
        { false, false, false, false, false, false, true,  true,  false }, // RAY_CAST_TARGET
    };
    static bool const  is_symetric = [](const bool  table[get_num_collision_classes()][get_num_collision_classes()]) {
        for (natural_8_bit i = 0U; i != get_num_collision_classes(); ++i)
            for (natural_8_bit j = i; j != get_num_collision_classes(); ++j)
            {
                INVARIANT(table[i][j] == table[j][i]);
                return false;
            }
        return true;
    }(table);
    return  table[as_number(cc1)][as_number(cc2)];
}


}
