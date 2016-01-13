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
 * by transition algorithms. An instance is constructed according to data in the other
 * part of a state (see the constructor). After construction all memory is allocated
 * for all components of the neural tissue and there are also initialised access mechanism
 * to the memory of the components. Nevertheless, the memory of components is NOT initialised.
 * Individual components (like cells, signallings, synapses) all have to be initialised
 * manually using methods providing access (i.e. bits_references) to memory allocated for
 * them. For example, a tissue cell at coordinates (x,y,c) can be initialised as follows:
 *
 *     // Let 'dyn_state' be a pointer to an instance of 'struct dynamic_state_of_neural_tissue'.
 *     // Let 'my_cell_type' be our (user defined) type representing a tissue cell of a desired kind.
 *
 *     // First we get an access to the memory allocated for the cell.
 *     bits_reference  bref = dyn_state->find_bits_of_cell(x,y,c);
 *     // Next we construct an instance of cell whose data should be stored into the memory.
 *     my_cell_type  my_cell; // The default construction means an initial state of the cell.
 *     // Finally, we write the data in 'my_cell' into the memory in a compressed form.
 *     my_cell >> bref; // This is the common way: the write is implemented as a user defined
 *                      // operator>>. The typical compression method is the serialisation.
 *
 * Details about structure of a state of the neural tissue can be found in the documentation:
 *      file:///<E2-root-dir>/doc/project_documentation/cellab/cellab.html#dynamic_state
 *      file:///<E2-root-dir>/doc/project_documentation/cellab/cellab.html#dynamic_state_memory_rep
 *
 */
struct dynamic_state_of_neural_tissue : private boost::noncopyable
{
    /**
     * It allocates the memory for all components of the neural tissue and initialises access mechanism
     * to the memory of the components. Nevertheless, the memory of components themselves is NOT initialised.
     * It has to be done manually using access methods listed bellow.
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

    /**
     * The parameter 'relative_index_of_cell' is an index (ordinal) into the list of cells of the
     * kind 'cell_kind'.
     */
    bits_reference  find_bits_of_cell(
            natural_32_bit const coord_along_x_axis,
            natural_32_bit const coord_along_y_axis,
            kind_of_cell const cell_kind,
            natural_32_bit const relative_index_of_cell
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


natural_16_bit  num_of_bits_to_store_territorial_state_of_synapse();

natural_8_bit  num_delimiters();

boost::multiprecision::int128_t  compute_num_bits_of_dynamic_state_of_neural_tissue_with_checked_operations(
        static_state_of_neural_tissue const& static_state_of_tissue
        );

boost::multiprecision::int128_t  compute_num_bits_of_dynamic_state_of_neural_tissue_with_checked_operations(
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_ptr
        );


}

#endif
