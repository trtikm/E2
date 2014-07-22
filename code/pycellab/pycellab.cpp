#include <cellab/static_state_of_cell.hpp>
#include <cellab/static_state_of_synapse.hpp>
#include <cellab/static_state_of_neural_tissue.hpp>
#include <utility/log.hpp>
#include <boost/python.hpp>
#include <vector>
#include <memory>

LOG_INITIALISE("pycellab.log")

using namespace boost::python;

static void push_back_ushort(std::vector<unsigned short>& VEC, unsigned short const value)
{
    VEC.push_back(value);
}

static void push_back_static_state_of_cell(std::vector<cellab::static_state_of_cell>& VEC,
        unsigned short max_number_of_synapses_in_territory_of_cell)
{
    VEC.push_back(
        cellab::static_state_of_cell(
            max_number_of_synapses_in_territory_of_cell
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
        unsigned char const num_kinds_of_cells,
        unsigned char const num_bits_per_cell,
        unsigned char const num_bits_per_synapse,
        unsigned int const num_cells_along_x_axis,
        unsigned int const num_cells_along_y_axis,
        std::vector<unsigned short> const&
            array_where_indices_are_kinds_of_cells_and_values_are_numbers_of_cells_in_column,
        std::vector<cellab::static_state_of_cell> const&
            array_where_indices_are_kinds_of_cells_and_values_are_static_states_of_cells,
        std::vector<cellab::static_state_of_synapse> const&
            row_major_matrix_where_indices_are_pairs_of_kinds_of_cells_and_values_are_static_states_of_synapses
        )
        : m_data(
            new cellab::static_state_of_neural_tissue(
                num_kinds_of_cells,
                num_bits_per_cell,
                num_bits_per_synapse,
                num_cells_along_x_axis,
                num_cells_along_y_axis,
                array_where_indices_are_kinds_of_cells_and_values_are_numbers_of_cells_in_column,
                array_where_indices_are_kinds_of_cells_and_values_are_static_states_of_cells,
                row_major_matrix_where_indices_are_pairs_of_kinds_of_cells_and_values_are_static_states_of_synapses
                )
            )
    {}

    unsigned char num_kinds_of_cells() const
    { return m_data->num_kinds_of_cells(); }

    unsigned char num_bits_per_cell() const
    { return m_data->num_bits_per_cell(); }

    unsigned char num_bits_per_synapse() const
    { return m_data->num_bits_per_synapse(); }

    unsigned int num_cells_along_x_axis() const
    { return m_data->num_cells_along_x_axis(); }

    unsigned int num_cells_along_y_axis() const
    { return m_data->num_cells_along_y_axis(); }

    unsigned int num_cells_along_columnar_axis() const
    { return m_data->num_cells_along_columnar_axis(); }

    unsigned short num_cells_in_column(unsigned char const kind_of_cell) const
    { return m_data->num_cells_in_column(kind_of_cell); }

    unsigned char compute_kind_of_cell_from_its_position_along_columnar_axis(
                        unsigned int position_of_cell_in_column) const
    { return m_data->compute_kind_of_cell_from_its_position_along_columnar_axis(position_of_cell_in_column); }

    cellab::static_state_of_cell get_static_state_of_cell(unsigned char const kind_of_cell) const
    { return m_data->get_static_state_of_cell(kind_of_cell); }

    cellab::static_state_of_synapse get_static_state_of_synapse(unsigned char const kind_of_source_cell,
                                                                unsigned char const kind_of_target_cell) const
    { return m_data->get_static_state_of_synapse(kind_of_source_cell,kind_of_target_cell); }

private:
    std::shared_ptr<cellab::static_state_of_neural_tissue> m_data;
};

BOOST_PYTHON_MODULE(pycellab)
{
    class_<std::vector<unsigned short> >("vector_of_ushorts");
    def("push_back_ushort",&push_back_ushort);

    class_<std::vector<cellab::static_state_of_cell> >("vector_of_static_states_of_cells");
    def("push_back_static_state_of_cell",&push_back_static_state_of_cell);

    class_<std::vector<cellab::static_state_of_synapse> >("vector_of_static_states_of_synapses");
    def("push_back_static_state_of_synapse",&push_back_static_state_of_synapse);

    class_<cellab::static_state_of_cell>("static_state_of_cell"
        ,init<unsigned int>())
        .def("max_number_of_synapses_in_territory_of_cell",
             &cellab::static_state_of_cell::max_number_of_synapses_in_territory_of_cell)
        ;

    class_<cellab::static_state_of_synapse>("static_state_of_synapse")
        ;

    class_<ptr_to_static_state_of_neural_tissue>("static_state_of_neural_tissue"
        ,init<
            unsigned char,
            unsigned char,
            unsigned char,
            unsigned int,
            unsigned int,
            std::vector<unsigned short>,
            std::vector<cellab::static_state_of_cell>,
            std::vector<cellab::static_state_of_synapse>
            >()
            )
        .def("num_kinds_of_cells",
             &ptr_to_static_state_of_neural_tissue::num_kinds_of_cells)
        .def("num_bits_per_cell",
             &ptr_to_static_state_of_neural_tissue::num_bits_per_cell)
        .def("num_bits_per_synapse",
             &ptr_to_static_state_of_neural_tissue::num_bits_per_synapse)
        .def("num_cells_along_x_axis",
             &ptr_to_static_state_of_neural_tissue::num_cells_along_x_axis)
        .def("num_cells_along_y_axis",
             &ptr_to_static_state_of_neural_tissue::num_cells_along_y_axis)
        .def("num_cells_along_columnar_axis",
             &ptr_to_static_state_of_neural_tissue::num_cells_along_columnar_axis)
        .def("num_cells_in_column",
             &ptr_to_static_state_of_neural_tissue::num_cells_in_column)
        .def("compute_kind_of_cell_from_its_position_along_columnar_axis",
             &ptr_to_static_state_of_neural_tissue::compute_kind_of_cell_from_its_position_along_columnar_axis)
        .def("get_static_state_of_cell",
             &ptr_to_static_state_of_neural_tissue::get_static_state_of_cell)
        .def("get_static_state_of_synapse",
             &ptr_to_static_state_of_neural_tissue::get_static_state_of_synapse)
        ;
}
