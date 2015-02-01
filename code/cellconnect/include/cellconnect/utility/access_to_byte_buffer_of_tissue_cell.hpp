#ifndef CELLCONNECT_UTILITY_ACCESS_TO_BYTE_BUFFER_OF_TISSUE_CELL_HPP_INCLUDED
#   define CELLCONNECT_UTILITY_ACCESS_TO_BYTE_BUFFER_OF_TISSUE_CELL_HPP_INCLUDED

#   include <cellab/dynamic_state_of_neural_tissue.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <memory>

namespace cellconnect {


natural_32_bit  compute_size_of_byte_buffer_of_tissue_cell(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue const> const  dynamic_state_ptr,
        cellab::kind_of_cell const  kind_of_tissue_cell
        );


natural_32_bit  read_from_byte_buffer_of_tissue_cell(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue const> const  dynamic_state_ptr,
        natural_32_bit const  x_coord_of_tissue_cell,
        natural_32_bit const  y_coord_of_tissue_cell,
        natural_32_bit const  columnar_coord_of_tissue_cell,
        natural_32_bit const  index_of_byte_where_to_start_reading,
        natural_8_bit const  num_bytes_to_read
        );


void  write_to_byte_buffer_of_tissue_cell(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        natural_32_bit const  x_coord_of_tissue_cell,
        natural_32_bit const  y_coord_of_tissue_cell,
        natural_32_bit const  columnar_coord_of_tissue_cell,
        natural_32_bit const  index_of_byte_where_to_start_writing,
        natural_8_bit const  num_bytes_to_write,
        natural_32_bit const  value_to_be_written
        );


}

#endif
