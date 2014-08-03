#ifndef CELLAB_DYNAMIC_STATE_OF_NEURAL_TISSUE_HPP_INCLUDED
#   define CELLAB_DYNAMIC_STATE_OF_NEURAL_TISSUE_HPP_INCLUDED

#   include <cellab/static_state_of_neural_tissue.hpp>
#   include <utility/bits_reference.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <boost/noncopyable.hpp>
#   include <boost/scoped_array.hpp>
#   include <array>
#   include <tuple>
#   include <memory>

namespace cellab {


struct reference_to_list_of_indices_of_synampses
{
    reference_to_list_of_indices_of_synampses(
            natural_16_bit* const pointer_to_pair_of_head_and_tail_indices_of_the_list,
            natural_16_bit const max_index_of_any_synapse_in_the_list
            );

    natural_16_bit head() const;
    natural_16_bit tail() const;

    void set_head(natural_16_bit const index);
    void set_tail(natural_16_bit const index);

    natural_16_bit max_index_of_any_synapse() const;

//    void push_back();
//    void push_front();
//    void pop_back();
//    void pop_front();

private:
    natural_16_bit* m_pointer_to_pair_of_head_and_tail_indices_of_the_list;
    natural_16_bit m_max_index_of_any_synapse;
};

struct reference_to_synapses_in_territory_of_cell
{
    reference_to_synapses_in_territory_of_cell(
            natural_16_bit* const array_of_seven_pairs_of_head_and_tail_synapse_indices,
            natural_8_bit* const pointer_to_memory_with_bits_of_all_synapses,
            natural_8_bit const seek_in_first_byte_in_memory_with_synapses,
            natural_8_bit const num_bits_per_synapse,
            natural_16_bit const total_number_of_synapses_in_the_territory
            );

    bits_reference find_bits_of_synapse(
            natural_16_bit const index_of_synapse_in_range_from_0_to_number_of_synapses_minus_1);

    natural_16_bit number_of_synapses() const;
    natural_8_bit num_bits_per_synapse() const;

    reference_to_list_of_indices_of_synampses indices_of_connected_synapses();
    reference_to_list_of_indices_of_synampses indices_of_synapses_to_be_moved_along_positive_x_axis();
    reference_to_list_of_indices_of_synampses indices_of_synapses_to_be_moved_along_negative_x_axis();
    reference_to_list_of_indices_of_synampses indices_of_synapses_to_be_moved_along_positive_y_axis();
    reference_to_list_of_indices_of_synampses indices_of_synapses_to_be_moved_along_negative_y_axis();
    reference_to_list_of_indices_of_synampses indices_of_synapses_to_be_moved_along_positive_columnar_axis();
    reference_to_list_of_indices_of_synampses indices_of_synapses_to_be_moved_along_negative_columnar_axis();

private:
    natural_16_bit* m_array_of_seven_pairs_of_head_and_tail_synapse_indices;
    natural_8_bit* m_bits_of_all_synapses;
    natural_8_bit m_seek_in_first_byte_in_memory_with_synapses;
    natural_8_bit m_num_bits_per_synapse;
    natural_16_bit m_number_of_synapses;
};

struct dynamic_state_of_neural_tissue : private boost::noncopyable
{
    dynamic_state_of_neural_tissue(
            std::shared_ptr<static_state_of_neural_tissue const> const pointer_to_static_state_of_neural_tissue
            );

    std::shared_ptr<static_state_of_neural_tissue const> get_static_state_of_neural_tissue() const;

    bits_reference find_bits_of_cell(
            natural_32_bit const seek_along_x_axis,
            natural_32_bit const seek_along_y_axis,
            natural_32_bit const seek_along_columnar_axis
            );
    reference_to_synapses_in_territory_of_cell find_synapses_in_territory_of_cell(
            natural_32_bit const seek_along_x_axis,
            natural_32_bit const seek_along_y_axis,
            natural_32_bit seek_along_columnar_axis
            );

    bits_reference find_bits_of_sensory_cell(natural_32_bit const index_of_sensory_cell);
    bits_reference find_bits_of_synapse_to_muscle(natural_32_bit const index_of_synapse_to_muscle);

private:
    std::shared_ptr<static_state_of_neural_tissue const> m_static_state_of_neural_tissue;
    boost::scoped_array<natural_8_bit> m_bits_of_all_cells_organised_in_3D_array_with_appendix_of_sensory_cells;
    natural_32_bit m_num_synapses_in_all_cell_territories_along_columnar_axis;
    boost::scoped_array<natural_8_bit>
        m_bits_of_synapses_in_teritories_of_all_cells_organised_in_3D_array_with_appendix_of_synapses_to_muscles;
    boost::scoped_array<natural_16_bit>
        m_heads_and_tails_of_seven_lists_of_indices_of_synapses_in_territories_of_all_cells_organised_in_3D_array;
};


}

#endif
