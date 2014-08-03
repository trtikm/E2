#include <cellab/static_state_of_cell.hpp>
#include <cellab/static_state_of_synapse.hpp>
#include <cellab/static_state_of_neural_tissue.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/log.hpp>
#include <boost/python.hpp>
#include <vector>
#include <memory>

LOG_INITIALISE("log-pycellab",true,true)

using namespace boost::python;

static void push_back_natural_16_bit(std::vector<natural_16_bit>& VEC, natural_16_bit const value)
{
    VEC.push_back(value);
}

static void push_back_static_state_of_cell(std::vector<cellab::static_state_of_cell>& VEC,
        natural_16_bit num_synapses_in_territory_of_cell)
{
    VEC.push_back(
        cellab::static_state_of_cell(
            num_synapses_in_territory_of_cell
            )
        );
}

static void push_back_static_state_of_synapse(std::vector<cellab::static_state_of_synapse>& VEC)
{
    VEC.push_back(
        cellab::static_state_of_synapse(
            )
        );
}

struct ptr_to_static_state_of_neural_tissue
{
    ptr_to_static_state_of_neural_tissue(
        natural_8_bit const num_kinds_of_cells_in_neural_tissue,
        natural_8_bit const num_kinds_of_sensory_cells,
        natural_8_bit const num_bits_per_cell,
        natural_8_bit const num_bits_per_synapse,
        natural_32_bit const num_tissue_cells_along_x_axis,
        natural_32_bit const num_tissue_cells_along_y_axis,
        std::vector<natural_16_bit> const&
            array_where_indices_are_kinds_of_tissue_cells_and_values_are_numbers_of_tissue_cells_in_column,
        std::vector<cellab::static_state_of_cell> const&
            array_where_indices_are_kinds_of_tissue_cells_and_values_are_static_states_of_tissue_cells,
        std::vector<cellab::static_state_of_cell> const&
            array_where_indices_are_kinds_of_sensory_cells_and_values_are_static_states_of_sensory_cells,
        std::vector<cellab::static_state_of_synapse> const&
            row_major_matrix_where_indices_are_pairs_of_kinds_of_tissue_cells_and_values_are_static_states_of_synapses,
        std::vector<cellab::static_state_of_synapse> const&
            array_where_indices_are_kinds_of_tissue_cells_and_values_are_static_states_of_synapses_to_muscles,
        natural_32_bit num_sensory_cells,
        natural_32_bit num_synapses_to_muscles
        )
        : m_data(
            new cellab::static_state_of_neural_tissue(
                num_kinds_of_cells_in_neural_tissue,
                num_kinds_of_sensory_cells,
                num_bits_per_cell,
                num_bits_per_synapse,
                num_tissue_cells_along_x_axis,
                num_tissue_cells_along_y_axis,
                array_where_indices_are_kinds_of_tissue_cells_and_values_are_numbers_of_tissue_cells_in_column,
                array_where_indices_are_kinds_of_tissue_cells_and_values_are_static_states_of_tissue_cells,
                array_where_indices_are_kinds_of_sensory_cells_and_values_are_static_states_of_sensory_cells,
                row_major_matrix_where_indices_are_pairs_of_kinds_of_tissue_cells_and_values_are_static_states_of_synapses,
                array_where_indices_are_kinds_of_tissue_cells_and_values_are_static_states_of_synapses_to_muscles,
                num_sensory_cells,
                num_synapses_to_muscles
                )
            )
    {}

    natural_8_bit num_kinds_of_cells_in_neural_tissue() const
    { return m_data->num_kinds_of_cells_in_neural_tissue(); }

    natural_8_bit num_kinds_of_sensory_cells() const
    { return m_data->num_kinds_of_sensory_cells(); }

    natural_8_bit num_bits_per_cell() const
    { return m_data->num_bits_per_cell(); }

    natural_8_bit num_bits_per_synapse() const
    { return m_data->num_bits_per_synapse(); }

    natural_32_bit num_tissue_cells_along_x_axis() const
    { return m_data->num_tissue_cells_along_x_axis(); }

    natural_32_bit num_tissue_cells_along_y_axis() const
    { return m_data->num_tissue_cells_along_y_axis(); }

    natural_32_bit num_tissue_cells_along_columnar_axis() const
    { return m_data->num_tissue_cells_along_columnar_axis(); }

    natural_16_bit num_tissue_cells_in_column(natural_8_bit const kind_of_tissue_cell) const
    { return m_data->num_tissue_cells_in_column(kind_of_tissue_cell); }

    natural_8_bit compute_kind_of_tissue_cell_from_its_position_along_columnar_axis(
                        natural_32_bit position_of_tissue_cell_in_column) const
    {
        return m_data->compute_kind_of_tissue_cell_from_its_position_along_columnar_axis(
                            position_of_tissue_cell_in_column
                            );
    }

