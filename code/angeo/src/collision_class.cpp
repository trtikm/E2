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
    case COLLISION_CLASS::COMMON_SCENE_OBJECT: return "COMMON_SCENE_OBJECT";
    case COLLISION_CLASS::INFINITE_MASS_OBJECT: return "INFINITE_MASS_OBJECT";
    case COLLISION_CLASS::AGENT_MOTION_OBJECT: return "AGENT_MOTION_OBJECT";
    case COLLISION_CLASS::SIGHT_TARGET: return "SIGHT_TARGET";
    case COLLISION_CLASS::RAY_CAST_SIGHT: return "RAY_CAST_SIGHT";
    case COLLISION_CLASS::TRIGGER_GENERAL: return "TRIGGER_GENERAL";
    case COLLISION_CLASS::TRIGGER_SPECIAL: return "TRIGGER_SPECIAL";
    case COLLISION_CLASS::TRIGGER_ACTIVATOR: return "TRIGGER_ACTIVATOR";
    default:
        UNREACHABLE();
    }
}


COLLISION_CLASS read_collison_class_from_string(std::string const& name)
{
    static std::unordered_map<std::string, COLLISION_CLASS> const  map{
        {"COMMON_SCENE_OBJECT", COLLISION_CLASS::COMMON_SCENE_OBJECT},
        {"INFINITE_MASS_OBJECT", COLLISION_CLASS::INFINITE_MASS_OBJECT},
        {"SIGHT_TARGET", COLLISION_CLASS::SIGHT_TARGET},
        {"RAY_CAST_SIGHT", COLLISION_CLASS::RAY_CAST_SIGHT},
        {"TRIGGER_GENERAL", COLLISION_CLASS::TRIGGER_GENERAL},
        {"TRIGGER_SPECIAL", COLLISION_CLASS::TRIGGER_SPECIAL},
        {"TRIGGER_ACTIVATOR", COLLISION_CLASS::TRIGGER_ACTIVATOR},
    };
    auto const  it = map.find(name);
    ASSUMPTION(it != map.cend());
    return it->second;
}


bool  are_colliding(COLLISION_CLASS const  cc1, COLLISION_CLASS const  cc2)
{
    static const bool  table[get_num_collision_classes()][get_num_collision_classes()] = {
        // This must be a SYMETRIC matrix!
        { true,  true,  true,  false, true,  true,  false, false }, // COMMON_SCENE_OBJECT
        { true,  false, true,  false, true,  true,  false, false }, // INFINITE_MASS_OBJECT
        { true,  true,  true,  false, false, true,  false, false }, // AGENT_MOTION_OBJECT
        { false, false, false, false, true,  false, false, false }, // SIGHT_TARGET
        { true,  true,  false, true,  false, false, false, false }, // RAY_CAST_SIGHT
        { true,  true,  true,  false, false, false, false, true  }, // TRIGGER_GENERAL
        { false, false, false, false, false, false, false, true  }, // TRIGGER_SPECIAL
        { false, false, false, false, false, true,  true,  false }, // TRIGGER_ACTIVATOR
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
