#ifndef ANGEO_COLLISION_CLASS_HPP_INCLUDED
#   define ANGEO_COLLISION_CLASS_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <string>

namespace angeo {


enum struct  COLLISION_CLASS : natural_8_bit
{
    COMMON_SCENE_OBJECT     = 0,
    INFINITE_MASS_OBJECT    = 1,
    AGENT_MOTION_OBJECT     = 2,
    SIGHT_TARGET            = 3,
    RAY_CAST_SIGHT          = 4,
    TRIGGER_GENERAL         = 5,
    TRIGGER_SPECIAL         = 6,
    TRIGGER_ACTIVATOR       = 7,
};


constexpr natural_8_bit  get_num_collision_classes()
{
    return (natural_8_bit)COLLISION_CLASS::TRIGGER_ACTIVATOR + (natural_8_bit)1;
}

inline natural_8_bit  as_number(COLLISION_CLASS const  cc) { return (natural_8_bit)cc; }
COLLISION_CLASS  as_collision_class(natural_8_bit  const  cc);
char const* to_string(COLLISION_CLASS const  cc);
COLLISION_CLASS read_collison_class_from_string(std::string const& name);

bool  are_colliding(COLLISION_CLASS const  cc1, COLLISION_CLASS const  cc2);


}

#endif
