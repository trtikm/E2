#ifndef ANGEO_COLLISION_CLASS_HPP_INCLUDED
#   define ANGEO_COLLISION_CLASS_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <string>

namespace angeo {


enum struct  COLLISION_CLASS : natural_8_bit
{
    // HINT: Colliders of the following classes are often used for building collision shapes of rigid bodies:
    STATIC_OBJECT               = 0,
    COMMON_MOVEABLE_OBJECT      = 1,
    HEAVY_MOVEABLE_OBJECT       = 2,
    AGENT_MOTION_OBJECT         = 3,

    // HINT: Colliders of the following classes are often used for sensing and message/request processing in agents and devices:
    FIELD_AREA                  = 4,
    SENSOR_DEDICATED            = 5,
    SENSOR_WIDE_RANGE           = 6,
    SENSOR_NARROW_RANGE         = 7,
    RAY_CAST_TARGET             = 8,
};


constexpr natural_8_bit  get_num_collision_classes()
{
    return (natural_8_bit)COLLISION_CLASS::RAY_CAST_TARGET + (natural_8_bit)1;
}

inline natural_8_bit  as_number(COLLISION_CLASS const  cc) { return (natural_8_bit)cc; }
COLLISION_CLASS  as_collision_class(natural_8_bit  const  cc);
char const* to_string(COLLISION_CLASS const  cc);
COLLISION_CLASS read_collison_class_from_string(std::string const& name);

bool  are_colliding(COLLISION_CLASS const  cc1, COLLISION_CLASS const  cc2);


}

#endif
