#ifndef CELLAB_UTILITIES_FOR_TRANSITION_ALGORITHMS_HPP_INCLUDED
#   define CELLAB_UTILITIES_FOR_TRANSITION_ALGORITHMS_HPP_INCLUDED

#   include <cellab/static_state_of_neural_tissue.hpp>
#   include <cellab/dynamic_state_of_neural_tissue.hpp>
#   include <cellab/shift_in_coordinates.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <utility/bits_reference.hpp>
#   include <memory>
#   include <tuple>

namespace cellab {


/**
 * It is just 3D vector.
 */
struct tissue_coordinates
{
    tissue_coordinates(
            natural_32_bit const coord_along_x_axis,
            natural_32_bit const coord_along_y_axis,
            natural_32_bit const coord_along_columnar_axis
            );
    natural_32_bit  get_coord_along_x_axis() const;
    natural_32_bit  get_coord_along_y_axis() const;
    natural_32_bit  get_coord_along_columnar_axis() const;
private:
    natural_32_bit  m_coord_along_x_axis;
    natural_32_bit  m_coord_along_y_axis;
    natural_32_bit  m_coord_along_columnar_axis;
};


/**
 * A spatial neighbourhood of some reference (central) point in the neural tissue is a set of tissue
 * coordinates defined relative to that point. The neighbourhood always has a shape of a box. So, it is
 * sufficient to specify it by two shift vectors from the central points to two extreme points of the
 * box (lying on the main diagonal of the box).
 */
struct spatial_neighbourhood
{
    spatial_neighbourhood(
            tissue_coordinates const& center,
                    //!< It is a reference point inside the defined neighbourhood. It does NOT have to necessarily
                    //!< lie in a geometrical center of the box (defined by the following two shifts to its extreme
                    //!< corners).
            shift_in_coordinates const& shift_to_low_corner,
            shift_in_coordinates const& shift_to_high_corner
            );
    tissue_coordinates const& get_center_of_neighbourhood() const;
    shift_in_coordinates const& get_shift_to_low_corner() const;
    shift_in_coordinates const& get_shift_to_high_corner() const;
private:
    tissue_coordinates m_center_of_neighbourhood;
    shift_in_coordinates m_shift_to_low_corner;
    shift_in_coordinates m_shift_to_high_corner;
};


bool operator==(tissue_coordinates const& left, tissue_coordinates const& right);

natural_32_bit  shift_coordinate_in_torus_axis(
        integer_64_bit coordinate,
        integer_64_bit const shift,
        natural_64_bit const length_of_axis
        );

natural_32_bit  shift_coordinate(
        natural_32_bit const coord,
        integer_64_bit const shift,
        natural_32_bit const length_of_axis,
        bool is_totus_axis
        );

tissue_coordinates  shift_coordinates(
        tissue_coordinates const& coords,
        shift_in_coordinates const& shift,
        natural_32_bit const num_cells_along_x_axis,
        natural_32_bit const num_cells_along_y_axis,
        natural_32_bit const num_cells_along_columnar_axis
        );



/**
 * The following three functions allows for enumeration of indices into an m-dimensional
 * array such that from a given coordinates (an m-tuple) they compute a successor coordinates
 * (another m-tuple) acoording to a given extent. For details see the documentation:
 *
 *      file:///<E2-root-dir>/doc/project_documentation/cellab/cellab.html#parallel_enumeration
 */

/** Enumeration of indices into a 3-dimension array. */
bool  go_to_next_coordinates(
        natural_32_bit& x_coord, natural_32_bit& y_coord, natural_32_bit& c_coord,
        natural_32_bit const extent,
        natural_32_bit const num_cells_along_x_axis,
        natural_32_bit const num_cells_along_y_axis,
        natural_32_bit const num_cells_along_columnar_axis
        );

/** Enumeration of indices into a 2-dimension array. */
bool  go_to_next_column(
        natural_32_bit& x_coord, natural_32_bit& y_coord,
        natural_32_bit const extent,
        natural_32_bit const num_cells_along_x_axis,
        natural_32_bit const num_cells_along_y_axis
        );

/** Enumeration of indices into a 1-dimension array. */
bool  go_to_next_index(
        natural_32_bit& index,
        natural_32_bit const extent,
        natural_32_bit const size
        );



integer_64_bit  clip_shift(
        integer_64_bit const shift,
        natural_32_bit const origin,
        natural_32_bit const length_of_axis
        );

integer_8_bit  clip_shift(
        integer_8_bit const shift,
        natural_32_bit const origin,
        natural_32_bit const length_of_axis,
        bool const is_it_torus_axis
        );

void  write_tissue_coordinates_to_bits_of_coordinates(tissue_coordinates const& coords, bits_reference& bits_ref);
tissue_coordinates  convert_bits_of_coordinates_to_tissue_coordinates(bits_reference const& bits_ref);

tissue_coordinates  get_coordinates_of_source_cell_of_synapse_in_tissue(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        tissue_coordinates const& coords_of_territorial_cell_of_synapse,
        natural_32_bit const index_of_synapse_in_territory
        );

tissue_coordinates  get_coordinates_of_source_cell_of_synapse_to_muscle(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        natural_32_bit const index_of_synapse_to_muscle
        );

natural_32_bit  get_begin_index_of_territorial_list_of_cell(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        tissue_coordinates const& coordinates_of_cell,
        natural_8_bit const index_of_territorial_list
        );
natural_32_bit  get_end_index_of_territorial_list_of_cell(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue,
        tissue_coordinates const& coordinates_of_cell,
        natural_8_bit const index_of_territorial_list
        );

void  swap_all_data_of_two_synapses(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        tissue_coordinates const& first_cell_coordinates,
        natural_32_bit const synapse_index_in_first_territory,
        tissue_coordinates const& second_cell_coordinates,
        natural_32_bit const synapse_index_in_second_territory
        );


/**
 * It is passed to user's callback function (from inside of either 'apply_transition_of_cells_of_tissue' or
 * 'apply_transition_of_synapses_of_tissue') in order to allow easy access all signalling within a local
 * neighbourhood of the territory inside which the updated cell or synapse appears. First three parameters
 * of this function are bound to fixed values (using std::bind inside both functions mentioned aboce), so
 * user's callback function provides only one (the last) argument, which is a shift vector from the current
 * territory into a desired one (within the neughbourhood). The shift vector is relative from the coordinates
 * of the current territory. So, the shift to the current territory is a sift vector (0,0,0). Note that the
 * neighbourhood is also passed to the user's callback function through two parameters 'shift_to_low_corner'
 * and 'shift_to_high_corner' identifying extreme corners of the neighbourhood. See the definition of
 * 'single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_cell' in the header file
 * 'thransition_algorithms.hpp' to see the prototype of the user's callback function.
 */
std::pair<bits_const_reference,kind_of_cell>  get_signalling_callback_function(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue,
        spatial_neighbourhood const& neighbourhood,
        shift_in_coordinates const& shift
        );


/**
 * It is passed to user's callback function (from inside of the function 'apply_transition_of_cells_of_tissue')
 * in order to allow easy access to synapses connected to an updated tissue cell. First six parameters of
 * this function are bound to fixed values (using std::bind inside 'apply_transition_of_cells_of_tissue'),
 * so user's callback function provides only one (the last) argument, which is an index of an enumerated
 * synapse. It is supposed to be in the range 0,...,'number_of_synapses_in_range'-1. Note that the value
 * 'number_of_synapses_in_range' is also passed to the user's callback function through another parameter.
 * See definition of 'single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_cell' in the
 * header file 'thransition_algorithms.hpp' to see the prototype of the user's callback function.
 */
std::tuple<bits_const_reference,kind_of_cell,kind_of_cell>  get_synapse_callback_function(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue,
        tissue_coordinates const& target_cell,
        kind_of_cell const kind_of_target_cell,
        natural_32_bit const number_of_synapses_in_range,
        natural_32_bit const shift_to_start_index,
        natural_32_bit const shift_from_start_index
        );

std::pair<bits_const_reference,kind_of_cell>  get_cell_callback_function(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue,
        spatial_neighbourhood const& neighbourhood,
        shift_in_coordinates const& shift
        );


}

#endif
