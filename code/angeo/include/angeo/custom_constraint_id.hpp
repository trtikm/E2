#ifndef ANGEO_CUSTOM_CONSTRAINT_ID_HPP_INCLUDED
#   define ANGEO_CUSTOM_CONSTRAINT_ID_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>

namespace angeo {


using custom_constraint_id = natural_32_bit;


inline constexpr custom_constraint_id  invalid_custom_constraint_id() { return 0U; }


}

#endif
