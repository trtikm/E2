#ifndef CELLAB_DYNAMIC_STATE_OF_NEURAL_TISSUE_HPP_INCLUDED
#   define CELLAB_DYNAMIC_STATE_OF_NEURAL_TISSUE_HPP_INCLUDED

#   include <cellab/static_state_of_neural_tissue.hpp>
#   include <cellab/bits_reference.hpp>
#   include <boost/noncopyable.hpp>
#   include <boost/scoped_array.hpp>
#   include <array>
#   include <tuple>
#   include <memory>

namespace cellab {


struct reference_to_list_of_indices_of_synampses
{
    reference_to_list_of_indices_of_synampses(
            std::pair<unsigned short,unsigned short>* const pointer_to_pair_of_head_and_tail_indices_of_the_list,
            unsigned short max_index_of_any_synapse_in_the_list
            );

    unsigned short head() const;
    unsigned short tail() const;

    void set_head(unsigned short const index);
    void set_tail(unsigned short const index);

    unsigned short max_index_of_any_synapse() const;

//    void push_back();
//    void push_front();
//    void pop_back();
//    void pop_front();

private:
    std::pair<unsigned short,unsigned short>* m_pair_of_head_and_tail_indices;
    unsigned short m_max_index_of_any_synapse;
};

struct reference_to_synapses_in_territory_of_cell
{
    reference_to_synapses_in_territory_of_cell(
            std::array<std::pair<unsigned short,unsigned short>,7>* const
                pointer_to_seven_lists_of_indices_of_synapses,
            unsigned char* const pointer_to_memory_with_bits_of_all_synapses,
            unsigned short total_number_of_synapses_in_the_territory,
            unsigned char num_bits_per_synapse
            );

    bits_reference find_bits_of_synapse(unsigned short const index_of_synapse /* 0..(number_of_synapses()-1) */);
    unsigned short number_of_synapses() const;

    reference_to_list_of_indices_of_synampses indices_of_connected_synapses();
    reference_to_list_of_indices_of_synampses indices_of_synapses_to_be_moved_along_positive_x_axis();
    reference_to_list_of_indices_of_synampses indices_of_synapses_to_be_moved_along_negative_x_axis();
    reference_to_list_of_indices_of_synampses indices_of_synapses_to_be_moved_along_positive_y_axis();
    reference_to_list_of_indices_of_synampses indices_of_synapses_to_be_moved_along_negative_y_axis();
    reference_to_list_of_indices_of_synampses indices_of_synapses_to_be_moved_along_positive_columnar_axis();
    reference_to_list_of_indices_of_synampses indices_of_synapses_to_be_moved_along_negative_columnar_axis();

private:
    std::array<std::pair<unsigned short,unsigned short>,7>* m_seven_lists_of_indices_of_synapses;
    unsigned char* m_bits_of_all_synapses;
    unsigned short m_number_of_synapses;
    unsigned char m_num_bits_per_synapse;
};

struct dynamic_state_of_neural_tissue : private boost::noncopyable
{
    dynamic_state_of_neural_tissue(
            std::shared_ptr<static_state_of_neural_tissue const> const pointer_to_static_state_of_neural_tissue
            );

    std::shared_ptr<static_state_of_neural_tissue const> get_static_state_of_neural_tissue() const;

    bits_reference find_bits_of_cell(
            unsigned int const seek_along_x_axis,
            unsigned int const seek_along_y_axis,
            unsigned int const seek_along_columnar_axis
            );
    reference_to_synapses_in_territory_of_cell find_synapses_in_territory_of_cell(
            unsigned int const seek_along_x_axis,
            unsigned int const seek_along_y_axis,
            unsigned int const seek_along_columnar_axis
            );
private:
    std::shared_ptr<static_state_of_neural_tissue const> m_static_state_of_neural_tissue;
    boost::scoped_array<unsigned char> m_bits_of_all_cells_organised_in_3D_array;
    boost::scoped_array<unsigned char> m_bits_of_all_synapses_of_all_cells_organised_in_4D_array;
    boost::scoped_array<std::array<std::pair<unsigned short,unsigned short>,7> >
        m_heads_and_tails_of_seven_lists_of_indices_of_synapses_in_territories_of_all_cells_organised_in_3D_array;
};


}

#endif
