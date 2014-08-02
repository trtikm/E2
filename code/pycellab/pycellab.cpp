#include <cellab/static_state_of_cell.hpp>
#include <cellab/static_state_of_synapse.hpp>
#include <cellab/static_state_of_neural_tissue.hpp>
#include <utility/log.hpp>
#include <boost/python.hpp>
#include <vector>
#include <memory>

LOG_INITIALISE("log-pycellab",true,true)

using namespace boost::python;

static void push_back_ushort(std::vector<unsigned short>& VEC, unsigned short const value)
{
    VEC.push_back(value);
}

static void push_back_static_state_of_cell(std::vector<cellab::static_state_of_cell>& VEC,
        unsigned short num_synapses_in_territory_of_cell)
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
        unsigned char const num_kinds_of_cells_in_neural_tissue,
        unsigned char const num_kinds_of_sensory_cells,
        unsigned char const num_bits_per_cell,
        unsigned char const num_bits_per_synapse,
        unsigned int const num_tissue_cells_along_x_axis,
        unsigned int const num_tissue_cells_along_y_axis,
        std::vector<unsigned short> const&
            array_where_indices_are_kinds_of_tissue_cells_and_values_are_numbers_of_tissue_cells_in_column,
        std::vector<cellab::static_state_of_cell> const&
            array_where_indices_are_kinds_of_tissue_cells_and_values_are_static_states_of_tissue_cells,
        std::vector<cellab::static_state_of_cell> const&
            array_where_indices_are_kinds_of_sensory_cells_and_values_are_static_states_of_sensory_cells,
        std::vector<cellab::static_state_of_synapse> const&
            row_major_matrix_where_indices_are_pairs_of_kinds_of_tissue_cells_and_values_are_static_states_of_synapses,
        std::vector<cellab::static_state_of_synapse> const&
            array_where_indices_are_kinds_of_tissue_cells_and_values_are_static_states_of_synapses_to_muscles,
        unsigned int num_sensory_cells,
        unsigned int num_synapses_to_muscles
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

    unsigned char num_kinds_of_cells_in_neural_tissue() const
    { return m_data->num_kinds_of_cells_in_neural_tissue(); }

    unsigned char num_kinds_of_sensory_cells() const
    { return m_data->num_kinds_of_sensory_cells(); }

    unsigned char num_bits_per_cell() const
    { return m_data->num_bits_per_cell(); }

    unsigned char num_bits_per_synapse() const
    { return m_data->num_bits_per_synapse(); }

    unsigned int num_tissue_cells_along_x_axis() const
    { return m_data->num_tissue_cells_along_x_axis(); }

    unsigned int num_tissue_cells_along_y_axis() const
    { return m_data->num_tissue_cells_along_y_axis(); }

    unsigned int num_tissue_cells_along_columnar_axis() const
    { return m_data->num_tissue_cells_along_columnar_axis(); }

    unsigned short num_tissue_cells_in_column(unsigned char const kind_of_tissue_cell) const
    { return m_data->num_tissue_cells_in_column(kind_of_tissue_cell); }

    unsigned char compute_kind_of_tissue_cell_from_its_position_along_columnar_axis(
                        unsigned int position_of_tissue_cell_in_column) const
    {
        return m_data->compute_kind_of_tissue_cell_from_its_position_along_columnar_axis(
                            position_of_tissue_cell_in_column
                            );
    }

    cellab::static_state_of_cell get_static_state_of_tissue_cell(unsigned char const kind_of_tissue_cell) const
    { return m_data->get_static_state_of_tissue_cell(kind_of_tissue_cell); }

    cellab::static_state_of_cell get_static_state_of_sensory_cell(unsigned char const kind_of_sensory_cell) const
    { return m_data->get_static_state_of_sensory_cell(kind_of_sensory_cell); }

    cellab::static_state_of_synapse get_static_state_of_synapse(
            unsigned char const kind_of_source_tissue_cell,
            unsigned char const kind_of_target_tissue_cell) const
    { return m_data->get_static_state_of_synapse(kind_of_source_tissue_cell,kind_of_target_tissue_cell); }

    cellab::static_state_of_synapse get_static_state_of_synapse_to_muscle(
            unsigned char const kind_of_tissue_cell) const
    { return m_data->get_static_state_of_synapse_to_muscle(kind_of_tissue_cell); }

    unsigned int num_sensory_cells() const
    { return m_data->num_sensory_cells(); }

    unsigned int num_synapses_to_muscles() const
    { return m_data->num_synapses_to_muscles(); }

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
        .def("num_synapses_in_territory_of_cell",
             &cellab::static_state_of_cell::num_synapses_in_territory_of_cell)
        ;

    class_<cellab::static_state_of_synapse>("static_state_of_synapse")
        ;

    class_<ptr_to_static_state_of_neural_tissue>("static_state_of_neural_tissue"
        ,init<
            unsigned char const,
            unsigned char const,
            unsigned char const,
            unsigned char const,
            unsigned int const,
            unsigned int const,
            std::vector<unsigned short> const&,
            std::vector<cellab::static_state_of_cell> const&,
            std::vector<cellab::static_state_of_cell> const&,
            std::vector<cellab::static_state_of_synapse> const&,
            std::vector<cellab::static_state_of_synapse> const&,
            unsigned int,
            unsigned int
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
