#ifndef CELLAB_DYNAMIC_STATE_OF_NEURAL_TISSUE_HPP_INCLUDED
#   define CELLAB_DYNAMIC_STATE_OF_NEURAL_TISSUE_HPP_INCLUDED

#   include <cellab/static_state_of_neural_tissue.hpp>
#   include <cellab/homogenous_slice_of_tissue.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <utility/array_of_bit_units.hpp>
#   include <utility/bits_reference.hpp>
#   include <boost/multiprecision/cpp_int.hpp>
#   include <boost/noncopyable.hpp>
#   include <vector>
#   include <memory>

namespace cellab {


/**
 * It defines that part of a state of the neural tissue which can be modified (updated)
 * by transition algorithms (see the header file 'transition_algorithms.hpp'). An instance
 * of this structure is constructed according to data in the other part of a state of the neural
 * tissue called 'static_state_of_neural_tissue' (see the constructor). After construction all
 * memory is allocated for all components of the neural tissue and there are also initialised
 * access mechanism to the memory of the components. Nevertheless, the memory of individual components
 * (like tissue cells, sensory cells, sinapses in tissue, synapses to muscles, and signalling) is NOT
 * initialised. All these components have to be initialised manually by a user. The user calls methods
 * (defined bellow) to get access to memory of individual components (via a bits_reference) and
 * the user then writes to the memory proper data. For example, a tissue cell at coordinates (x,y,c)
 * can be initialised as follows:
 *
 *     // Let 'dyn_state' be a pointer to an instance of 'struct dynamic_state_of_neural_tissue'.
 *     // Let 'my_cell_type' be a user defined type representing a tissue cell (of a some kind).
 *
 *     // First we get an access to the memory allocated for the cell.
 *     bits_reference  bref = dyn_state->find_bits_of_cell(x,y,c);
 *     // Next we construct an instance of cell whose data should be stored into the memory.
 *     my_cell_type  my_cell; // Let the default constructor builds an initial state of the cell.
 *     // Finally, we write the data in 'my_cell' into the memory in a compressed form.
 *     my_cell >> bref; // we typically implement an operator>> which simply serialises 'my_cell'
 *                      // into the memory referenced by 'bref'. Of course, 'my_cell' can be
 *                      // stored in the memory in any form we want. We may consider to use some
 *                      // compression method on 'my_cell' in order to save memory. Note that it
 *                      // is the user who specifies size of the memory. It is done through
 *                      // 'static_state_of_neural_tissue' whose instance is passed to the constructor
 *                      // of 'dynamic_state_of_neural_tissue'.
 *
 * Details about structure of a dynamic state of the neural tissue can be found in the documentation:
 *      file:///<E2-root-dir>/doc/project_documentation/cellab/cellab.html#dynamic_state
 *      file:///<E2-root-dir>/doc/project_documentation/cellab/cellab.html#dynamic_state_memory_rep
 *
 */
struct dynamic_state_of_neural_tissue : private boost::noncopyable
{
    /**
     * It allocates the memory for all components of the neural tissue and initialises access mechanism
     * to the memory of the components. Nevertheless, the memory of components themselves is NOT initialised.
     * It has to be done manually as described above. Use methods bellow to access memory of individual components.
     *
     * A constructed instance will take a shared ownership of the passed instance of 'static_state_of_neural_tissue'.
     */
    dynamic_state_of_neural_tissue(
            std::shared_ptr<static_state_of_neural_tissue const> const pointer_to_static_state_of_neural_tissue
            );

    /**
     * Releases all memory allocated during the construction.
     */
    ~dynamic_state_of_neural_tissue();

    /**
     * Here is a collection of constants representing the other part of a state of the neural tissue.
     * It returns a pointet to the same structure which was passed to the constructor.
     */
    std::shared_ptr<static_state_of_neural_tissue const>  get_static_state_of_neural_tissue() const;


    bits_reference  find_bits_of_cell(
            natural_32_bit const coord_along_x_axis,
            natural_32_bit const coord_along_y_axis,
            kind_of_cell const cell_kind,
            natural_32_bit const relative_index_of_cell
                    //!< It is an index (ordinal) into the list of cells of the kind 'cell_kind'.
            );

    bits_reference  find_bits_of_cell_in_tissue(
            natural_32_bit const coord_along_x_axis,
            natural_32_bit const coord_along_y_axis,
            natural_32_bit const coord_along_columnar_axis
            );

    /**
     * The first three parameters identify a territory of a tissue cell the synapse is located in. The
     * last parameter is an index (ordinal) of the component in the list of components of that kind in that
     * territory.
     */
    bits_reference  find_bits_of_synapse_in_tissue(
            natural_32_bit const coord_to_cell_along_x_axis,
            natural_32_bit const coord_to_cell_along_y_axis,
            natural_32_bit const coord_to_cell_along_columnar_axis,
            natural_32_bit const index_of_synapse_in_territory_of_cell
            );

