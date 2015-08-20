#include <cellconnect/column_shift_function.hpp>
#include <utility/basic_numeric_types.hpp>
#include <memory>
#include <vector>
#include <utility/development.hpp>

namespace cellconnect {


column_shift_function::column_shift_function()
    : m_use_identity_function(true)
{}


std::pair<natural_32_bit,natural_32_bit>  column_shift_function::operator()(natural_32_bit const  x_coord, natural_32_bit const  y_coord) const
{
    if (m_use_identity_function)
        return std::make_pair(x_coord,y_coord);

    NOT_IMPLEMENTED_YET();
}


}
