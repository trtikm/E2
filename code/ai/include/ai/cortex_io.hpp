#ifndef AI_CORTEX_IO_HPP_INCLUDED
#   define AI_CORTEX_IO_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <memory>
#   include <vector>

namespace ai {


struct  cortex_io
{
    std::vector<float_32_bit>  input;   // Sensory info passed to the cortex; all values must be in range <0.0, 1.0>.
                                        // The size of the vector must be constant during whole simulation! 
    natural_32_bit  num_inner_inputs;   // Values 'input[0..(num_inner-1)]' comes from sensors of the inner enviroment,
                                        // and values 'input[num_inner..]' comes from sensors of the outer enviroment.
 
    std::vector<float_32_bit>  output;  // Actions info produced by the cortex; all values must be in range <0.0, 1.0>.
                                        // The size of the vector must be constant during whole simulation! 
};


using  cortex_io_ptr = std::shared_ptr<cortex_io>;


}

#endif
