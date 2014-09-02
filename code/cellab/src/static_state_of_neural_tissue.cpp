#include <cellab/static_state_of_neural_tissue.hpp>
#include <utility/checked_number_operations.hpp>
#include <utility/assumptions.hpp>
#include <utility/log.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <algorithm>
#include <limits>

namespace cellab {


extern boost::multiprecision::int128_t
compute_num_bits_of_dynamic_state_of_neural_tissue_with_checked_operations(
            static_state_of_neural_tissue const& static_state_of_tissue
            );


static_state_of_neural_tissue::static_state_of_neural_tissue(
        natural_16_bit const num_kinds_of_cells,
        natural_16_bit const num_bits_per_cell,
        natural_16_bit const num_bits_per_synapse,
        natural_16_bit const num_bits_per_signalling,
        natural_32_bit const num_cells_along_x_axis,
        natural_32_bit const num_cells_along_y_axis,
        std::vector<natural_32_bit> const& num_cells_along_columnar_axis_of_cell_kind,
        std::vector<natural_32_bit> const& num_synapses_in_territory_of_cell_kind,
        natural_32_bit const num_sensory_cells,
        natural_32_bit const num_synapses_to_muscles,
        bool const is_x_axis_torus_axis,
        bool const is_y_axis_torus_axis,
        bool const is_columnar_axis_torus_axis,
        std::vector<integer_8_bit> const& x_radius_of_signalling_neighbourhood_of_cell,
        std::vector<integer_8_bit> const& y_radius_of_signalling_neighbourhood_of_cell,
        std::vector<integer_8_bit> const& columnar_radius_of_signalling_neighbourhood_of_cell,
        std::vector<integer_8_bit> const& x_radius_of_signalling_neighbourhood_of_synapse,
        std::vector<integer_8_bit> const& y_radius_of_signalling_neighbourhood_of_synapse,
        std::vector<integer_8_bit> const& columnar_radius_of_signalling_neighbourhood_of_synapse,
        std::vector<integer_8_bit> const& x_radius_of_cellular_neighbourhood_of_signalling,
        std::vector<integer_8_bit> const& y_radius_of_cellular_neighbourhood_of_signalling,
        std::vector<integer_8_bit> const& columnar_radius_of_cellular_neighbourhood_of_signalling
        )
    : m_num_kinds_of_cells(num_kinds_of_cells)
    , m_num_bits_per_cell(num_bits_per_cell)
    , m_num_bits_per_synapse(num_bits_per_synapse)
    , m_num_bits_per_signalling(num_bits_per_signalling)
    , m_num_cells_along_x_axis(num_cells_along_x_axis)
    , m_num_cells_along_y_axis(num_cells_along_y_axis)
    , m_num_cells_along_columnar_axis(
            [](std::vector<natural_32_bit> const& v)
            {
                ASSUMPTION(!v.empty());
                natural_32_bit sum = 0U;
                for (natural_32_bit i : v)
                {
                    ASSUMPTION(i > 0U);
                    sum = checked_add_32_bit(sum,i);
                }
                return sum;
            }(num_cells_along_columnar_axis_of_cell_kind)
        )
    , m_num_cells_along_columnar_axis_of_cell_kind(num_cells_along_columnar_axis_of_cell_kind)
    , m_num_synapses_in_territory_of_cell_kind(num_synapses_in_territory_of_cell_kind)
    , m_num_sensory_cells(num_sensory_cells)
    , m_num_synapses_to_muscles(num_synapses_to_muscles)
    , m_end_index_along_columnar_axis_of_cell_kind(
            [](std::vector<natural_32_bit> v)
            {
                for (natural_32_bit i = 1U; i < v.size(); ++i)
                    v.at(i) += v.at(i - 1);
                return v;
            }(num_cells_along_columnar_axis_of_cell_kind)
        )
    , m_is_x_axis_torus_axis(is_x_axis_torus_axis)
    , m_is_y_axis_torus_axis(is_y_axis_torus_axis)
    , m_is_columnar_axis_torus_axis(is_columnar_axis_torus_axis)
    , m_x_radius_of_signalling_neighbourhood_of_cell(x_radius_of_signalling_neighbourhood_of_cell)
    , m_y_radius_of_signalling_neighbourhood_of_cell(y_radius_of_signalling_neighbourhood_of_cell)
    , m_columnar_radius_of_signalling_neighbourhood_of_cell(columnar_radius_of_signalling_neighbourhood_of_cell)
    , m_x_radius_of_signalling_neighbourhood_of_synapse(x_radius_of_signalling_neighbourhood_of_synapse)
    , m_y_radius_of_signalling_neighbourhood_of_synapse(y_radius_of_signalling_neighbourhood_of_synapse)
    , m_columnar_radius_of_signalling_neighbourhood_of_synapse(columnar_radius_of_signalling_neighbourhood_of_synapse)
    , m_x_radius_of_cellular_neighbourhood_of_signalling(x_radius_of_cellular_neighbourhood_of_signalling)
    , m_y_radius_of_cellular_neighbourhood_of_signalling(y_radius_of_cellular_neighbourhood_of_signalling)
    , m_columnar_radius_of_cellular_neighbourhood_of_signalling(columnar_radius_of_cellular_neighbourhood_of_signalling)
{
    ASSUMPTION(m_num_kinds_of_cells > 0U);
    ASSUMPTION(m_num_kinds_of_cells < std::numeric_limits<natural_16_bit>::max());

    ASSUMPTION(m_num_bits_per_cell > 0U);
    ASSUMPTION(m_num_bits_per_cell < std::numeric_limits<natural_16_bit>::max());

    ASSUMPTION(m_num_bits_per_synapse > 0U);
    ASSUMPTION(m_num_bits_per_synapse < std::numeric_limits<natural_16_bit>::max());

    ASSUMPTION(m_num_bits_per_signalling > 0U);
    ASSUMPTION(m_num_bits_per_signalling < std::numeric_limits<natural_16_bit>::max());

    ASSUMPTION(m_num_cells_along_x_axis > 0U);
    ASSUMPTION(m_num_cells_along_x_axis < std::numeric_limits<natural_32_bit>::max());

    ASSUMPTION(m_num_cells_along_y_axis > 0U);
    ASSUMPTION(m_num_cells_along_y_axis < std::numeric_limits<natural_32_bit>::max());

    ASSUMPTION(m_num_cells_along_columnar_axis > 0U);
    ASSUMPTION(m_num_cells_along_columnar_axis < std::numeric_limits<natural_32_bit>::max());

    ASSUMPTION(m_num_kinds_of_cells == m_num_cells_along_columnar_axis_of_cell_kind.size());

    ASSUMPTION(m_num_kinds_of_cells == m_num_synapses_in_territory_of_cell_kind.size());
    ASSUMPTION(
            [](std::vector<natural_32_bit> const& v)
            {
                for (natural_32_bit value : v)
                    if (value == 0U || value == std::numeric_limits<natural_32_bit>::max())
                        return false;
                return true;
            }(m_num_synapses_in_territory_of_cell_kind)
        );

    ASSUMPTION(m_num_sensory_cells < std::numeric_limits<natural_32_bit>::max());
    ASSUMPTION(m_num_synapses_to_muscles < std::numeric_limits<natural_32_bit>::max());

    struct local
    {
        static bool check_radii(std::vector<integer_8_bit> const& v)
        {
            for (integer_8_bit value : v)
                if (value < 0 || value == std::numeric_limits<integer_8_bit>::max())
                    return false;
            return true;
        }
    };

    ASSUMPTION(m_num_kinds_of_cells == m_x_radius_of_signalling_neighbourhood_of_cell.size());
    ASSUMPTION( local::check_radii(m_x_radius_of_signalling_neighbourhood_of_cell) );

    ASSUMPTION(m_num_kinds_of_cells == m_y_radius_of_signalling_neighbourhood_of_cell.size());
    ASSUMPTION( local::check_radii(m_y_radius_of_signalling_neighbourhood_of_cell) );

    ASSUMPTION(m_num_kinds_of_cells == m_columnar_radius_of_signalling_neighbourhood_of_cell.size());
    ASSUMPTION( local::check_radii(m_columnar_radius_of_signalling_neighbourhood_of_cell) );

    ASSUMPTION(m_num_kinds_of_cells == m_x_radius_of_signalling_neighbourhood_of_synapse.size());
    ASSUMPTION( local::check_radii(m_x_radius_of_signalling_neighbourhood_of_synapse) );

    ASSUMPTION(m_num_kinds_of_cells == m_y_radius_of_signalling_neighbourhood_of_synapse.size());
    ASSUMPTION( local::check_radii(m_y_radius_of_signalling_neighbourhood_of_synapse) );

    ASSUMPTION(m_num_kinds_of_cells == m_columnar_radius_of_signalling_neighbourhood_of_synapse.size());
    ASSUMPTION( local::check_radii(m_columnar_radius_of_signalling_neighbourhood_of_synapse) );

    ASSUMPTION(m_num_kinds_of_cells == m_x_radius_of_cellular_neighbourhood_of_signalling.size());
    ASSUMPTION( local::check_radii(m_x_radius_of_cellular_neighbourhood_of_signalling) );

    ASSUMPTION(m_num_kinds_of_cells == m_y_radius_of_cellular_neighbourhood_of_signalling.size());
    ASSUMPTION( local::check_radii(m_y_radius_of_cellular_neighbourhood_of_signalling) );

    ASSUMPTION(m_num_kinds_of_cells == m_columnar_radius_of_cellular_neighbourhood_of_signalling.size());
    ASSUMPTION( local::check_radii(m_columnar_radius_of_cellular_neighbourhood_of_signalling) );

    // Now follows not only computes number of bits which will be taken to store dynamic state of
    // the neural tissue, but is also check for wrap error for unsigned integers which could
    // otherwise occure later when computing addresses of bits of individual element of the tissue in
    // an instance of dynamic_state_of_neural_tissue.
    // So, do NOT remove this code!!!!!!!!
    boost::multiprecision::int128_t const num_bits =
        compute_num_bits_of_dynamic_state_of_neural_tissue_with_checked_operations(*this);
    LOG(debug,"Dynamic state of the neural tissue will take at least " << num_bits << " bits when created.");

    LOG(debug,FUNCTION_PROTOTYPE());
}

static_state_of_neural_tissue::~static_state_of_neural_tissue()
{
    LOG(debug,FUNCTION_PROTOTYPE());
}

natural_16_bit  static_state_of_neural_tissue::num_kinds_of_cells() const
{
    return m_num_kinds_of_cells;
}

natural_16_bit  static_state_of_neural_tissue::num_bits_per_cell() const
{
    return m_num_bits_per_cell;
}

natural_16_bit  static_state_of_neural_tissue::num_bits_per_synapse() const
{
    return m_num_bits_per_synapse;
}

natural_16_bit  static_state_of_neural_tissue::num_bits_per_signalling() const
{
    return m_num_bits_per_signalling;
}

natural_32_bit  static_state_of_neural_tissue::num_cells_along_x_axis() const
{
    return m_num_cells_along_x_axis;
}

natural_32_bit  static_state_of_neural_tissue::num_cells_along_y_axis() const
{
    return m_num_cells_along_y_axis;
}

natural_32_bit  static_state_of_neural_tissue::num_cells_along_columnar_axis() const
{
    return m_num_cells_along_columnar_axis;
}

natural_32_bit  static_state_of_neural_tissue::num_cells_along_columnar_axis_of_cell_kind(
        kind_of_cell const cell_kind) const
{
    ASSUMPTION(cell_kind < num_kinds_of_cells());
    return m_num_cells_along_columnar_axis_of_cell_kind.at(cell_kind);
}

natural_32_bit  static_state_of_neural_tissue::num_synapses_in_territory_of_cell_kind(
        kind_of_cell const cell_kind) const
{
    ASSUMPTION(cell_kind < num_kinds_of_cells());
    return m_num_synapses_in_territory_of_cell_kind.at(cell_kind);
}

natural_32_bit  static_state_of_neural_tissue::num_sensory_cells() const
{
    return m_num_sensory_cells;
}

natural_32_bit  static_state_of_neural_tissue::num_synapses_to_muscles() const
{
    return m_num_synapses_to_muscles;
}

kind_of_cell  static_state_of_neural_tissue::compute_kind_of_cell_from_its_position_along_columnar_axis(
            natural_32_bit position_of_cell_in_column) const
{
    ASSUMPTION(position_of_cell_in_column < num_cells_along_columnar_axis());
    std::vector<natural_32_bit>::const_iterator it =
            std::upper_bound(
                    m_end_index_along_columnar_axis_of_cell_kind.begin(),
                    m_end_index_along_columnar_axis_of_cell_kind.end(),
                    position_of_cell_in_column
                    );
    if (it == m_end_index_along_columnar_axis_of_cell_kind.end())
        return num_kinds_of_cells() - 1U;

    return static_cast<kind_of_cell>(it - m_end_index_along_columnar_axis_of_cell_kind.begin());
}

bool  static_state_of_neural_tissue::is_x_axis_torus_axis() const
{
    return m_is_x_axis_torus_axis;
}

bool  static_state_of_neural_tissue::is_y_axis_torus_axis() const
{
    return m_is_y_axis_torus_axis;
}

bool  static_state_of_neural_tissue::is_columnar_axis_torus_axis() const
{
    return m_is_columnar_axis_torus_axis;
}

integer_8_bit  static_state_of_neural_tissue::get_x_radius_of_signalling_neighbourhood_of_cell(
        kind_of_cell const cell_kind) const
{
    ASSUMPTION(cell_kind < num_kinds_of_cells());
    return m_x_radius_of_signalling_neighbourhood_of_cell.at(cell_kind);
}

integer_8_bit  static_state_of_neural_tissue::get_y_radius_of_signalling_neighbourhood_of_cell(
        kind_of_cell const cell_kind) const
{
    ASSUMPTION(cell_kind < num_kinds_of_cells());
    return m_y_radius_of_signalling_neighbourhood_of_cell.at(cell_kind);
}

integer_8_bit  static_state_of_neural_tissue::get_columnar_radius_of_signalling_neighbourhood_of_cell(
        kind_of_cell const cell_kind) const
{
    ASSUMPTION(cell_kind < num_kinds_of_cells());
    return m_columnar_radius_of_signalling_neighbourhood_of_cell.at(cell_kind);
}

integer_8_bit  static_state_of_neural_tissue::get_x_radius_of_signalling_neighbourhood_of_synapse(
        kind_of_cell const cell_kind) const
{
    ASSUMPTION(cell_kind < num_kinds_of_cells());
    return m_x_radius_of_signalling_neighbourhood_of_synapse.at(cell_kind);
}

integer_8_bit  static_state_of_neural_tissue::get_y_radius_of_signalling_neighbourhood_of_synapse(
        kind_of_cell const cell_kind) const
{
    ASSUMPTION(cell_kind < num_kinds_of_cells());
    return m_y_radius_of_signalling_neighbourhood_of_synapse.at(cell_kind);
}

integer_8_bit  static_state_of_neural_tissue::get_columnar_radius_of_signalling_neighbourhood_of_synapse(
        kind_of_cell const cell_kind) const
{
    ASSUMPTION(cell_kind < num_kinds_of_cells());
    return m_columnar_radius_of_signalling_neighbourhood_of_synapse.at(cell_kind);
}

integer_8_bit  static_state_of_neural_tissue::get_x_radius_of_cellular_neighbourhood_of_signalling(
        kind_of_cell const cell_kind) const
{
    ASSUMPTION(cell_kind < num_kinds_of_cells());
    return m_x_radius_of_cellular_neighbourhood_of_signalling.at(cell_kind);
}

integer_8_bit  static_state_of_neural_tissue::get_y_radius_of_cellular_neighbourhood_of_signalling(
        kind_of_cell const cell_kind) const
{
    ASSUMPTION(cell_kind < num_kinds_of_cells());
    return m_y_radius_of_cellular_neighbourhood_of_signalling.at(cell_kind);
}

integer_8_bit  static_state_of_neural_tissue::get_columnar_radius_of_cellular_neighbourhood_of_signalling(
        kind_of_cell const cell_kind) const
{
    ASSUMPTION(cell_kind < num_kinds_of_cells());
    return m_columnar_radius_of_cellular_neighbourhood_of_signalling.at(cell_kind);
}


}
