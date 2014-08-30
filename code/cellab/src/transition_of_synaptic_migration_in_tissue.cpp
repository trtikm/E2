#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <cellab/static_state_of_neural_tissue.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/assumptions.hpp>
#include <memory>

namespace cellab {

struct shift_in_coordinates
{
    shift_in_coordinates(integer_8_bit const shift_along_x_axis,
                         integer_8_bit const shift_along_y_axis,
                         integer_8_bit const shift_along_columnar_axis);
    integer_8_bit get_shift_along_x_axis() const;
    integer_8_bit get_shift_along_y_axis() const;
    integer_8_bit get_shift_along_columnar_axis() const;
private:
    integer_8_bit m_shift_along_x_axis;
    integer_8_bit m_shift_along_y_axis;
    integer_8_bit m_shift_along_columnar_axis;
};

enum migration_attempt_of_synapse
{
    CONNECT_TO_TERRITORY_CELL = 0,
    DISCONNECT_FROM_TERRITORY_CELL = 1,
    MOVE_ALONG_POSITIVE_X_AXIS = 2,
    MOVE_ALONG_NEGATIVE_X_AXIS = 3,
    MOVE_ALONG_POSITIVE_Y_AXIS = 4,
    MOVE_ALONG_NEGATIVE_Y_AXIS = 5,
    MOVE_ALONG_POSITIVE_COLUMNAR_AXIS = 6,
    MOVE_ALONG_NEGATIVE_COLUMNAR_AXIS = 7
};

}

namespace cellab { namespace private_interal_implementation_details {


extern void move_synapses_into_proper_lists_by_their_migration_attempts_in_territories_of_all_cells(
        std::shared_ptr<dynamic_state_of_neural_tissue const> const dynamic_state_of_tissue,
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue,
        natural_32_bit num_avalilable_thread_for_creation_and_use
        );

extern void exchange_synapses_in_lists_in_territories_of_all_cells(
        std::shared_ptr<dynamic_state_of_neural_tissue const> const dynamic_state_of_tissue,
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue,
        natural_8_bit const list_index_in_pivot_cell,
        shift_in_coordinates const& shift,
        natural_8_bit const list_index_in_shift_cell,
        natural_32_bit num_avalilable_thread_for_creation_and_use
        );


}}


namespace cellab {


void apply_transition_of_synaptic_migration_in_tissue(
        std::shared_ptr<dynamic_state_of_neural_tissue const> const dynamic_state_of_tissue,
        natural_32_bit num_avalilable_thread_for_creation_and_use
        )
{
    std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue =
            dynamic_state_of_tissue->get_static_state_of_neural_tissue();

    using cellab_private = cellab::private_interal_implementation_details;

    cellab_private::move_synapses_into_proper_lists_by_their_migration_attempts_in_territories_of_all_cells(
                dynamic_state_of_tissue,
                static_state_of_tissue,
                num_avalilable_thread_for_creation_and_use
                );
    cellab_private::exchange_synapses_in_lists_in_territories_of_all_cells(
                dynamic_state_of_tissue,
                static_state_of_tissue,
                dynamic_state_of_tissue->get_index_of_migration_list(CONNECT_TO_TERRITORY_CELL),
                shift_in_coordinates(0,0,0),
                dynamic_state_of_tissue->get_index_of_migration_list(DISCONNECT_FROM_TERRITORY_CELL),
                num_avalilable_thread_for_creation_and_use
                );
    cellab_private::exchange_synapses_in_lists_in_territories_of_all_cells(
                dynamic_state_of_tissue,
                static_state_of_tissue,
                dynamic_state_of_tissue->get_index_of_migration_list(MOVE_ALONG_POSITIVE_X_AXIS),
                shift_in_coordinates(1,0,0),
                dynamic_state_of_tissue->get_index_of_migration_list(MOVE_ALONG_NEGATIVE_X_AXIS),
                num_avalilable_thread_for_creation_and_use
                );
    cellab_private::exchange_synapses_in_lists_in_territories_of_all_cells(
                dynamic_state_of_tissue,
                static_state_of_tissue,
                dynamic_state_of_tissue->get_index_of_migration_list(MOVE_ALONG_POSITIVE_Y_AXIS),
                shift_in_coordinates(0,1,0),
                dynamic_state_of_tissue->get_index_of_migration_list(MOVE_ALONG_NEGATIVE_Y_AXIS),
                num_avalilable_thread_for_creation_and_use
                );
    cellab_private::exchange_synapses_in_lists_in_territories_of_all_cells(
                dynamic_state_of_tissue,
                static_state_of_tissue,
                dynamic_state_of_tissue->get_index_of_migration_list(MOVE_ALONG_POSITIVE_COLUMNAR_AXIS),
                shift_in_coordinates(0,0,1),
                dynamic_state_of_tissue->get_index_of_migration_list(MOVE_ALONG_NEGATIVE_COLUMNAR_AXIS),
                num_avalilable_thread_for_creation_and_use
                );
}


}