    /**
     * The first three parameters identify a territory of a tissue cell the territorial state of synapse is located in.
     * The last parameter is an index (ordinal) into the list of territorial states in that territory.
     */
    bits_reference  find_bits_of_territorial_state_of_synapse_in_tissue(
            natural_32_bit const coord_to_cell_along_x_axis,
            natural_32_bit const coord_to_cell_along_y_axis,
            natural_32_bit const coord_to_cell_along_columnar_axis,
            natural_32_bit const index_of_synapse_in_territory_of_cell
            );

    /**
     * The first three parameters identify a territory of a tissue cell the coordinates is located in.
     * The last parameter is an index (ordinal) of coordinates (triple x,y,c) in the list of coordinates
     * in that territory.
     */
    bits_reference  find_bits_of_coords_of_source_cell_of_synapse_in_tissue(
            natural_32_bit const coord_to_cell_along_x_axis,
            natural_32_bit const coord_to_cell_along_y_axis,
            natural_32_bit const coord_to_cell_along_columnar_axis,
            natural_32_bit const index_of_synapse_in_territory_of_cell
            );

    /**
     * The parameters identify a territory of a tissue cell the signalling is located in.
     */
    bits_reference  find_bits_of_signalling(
            natural_32_bit const coord_to_cell_along_x_axis,
            natural_32_bit const coord_to_cell_along_y_axis,
            natural_32_bit const coord_to_cell_along_columnar_axis
            );

    /**
     * The first three parameters identify a territory in the tissue cell the delimiters list is located in. The
     * last parameter is an index (ordinal) of the delimiter in the delimiters list in that territory.
     * See definition of the method for assumptions for the arguments.
     */
    bits_reference  find_bits_of_delimiter_between_territorial_lists(
            natural_32_bit const coord_to_cell_along_x_axis,
            natural_32_bit const coord_to_cell_along_y_axis,
            natural_32_bit const coord_to_cell_along_columnar_axis,
            natural_8_bit const index_of_delimiter
            );

    bits_reference  find_bits_of_sensory_cell(natural_32_bit const index_of_sensory_cell);

    bits_reference  find_bits_of_synapse_to_muscle(natural_32_bit const index_of_synapse_to_muscle);
    bits_reference  find_bits_of_coords_of_source_cell_of_synapse_to_muscle(
            natural_32_bit const index_of_synapse_to_muscle
            );

    natural_8_bit  num_bits_per_source_cell_coordinate() const;
    natural_8_bit  num_bits_per_delimiter_number(kind_of_cell const  kind_of_tissue_cell) const;

private:
    typedef std::shared_ptr<homogenous_slice_of_tissue> pointer_to_homogenous_slice_of_tissue;

    std::shared_ptr<static_state_of_neural_tissue const> m_static_state_of_neural_tissue;
    natural_8_bit m_num_bits_per_source_cell_coordinate;
    std::vector<natural_8_bit> m_num_bits_per_delimiter_number;
    std::vector<pointer_to_homogenous_slice_of_tissue> m_slices_of_cells;
    std::vector<pointer_to_homogenous_slice_of_tissue> m_slices_of_synapses;
    std::vector<pointer_to_homogenous_slice_of_tissue> m_slices_of_territorial_states_of_synapses;
    std::vector<pointer_to_homogenous_slice_of_tissue> m_slices_of_source_cell_coords_of_synapses;
    std::vector<pointer_to_homogenous_slice_of_tissue> m_slices_of_signalling_data;
    std::vector<pointer_to_homogenous_slice_of_tissue> m_slices_of_delimiters_between_territorial_lists;
    array_of_bit_units m_bits_of_sensory_cells;
    array_of_bit_units m_bits_of_synapses_to_muscles;
    array_of_bit_units m_bits_of_source_cell_coords_of_synapses_to_muscles;
};

/**
 * Possible territorial states of a synapse are defined in the header file 'territorial_state_of_synapse.hpp'
 * as an enum type. This fuction computes a lowest 1-byte aligned number of bits which are necessary to store
 * the states in the memory.
 */
natural_16_bit  num_of_bits_to_store_territorial_state_of_synapse();

/**
 * There are exactly 7 lists of synapses in each territory of the neural tissue. They can be separated in
 * the memory using 6 delimiters. So, this function returns the number 6.
 */
natural_8_bit  num_delimiters();


/**
 * The remaining two functions allows a user of the neural tissue to estimate memory consumption of a
 * dynamic state before it is physically allocated.
 */

boost::multiprecision::int128_t  compute_num_bits_of_dynamic_state_of_neural_tissue_with_checked_operations(
        static_state_of_neural_tissue const& static_state_of_tissue
        );

boost::multiprecision::int128_t  compute_num_bits_of_dynamic_state_of_neural_tissue_with_checked_operations(
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_ptr
        );


}

#endif
