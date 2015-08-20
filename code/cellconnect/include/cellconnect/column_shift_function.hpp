#ifndef CELLCONNECT_COLUMN_SHIFT_FUNCTION_HPP_INCLUDED
#   define CELLCONNECT_COLUMN_SHIFT_FUNCTION_HPP_INCLUDED

#   include <cellab/dynamic_state_of_neural_tissue.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <functional>

namespace cellconnect {


struct column_shift_function
{
    /**
     * @brief column_shift_function
     * The default constructor initialises the identity shift function.
     */
    column_shift_function();

    std::pair<natural_32_bit,natural_32_bit>  operator()(natural_32_bit const  x_coord, natural_32_bit const  y_coord) const;

private:
    bool  m_use_identity_function;
};


}

#endif