    cellab::static_state_of_cell get_static_state_of_tissue_cell(natural_8_bit const kind_of_tissue_cell) const
    { return m_data->get_static_state_of_tissue_cell(kind_of_tissue_cell); }

    cellab::static_state_of_cell get_static_state_of_sensory_cell(natural_8_bit const kind_of_sensory_cell) const
    { return m_data->get_static_state_of_sensory_cell(kind_of_sensory_cell); }

    cellab::static_state_of_synapse get_static_state_of_synapse(
            natural_8_bit const kind_of_source_tissue_cell,
            natural_8_bit const kind_of_target_tissue_cell) const
    { return m_data->get_static_state_of_synapse(kind_of_source_tissue_cell,kind_of_target_tissue_cell); }

    cellab::static_state_of_synapse get_static_state_of_synapse_to_muscle(
            natural_8_bit const kind_of_tissue_cell) const
    { return m_data->get_static_state_of_synapse_to_muscle(kind_of_tissue_cell); }

    natural_32_bit num_sensory_cells() const
    { return m_data->num_sensory_cells(); }

    natural_32_bit num_synapses_to_muscles() const
    { return m_data->num_synapses_to_muscles(); }

private:
    std::shared_ptr<cellab::static_state_of_neural_tissue> m_data;
};

BOOST_PYTHON_MODULE(pycellab)
{
    class_<std::vector<natural_16_bit> >("vector_of_natural_16_bit");
    def("push_back_natural_16_bit",&push_back_natural_16_bit);

    class_<std::vector<cellab::static_state_of_cell> >("vector_of_static_states_of_cells");
    def("push_back_static_state_of_cell",&push_back_static_state_of_cell);

    class_<std::vector<cellab::static_state_of_synapse> >("vector_of_static_states_of_synapses");
    def("push_back_static_state_of_synapse",&push_back_static_state_of_synapse);

    class_<cellab::static_state_of_cell>("static_state_of_cell"
        ,init<natural_32_bit>())
        .def("num_synapses_in_territory_of_cell",
             &cellab::static_state_of_cell::num_synapses_in_territory_of_cell)
        ;

    class_<cellab::static_state_of_synapse>("static_state_of_synapse")
        ;

    class_<ptr_to_static_state_of_neural_tissue>("static_state_of_neural_tissue"
        ,init<
            natural_8_bit const,
            natural_8_bit const,
            natural_8_bit const,
            natural_8_bit const,
            natural_32_bit const,
            natural_32_bit const,
            std::vector<natural_16_bit> const&,
            std::vector<cellab::static_state_of_cell> const&,
            std::vector<cellab::static_state_of_cell> const&,
            std::vector<cellab::static_state_of_synapse> const&,
            std::vector<cellab::static_state_of_synapse> const&,
            natural_32_bit,
            natural_32_bit
            >()
            )
        .def("num_kinds_of_cells_in_neural_tissue",
             &ptr_to_static_state_of_neural_tissue::num_kinds_of_cells_in_neural_tissue)
        .def("num_kinds_of_sensory_cells",
             &ptr_to_static_state_of_neural_tissue::num_kinds_of_sensory_cells)
        .def("num_bits_per_cell",
             &ptr_to_static_state_of_neural_tissue::num_bits_per_cell)
        .def("num_bits_per_synapse",
             &ptr_to_static_state_of_neural_tissue::num_bits_per_synapse)
        .def("num_tissue_cells_along_x_axis",
             &ptr_to_static_state_of_neural_tissue::num_tissue_cells_along_x_axis)
        .def("num_tissue_cells_along_y_axis",
             &ptr_to_static_state_of_neural_tissue::num_tissue_cells_along_y_axis)
        .def("num_tissue_cells_along_columnar_axis",
             &ptr_to_static_state_of_neural_tissue::num_tissue_cells_along_columnar_axis)
        .def("num_tissue_cells_in_column",
             &ptr_to_static_state_of_neural_tissue::num_tissue_cells_in_column)
        .def("compute_kind_of_tissue_cell_from_its_position_along_columnar_axis",
             &ptr_to_static_state_of_neural_tissue::compute_kind_of_tissue_cell_from_its_position_along_columnar_axis)
        .def("get_static_state_of_tissue_cell",
             &ptr_to_static_state_of_neural_tissue::get_static_state_of_tissue_cell)
        .def("get_static_state_of_sensory_cell",
             &ptr_to_static_state_of_neural_tissue::get_static_state_of_sensory_cell)
        .def("get_static_state_of_synapse",
             &ptr_to_static_state_of_neural_tissue::get_static_state_of_synapse)
        .def("get_static_state_of_synapse_to_muscle",
             &ptr_to_static_state_of_neural_tissue::get_static_state_of_synapse_to_muscle)
        .def("num_sensory_cells",
             &ptr_to_static_state_of_neural_tissue::num_sensory_cells)
        .def("num_synapses_to_muscles",
             &ptr_to_static_state_of_neural_tissue::num_synapses_to_muscles)
        ;
}
