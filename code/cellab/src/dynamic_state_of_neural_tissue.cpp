#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <utility/checked_number_operations.hpp>
#include <utility/assumptions.hpp>
#include <utility/log.hpp>


//static natural_8_bit compute_num_of_bits_to_store_number(natural_32_bit number)
//{
//    natural_8_bit num_bits = 1;
//    do
//    {
//        number >>= 1;
//        ++num_bits;
//    }
//    while (number != 0U);
//    return num_bits;
//}


//namespace cellab {


//dynamic_state_of_neural_tissue::dynamic_state_of_neural_tissue(
//        std::shared_ptr<static_state_of_neural_tissue const> const pointer_to_static_state_of_neural_tissue
//        )
//    : m_static_state_of_neural_tissue(pointer_to_static_state_of_neural_tissue)
//    , m_slices_of_tissue_cells(m_static_state_of_neural_tissue->num_kinds_of_cells_in_neural_tissue())
//    , m_slices_of_tissue_synapses(m_static_state_of_neural_tissue->num_kinds_of_cells_in_neural_tissue())
//    , m_slices_of_tissue_signalling_data(m_static_state_of_neural_tissue->num_kinds_of_cells_in_neural_tissue())
//    , m_slices_of_tissue_migration_data(m_static_state_of_neural_tissue->num_kinds_of_cells_in_neural_tissue())
//    , m_num_bits_per_number_in_migration(m_static_state_of_neural_tissue->num_kinds_of_cells_in_neural_tissue())
//    , m_bits_of_sensory_cells(m_static_state_of_neural_tissue->num_bits_per_cell(),
//                              m_static_state_of_neural_tissue->num_sensory_cells())
//    , m_bits_of_synapses_to_muscles(m_static_state_of_neural_tissue->num_bits_per_synapse(),
//                                    m_static_state_of_neural_tissue->num_synapses_to_muscles())
//{
//    for (natural_16_bit i = 0U; i < m_static_state_of_neural_tissue->num_kinds_of_cells_in_neural_tissue(); ++i)
//    {
//        m_slices_of_tissue_cells.at(i) =
//                pointer_to_homogenous_slice_of_tissue(
//                    new homogenous_slice_of_tissue(
//                        m_static_state_of_neural_tissue->num_bits_per_cell(),
//                        m_static_state_of_neural_tissue->num_tissue_cells_along_x_axis(),
//                        m_static_state_of_neural_tissue->num_tissue_cells_along_y_axis(),
//                        m_static_state_of_neural_tissue->num_tissue_cells_in_column_of_cell_kind(i)
//                        )
//                    );
//        m_slices_of_tissue_synapses.at(i) =
//                pointer_to_homogenous_slice_of_tissue(
//                    new homogenous_slice_of_tissue(
//                        m_static_state_of_neural_tissue->num_bits_per_synapse(),
//                        m_static_state_of_neural_tissue->num_tissue_cells_along_x_axis(),
//                        m_static_state_of_neural_tissue->num_tissue_cells_along_y_axis(),
//                        checked_mul_64_bit(
//                            m_static_state_of_neural_tissue->num_tissue_cells_in_column_of_cell_kind(i),
//                            m_static_state_of_neural_tissue->get_static_state_of_tissue_cell(i)
//                                                           .num_synapses_in_territory_of_cell()
//                            )
//                        )
//                    );
//        m_slices_of_tissue_signalling_data.at(i) =
//                pointer_to_homogenous_slice_of_tissue(
//                    new homogenous_slice_of_tissue(
//                        m_static_state_of_neural_tissue->num_bits_per_signalling(),
//                        m_static_state_of_neural_tissue->num_tissue_cells_along_x_axis(),
//                        m_static_state_of_neural_tissue->num_tissue_cells_along_y_axis(),
//                        m_static_state_of_neural_tissue->num_tissue_cells_in_column_of_cell_kind(i)
//                        )
//                    );
//        m_num_bits_per_number_in_migration.at(i) =
//                compute_num_of_bits_to_store_number(
//                    m_static_state_of_neural_tissue->get_static_state_of_tissue_cell(i)
//                                                   .num_synapses_in_territory_of_cell()
//                    );
//        m_slices_of_tissue_migration_data.at(i) =
//                pointer_to_homogenous_slice_of_tissue(
//                    new homogenous_slice_of_tissue(
//                        natural_16_bit(8) * m_num_bits_per_number_in_migration.at(i),
//                        m_static_state_of_neural_tissue->num_tissue_cells_along_x_axis(),
//                        m_static_state_of_neural_tissue->num_tissue_cells_along_y_axis(),
//                        m_static_state_of_neural_tissue->num_tissue_cells_in_column_of_cell_kind(i)
//                        )
//                    );
//    }

