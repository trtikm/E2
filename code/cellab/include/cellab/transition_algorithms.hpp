#ifndef CELLAB_TRANSITION_ALGORITHMS_HPP_INCLUDED
#   define CELLAB_TRANSITION_ALGORITHMS_HPP_INCLUDED

#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <cellab/territorial_state_of_synapse.hpp>
#include <cellab/shift_in_coordinates.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/bits_reference.hpp>
#include <functional>
#include <memory>


/**
 * This module intoruces 6 algorithms which together provide s single transition function
 * which is able to update the current state of the neural tissue (represented by the pair
 * static and dynamic state) to the next one. The algorithm has to be applied to the
 * neural tissue sequentially in the order as they are listed in this header file. If they
 * are applied in a different order the resulting state is undefined. Also note that states
 * of the neural tissue between call of the algorithms is only intermediate, i.e. inconsistent.
 * So, you always have to call all six algorithms (in the exact order) in order to receive
 * a next valid state from a given valid state. Finally, the algorithms overwrite the old
 * state of the neural tissue by the next one.
 *
 * For more info read about the algorithms see the documentation:
 *      file:///<E2-root-dir>/doc/project_documentation/cellab/cellab.html#transition_algorithms
 */


namespace cellab {


/**
 * It defines a prototype of a user-defined callback function which is called for each synapse to
 * muscle from the algorithm 'apply_transition_of_synapses_to_muscles' (see its declaration bellow)
 * in order to compute a next state of the synapse from the current one.
 *
 * We assume the call-back function implement some single-threaded, thread-safe, and in-place algorithm.
 *
 * For more info read the documentation:
 *      file:///<E2-root-dir>/doc/project_documentation/cellab/cellab.html#algorithm_synapses_to_muscles
 */
typedef std::function<
            void(
                bits_reference& bits_of_synapse_to_be_updated,
                        //!< It is a reference to the the memory where the current state of the updated synapse
                        //!< to muscle is stored. The size, and content of this memory is defined by the
                        //!< user. We thus assume that the function already has (or is build on) that information.
                        //!< The function is supposed to compute a next state from the state encoded here.
                        //!< Once the next state is computed the function have to write that state to
                        //!< this memory, i.e. the function overwrites the old state by the computed one.
                kind_of_synapse_to_muscle kind_of_synapse_to_muscle_to_be_updated,
                        //!< This information may be helpful or even necessary for the function to correctly
                        //!< interpret the content of the memory referenced by 'bits_of_synapse_to_be_updated'.
                        //!< It can, for example, be used for a selection of a particular sub-routine, which will
                        //!< actually compute the next state of the considered synapse to muscle.
                kind_of_cell kind_of_source_cell,
                        //!< A synapse to muscle is always related to a cell in the tissue. The cell represents
                        //!< a source of information for the synapse about how to update its current state.
                bits_const_reference const& bits_of_source_cell
                        //!< It is a reference to the the memory where the current state of the tissue cell
                        //!< related to the updated synapse to muscle is stored. It serves as a source of information
                        //!< about how to update the current state of the synapse to muscle to the next one.
                        //!< The size, and content of the referenced memory is defined by the user. We thus assume that
                        //!< the function already has (or is build on) that information.
                ) >
        single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_synapse_to_muscle;


/**
 * It defines a prototype of a user-defined callback function which is called for each synapse in the
 * tissue from the algorithm 'apply_transition_of_synapses_of_tissue' (see its declaration bellow)
 * in order to compute a next state of the synapse from the current one.
 *
 * We assume the call-back function implement some single-threaded, thread-safe, and in-place algorithm.
 *
 * For more info read the documentation:
 *      file:///<E2-root-dir>/doc/project_documentation/cellab/cellab.html#algorithm_synapses_in_tissue
 */
typedef std::function<
            territorial_state_of_synapse(
                        //!< The function is supposed to return a new (updated) territorial state of the
                        //!< processed synapse (see the header file 'territorial_state_of_synapse.hpp' for
                        //!< info about territorial states).
                bits_reference& bits_of_synapse_to_be_updated,
                        //!< It is a reference to the the memory where the current state of the updated synapse
                        //!< in the tissue is stored. The size, and content of this memory is defined by the
                        //!< user. We thus assume that the function already has (or is build on) that information.
                        //!< The function is supposed to compute a next state from the state encoded here.
                        //!< Once the next state is computed the function have to write that state to
                        //!< this memory, i.e. the function overwrites the old state by the computed one.
                kind_of_cell kind_of_source_cell,
                        //!< A synapse in the tissue is always related to a source cell in the tissue. The cell
                        //!< represents a source of information for the synapse about how to update its current state.
                bits_const_reference const& bits_of_source_cell,
                        //!< It is a reference to the the memory where the current state of the source tissue
                        //!< cell of the updated synapse is stored. It serves as a source of information about how
                        //!< to update the current state of the synapse to the next one. The size, and content of
                        //!< the referenced memory is defined by a user. We thus assume that the function already
                        //!< has (or is build on) that information.
                kind_of_cell kind_of_territory_cell,
                        //!< A synapse in the tissue always appears in some territory and there it can participate
                        //!< in information delivery to the tissue cell appearing in that territory as well. This
                        //!< parameter provides information about the kind of the target tissue cell.
                bits_const_reference const& bits_of_territory_cell,
                        //!< A synapse in the tissue always appears in some territory and there it can participate in
                        //!< information delivery to the tissue cell appearing in that territory as well. This target cell
                        //!< may thus represent a source of information for the synapse about how to update its current state.
                        //!< This parameter is a reference to the the memory where the current state of the target tissue
                        //!< cell of the updated synapse is stored. The size, and content of the referenced memory is defined
                        //!< by a user. We thus assume that the function already has (or is build on) that information.
                territorial_state_of_synapse current_territorial_state_of_synapse,
                        //!< This numeric constant specifies the current behaviour of the synapse in the territory:
                        //!< whether it delivers signals to the target cell (appearing in the same territory as the
                        //!< synapse), or whether the synapse only passes through the territory. See the header
                        //!< 'territorial_state_of_synapse.hpp' for more info.
                shift_in_coordinates const& shift_to_low_corner,
                        //!< This is a 3D vector pointing from the tissue coordinates of the current territory (i.e.
                        //!< the territory where the synapse currently appears) to a territory with lowest possible
                        //!< tissue coordinates which can be accessed from the user's callback function by calling
                        //!< the function 'get_signalling' discussed bellow.
                shift_in_coordinates const& shift_to_high_corner,
                        //!< This is a 3D vector pointing from the tissue coordinates of the current territory (i.e.
                        //!< the territory where the synapse currently appears) to a territory with highest possible
                        //!< tissue coordinates which can be accessed from the user's callback function by calling
                        //!< the function 'get_signalling' discussed bellow.
                std::function<std::pair<bits_const_reference,kind_of_cell>(shift_in_coordinates const&)> const&
                    get_signalling
                        //!< It is a function accepting a 3D vector pointing from the current territory
                        //!< (i.e. the territory where the synapse currently appears) to any territory
                        //!< within a region (box) given by the corner vectors 'shift_to_low_corner' and
                        //!< 'shift_to_high_corner' discussed above. The current territory lies
                        //!< in the center of that box. Said it differently, each coordinate of a vector passed
                        //!< to this function must be within the range of coordinates given by the
                        //!< corresponding coordinates of both mentioned corner vectors. The function returns
                        //!< a pair whose the first element is a reference to the memory where is stored
                        //!< the signalling (see the documentation, namely the paragraph:
                        //!<    file:///<E2-root-dir>/doc/project_documentation/cellab/cellab.html#signalling
                        //!< of the section:
                        //!<    file:///<E2-root-dir>/doc/project_documentation/cellab.html#structure_of_column
                        //!< ) of the territory referenced by the passed vector, and the second argument is
                        //!< the kind of that territory.
                        //!< Note that it is the parametrisation who defines size, content, and a compression method
                        //!< of the memory reserved for any signalling. We thus assume that the user's callback function
                        //!< already has (or is build on) that information.
                ) >
        single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_synapse_inside_tissue;


/**
 * It defines a prototype of a user-defined callback function which is called for each signalling in the
 * tissue from the algorithm 'apply_transition_of_signalling_in_tissue' (see its declaration bellow)
 * in order to compute a next state of the signalling from the current one.
 *
 * We assume the call-back function implement some single-threaded, thread-safe, and in-place algorithm.
 *
 * For more info read the documentation:
 *      file:///<E2-root-dir>/doc/project_documentation/cellab/cellab.html#algorithm_signalling
 */
typedef std::function<
            void(
                bits_reference& bits_of_signalling_data_to_be_updated,
                        //!< It is a reference to the the memory where the current state of the updated signalling
                        //!< in the tissue is stored. The size, and content of this memory is defined by the
                        //!< user. We thus assume that the function already has (or is build on) that information.
                        //!< The function is supposed to compute a next state from the state encoded here.
                        //!< Once the next state is computed the function have to write that new state to
                        //!< this memory, i.e. the function overwrites the old state by the computed one.
                kind_of_cell kind_of_territory_cell,
                        //!< This information may be helpful or even necessary for the function to correctly
                        //!< interpret the content of the memory referenced by 'bits_of_signalling_data_to_be_updated'.
                        //!< It can, for example, be used for a selection of a particular sub-routine, which will
                        //!< actually compute the next state of the considered signalling.
                shift_in_coordinates const& shift_to_low_corner,
                        //!< This is a 3D vector pointing from the tissue coordinates of the current territory (i.e.
                        //!< the territory where the signalling appears) to a territory with lowest possible
                        //!< tissue coordinates which can be accessed from the user's callback function by calling
                        //!< the function 'get_cell' discussed bellow.
                shift_in_coordinates const& shift_to_high_corner,
                        //!< This is a 3D vector pointing from the tissue coordinates of the current territory (i.e.
                        //!< the territory where the signalling appears) to a territory with highest possible
                        //!< tissue coordinates which can be accessed from the user's callback function by calling
                        //!< the function 'get_cell' discussed bellow.
                std::function<std::pair<bits_const_reference,kind_of_cell>(shift_in_coordinates const&)> const&
                    get_cell
                        //!< It is a function accepting a 3D vector pointing from the current territory
                        //!< (i.e. the territory where the signalling appears) to any territory
                        //!< within a region (box) given by the corner vectors 'shift_to_low_corner' and
                        //!< 'shift_to_high_corner' discussed above. The current territory lies
                        //!< in the center of that box. Said it differently, each coordinate of a vector passed
                        //!< to this function must be within the range of coordinates given by the
                        //!< corresponding coordinates of both mentioned corner vectors. The function returns
                        //!< a pair whose the first element is a reference to the memory where is stored
                        //!< the cell (see the documentation, namely the paragraph:
                        //!<    file:///<E2-root-dir>/doc/project_documentation/cellab/cellab.html#tissue_cell
                        //!< of the section:
                        //!<    file:///<E2-root-dir>/doc/project_documentation/cellab.html#structure_of_column
                        //!< ) of the territory referenced by the passed vector, and the second argument is
                        //!< the kind of that territory (i.e. the kind of that cell).
                        //!< Note that it is the parametrisation who defines size, content, and a compression method
                        //!< of the memory reserved for any cell. We thus assume that the user's callback function
                        //!< already has (or is build on) that information.
                ) >
        single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_signalling;


/**
 * It defines a prototype of a user-defined callback function which is called for each cell in the
 * tissue from the algorithm 'apply_transition_of_cells_of_tissue' (see its declaration bellow)
 * in order to compute a next state of the cell from the current one.
 *
 * We assume the call-back function implement some single-threaded, thread-safe, and in-place algorithm.
 *
 * For more info read the documentation:
 *      file:///<E2-root-dir>/doc/project_documentation/cellab/cellab.html#algorithm_cells_in_tissue
 */
typedef std::function<
            void(
                bits_reference& bits_of_cell_to_be_updated,
                        //!< It is a reference to the the memory where the current state of the updated cell
                        //!< in the tissue is stored. The size, and content of this memory is defined by the
                        //!< user. We thus assume that the function already has (or is build on) that information.
                        //!< The function is supposed to compute a next state from the state encoded here.
                        //!< Once the next state is computed the function have to write that new state to
                        //!< this memory, i.e. the function overwrites the old state by the computed one.
                kind_of_cell kind_of_cell_to_be_updated,
                        //!< This information may be helpful or even necessary for the function to correctly
                        //!< interpret the content of the memory referenced by 'bits_of_cell_to_be_updated'.
                        //!< It can, for example, be used for a selection of a particular sub-routine, which will
                        //!< actually compute the next state of the considered cell.
                natural_32_bit num_of_synapses_connected_to_the_cell,
                        //!< It is a number of all those synapses in the territory of the updated cell, whose
                        //!< territorial state is 0 (see the header file 'territorial_state_of_synapse.hpp').
                std::function<std::tuple<bits_const_reference,kind_of_cell,kind_of_cell>(natural_32_bit)> const&
                    get_connected_synapse_at_index,
                        //!< This functions allows the user enumerate all synapses connected to the updated cell.
                        //!< A synapse is identified by an index in the range 0,...,'num_of_synapses_connected_to_the_cell'-1.
                        //!< The function returns a reference to the memory where the synapse is stored, the kind
                        //!< of source cell of the enumerated synapse, and the kind of the target cell of the synapse.
                        //!< Since the synapse is connected to the updated cell, the kind of the target cell is equal
                        //!< to 'kind_of_cell_to_be_updated'. Note that the size, and content of synapse's memory
                        //!< is defined by the user. We thus assume that user's function already has that information.
                        //!< The function which is passed through this parameter to the user's callback function is
                        //!< the function 'get_synapse_callback_function' declared in the header file
                        //!< 'utilities_for_transition_algorithms.hpp'.
                shift_in_coordinates const& shift_to_low_corner,
                        //!< This is a 3D vector pointing from the tissue coordinates of the current territory (i.e.
                        //!< the territory where the cell currently appears) to a territory with lowest possible
                        //!< tissue coordinates which can be accessed from the user's callback function by calling
                        //!< the function 'get_signalling' discussed bellow.
                shift_in_coordinates const& shift_to_high_corner,
                        //!< This is a 3D vector pointing from the tissue coordinates of the current territory (i.e.
                        //!< the territory where the cell currently appears) to a territory with highest possible
                        //!< tissue coordinates which can be accessed from the user's callback function by calling
                        //!< the function 'get_signalling' discussed bellow.
                std::function<std::pair<bits_const_reference,kind_of_cell>(shift_in_coordinates const&)> const&
                    get_signalling
                        //!< It is a function accepting a 3D vector pointing from the current territory
                        //!< (i.e. the territory where the cell currently appears) to any territory
                        //!< within a region (box) given by the corner vectors 'shift_to_low_corner' and
                        //!< 'shift_to_high_corner' discussed above. The current territory lies
                        //!< in the center of that box. Said it differently, each coordinate of a vector passed
                        //!< to this function must be within the range of coordinates given by the
                        //!< corresponding coordinates of both mentioned corner vectors. The function returns
                        //!< a pair whose the first element is a reference to the memory where is stored
                        //!< the signalling (see the documentation, namely the paragraph:
                        //!<    file:///<E2-root-dir>/doc/project_documentation/cellab/cellab.html#signalling
                        //!< of the section:
                        //!<    file:///<E2-root-dir>/doc/project_documentation/cellab.html#structure_of_column
                        //!< ) of the territory referenced by the passed vector, and the second argument is
                        //!< the kind of that territory.
                        //!< Note that it is the parametrisation who defines size, content, and a compression method
                        //!< of the memory reserved for any signalling. We thus assume that the user's callback function
                        //!< already has (or is build on) that information.
                ) >
        single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_cell;


/**
 * This algorithm represent the first of the six steps in computation of a next state of the neural
 * tissue from the current one. The algorithm enumerates all synapses to muscles in the neural tissue
 * and for each of them it calls the passed user-defined callback function. This function is actualy
 * responsible to compute a next state of the updated synapse from the current one, see the definition
 * of the type 'single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_synapse_to_muscle'
 * above.
 *
 * For more info read the documentation:
 *      file:///<E2-root-dir>/doc/project_documentation/cellab/cellab.html#algorithm_synapses_to_muscles
 */
void apply_transition_of_synapses_to_muscles(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
                //!< The neural tissue whose states of synapses to muscles will be updated.
        single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_synapse_to_muscle const&
            transition_function_of_packed_synapse_to_muscle,
                //!< A user-defined callback function which will be called for each synapse to muscle.
                //!< The function must be single-threaded, thread-safe, and in-place (i.e. in situ).
                //!< It is responsible for the actual computation of a next state of an updated synapse
                //!< to muscle from the current one.
        natural_32_bit const  num_threads_avalilable_for_computation
        );


/**
 * This algorithm represent the second of the six steps in computation of a next state of the neural
 * tissue from the current one. The algorithm enumerates all synapses in the neural tissue
 * and for each of them it calls the passed user-defined callback function. This function is actualy
 * responsible to compute a next state of the updated synapse from the current one, see the definition
 * of the type 'single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_synapse_inside_tissue'
 * above.
 *
 * For more info read the documentation:
 *      file:///<E2-root-dir>/doc/project_documentation/cellab/cellab.html#algorithm_synapses_in_tissue
 */
void apply_transition_of_synapses_of_tissue(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
                //!< The neural tissue whose states of synapses in the tissue will be updated.
        single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_synapse_inside_tissue const&
            transition_function_of_packed_synapse_inside_tissue,
                //!< A user-defined callback function which will be called for each synapse in the tissue.
                //!< The function must be single-threaded, thread-safe, and in-place (i.e. in situ).
                //!< It is responsible for the actual computation of a next state of an updated synapse
                //!< from the current one.
        natural_32_bit const  num_threads_avalilable_for_computation
        );

void apply_transition_of_territorial_lists_of_synapses(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        natural_32_bit const  num_threads_avalilable_for_computation
        );

void  apply_transition_of_synaptic_migration_in_tissue(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        natural_32_bit const  num_threads_avalilable_for_computation
        );

/**
 * This algorithm represent the fifth of the six steps in computation of a next state of the neural
 * tissue from the current one. The algorithm enumerates all singallings in territories of tissue cells
 * and for each of them it calls the passed user-defined callback function. This function is actualy
 * responsible to compute a next state of the updated signalling from the current one, see the definition
 * of the type 'single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_signalling' above.
 *
 * For more info read the documentation:
 *      file:///<E2-root-dir>/doc/project_documentation/cellab/cellab.html#algorithm_signalling
 */
void apply_transition_of_signalling_in_tissue(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_signalling const&
            transition_function_of_packed_signalling,
        natural_32_bit const  num_threads_avalilable_for_computation
        );


/**
 * This algorithm represent the last of the six steps in computation of a next state of the neural
 * tissue from the current one. The algorithm enumerates all tissue cells in the neural tissue
 * and for each of them it calls the passed user-defined callback function. This function is actualy
 * responsible to compute a next state of the updated cell from the current one, see the definition
 * of the type 'single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_cell' above.
 *
 * For more info read the documentation:
 *      file:///<E2-root-dir>/doc/project_documentation/cellab/cellab.html#algorithm_cells_in_tissue
 */
void apply_transition_of_cells_of_tissue(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
                //!< The neural tissue whose states of tissue cells will be updated.
        single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_cell const&
            transition_function_of_packed_cell,
                //!< A user-defined callback function which will be called for each cell in the tissue.
                //!< The function must be single-threaded, thread-safe, and in-place (i.e. in situ).
                //!< It is responsible for the actual computation of a next state of an updated cell
                //!< from the current one.
        natural_32_bit const  num_threads_avalilable_for_computation
        );


}

#endif
