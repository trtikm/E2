#ifndef CELLAB_DYNAMIC_STATE_OF_NEURAL_TISSUE_HPP_INCLUDED
#   define CELLAB_DYNAMIC_STATE_OF_NEURAL_TISSUE_HPP_INCLUDED

#   include <cellab/static_state_of_neural_tissue.hpp>
#   include <cellab/homogenous_slice_of_tissue.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <utility/array_of_bit_units.hpp>
#   include <utility/bits_reference.hpp>
#   include <vector>
#   include <memory>

namespace cellab {


struct dynamic_state_of_neural_tissue
{
    dynamic_state_of_neural_tissue(
            std::shared_ptr<static_state_of_neural_tissue const> const pointer_to_static_state_of_neural_tissue
            );

    std::shared_ptr<static_state_of_neural_tissue const> get_static_state_of_neural_tissue() const;

    bits_reference find_bits_of_cell_in_tissue(
            natural_32_bit const seek_along_x_axis,
            natural_32_bit const seek_along_y_axis,
            natural_32_bit const seek_along_columnar_axis
            );

    bits_reference find_bits_of_synapse_in_tissue(
            natural_32_bit const seek_to_cell_along_x_axis,
            natural_32_bit const seek_to_cell_along_y_axis,
            natural_32_bit const seek_to_cell_along_columnar_axis,
            natural_32_bit const index_of_synapse_in_territory_of_cell
            );

    bits_reference find_bits_of_signalling(
            natural_32_bit const seek_to_cell_along_x_axis,
            natural_32_bit const seek_to_cell_along_y_axis,
            natural_32_bit const seek_to_cell_along_columnar_axis
            );

    bits_reference find_bits_of_sensory_cell(natural_32_bit const index_of_sensory_cell);
    bits_reference find_bits_of_synapse_to_muscle(natural_32_bit const index_of_synapse_to_muscle);

private:
    typedef std::shared_ptr<homogenous_slice_of_tissue> pointer_to_homogenous_slice_of_tissue;

    std::shared_ptr<static_state_of_neural_tissue const> m_static_state_of_neural_tissue;
    std::vector<pointer_to_homogenous_slice_of_tissue> m_slices_of_tissue_cells;
    std::vector<pointer_to_homogenous_slice_of_tissue> m_slices_of_tissue_synapses;
    std::vector<pointer_to_homogenous_slice_of_tissue> m_slices_of_tissue_signalling_data;
    std::vector<pointer_to_homogenous_slice_of_tissue> m_slices_of_tissue_migration_data;
    array_of_bit_units m_bits_of_sensory_cells;
    array_of_bit_units m_bits_of_synapses_to_muscles;
};


// This function can early-detects wrap error for unsigned integers which could
// otherwise occure later when computing addresses of bits of individual cells inside
// dynamic_state_of_neural_tissue. By 'early-detection' we mean that the function is
// called in construction of static_state_of_neural_tissue. Therefore, it is safe to
// completelly omit all such checks inside any instance of dynamic_state_of_neural_tissue.
void check_for_unsigned_wrap_errors(static_state_of_neural_tissue const& tissue_props);


}

#endif