//    LOG(debug,FUNCTION_PROTOTYPE());
//}

//dynamic_state_of_neural_tissue::~dynamic_state_of_neural_tissue()
//{
//    LOG(debug,FUNCTION_PROTOTYPE());
//}

//boost::multiprecision::int128_t compute_num_bits_of_dynamic_state_of_neural_tissue_with_checked_operations(
//        static_state_of_neural_tissue const& static_state_of_tissue
//        )
//{
//    boost::multiprecision::int128_t num_bits = 0ULL;

//    num_bits += compute_num_bits_of_all_array_units_with_checked_operations(
//                        static_state_of_tissue.num_bits_per_cell(),
//                        static_state_of_tissue.num_sensory_cells()
//                        );
//    num_bits += compute_num_bits_of_all_array_units_with_checked_operations(
//                        static_state_of_tissue.num_bits_per_synapse(),
//                        static_state_of_tissue.num_synapses_to_muscles()
//                        );

//    for (natural_16_bit i = 0U; i < static_state_of_tissue.num_kinds_of_cells_in_neural_tissue(); ++i)
//    {
//        num_bits += compute_num_bits_of_slice_of_tissue_with_checked_operations(
//                        static_state_of_tissue.num_bits_per_cell(),
//                        static_state_of_tissue.num_tissue_cells_along_x_axis(),
//                        static_state_of_tissue.num_tissue_cells_along_y_axis(),
//                        static_state_of_tissue.num_tissue_cells_in_column_of_cell_kind(i)
//                        );
//        num_bits += compute_num_bits_of_slice_of_tissue_with_checked_operations(
//                        static_state_of_tissue.num_bits_per_synapse(),
//                        static_state_of_tissue.num_tissue_cells_along_x_axis(),
//                        static_state_of_tissue.num_tissue_cells_along_y_axis(),
//                        checked_mul_64_bit(
//                            static_state_of_tissue.num_tissue_cells_in_column_of_cell_kind(i),
//                            static_state_of_tissue.get_static_state_of_tissue_cell(i)
//                                                  .num_synapses_in_territory_of_cell()
//                            )
//                        );
//        num_bits += compute_num_bits_of_slice_of_tissue_with_checked_operations(
//                        static_state_of_tissue.num_bits_per_signalling(),
//                        static_state_of_tissue.num_tissue_cells_along_x_axis(),
//                        static_state_of_tissue.num_tissue_cells_along_y_axis(),
//                        static_state_of_tissue.num_tissue_cells_in_column_of_cell_kind(i)
//                        );
//        num_bits += compute_num_bits_of_slice_of_tissue_with_checked_operations(
//                        natural_16_bit(8) * compute_num_of_bits_to_store_number(
//                                                    static_state_of_tissue.get_static_state_of_tissue_cell(i)
//                                                                          .num_synapses_in_territory_of_cell()
//                                                    ),
//                        static_state_of_tissue.num_tissue_cells_along_x_axis(),
//                        static_state_of_tissue.num_tissue_cells_along_y_axis(),
//                        static_state_of_tissue.num_tissue_cells_in_column_of_cell_kind(i)
//                        );
//    }

//    return num_bits;
//}

//boost::multiprecision::int128_t compute_num_bits_of_dynamic_state_of_neural_tissue_with_checked_operations(
//        std::shared_ptr<static_state_of_neural_tissue const> const static_state_ptr
//        )
//{
//    ASSUMPTION(static_state_ptr.operator bool());
//    return compute_num_bits_of_dynamic_state_of_neural_tissue_with_checked_operations(*static_state_ptr.get());
//}


//}
