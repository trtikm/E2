#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <cellab/static_state_of_neural_tissue.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/bits_reference.hpp>
#include <utility/assumptions.hpp>
#include <functional>
#include <memory>
#include <vector>
#include <tuple>
#include <thread>

#include <utility/development.hpp>

static natural_64_bit go_to_next_value_modulo_range(natural_64_bit const current_value,
                                                    natural_64_bit const range,
                                                    natural_64_bit& extent)
{
    natural_64_bit const next_value = (current_value + extent) % range;
    extent = (current_value + extent) / range;
    return next_value;
}

static bool go_to_next_coordinates(natural_32_bit& x_coord, natural_32_bit& y_coord, natural_32_bit& c_coord,
                                   natural_32_bit const extent,
                                   natural_32_bit const num_tissue_cells_along_x_axis,
                                   natural_32_bit const num_tissue_cells_along_y_axis,
                                   natural_32_bit const num_tissue_cells_along_columnar_axis)
{
    natural_64_bit extent_64_bit = extent;
    c_coord = go_to_next_value_modulo_range(c_coord,num_tissue_cells_along_columnar_axis,extent_64_bit);
    x_coord = go_to_next_value_modulo_range(x_coord,num_tissue_cells_along_x_axis,extent_64_bit);
    y_coord = go_to_next_value_modulo_range(y_coord,num_tissue_cells_along_y_axis,extent_64_bit);
    return extent_64_bit == 0ULL;
}

static natural_32_bit do_coordinate_shift(natural_32_bit const coordinate, integer_8_bit const extent,
                                          natural_32_bit const length_of_axis)
{
    ASSUMPTION(static_cast<integer_64_bit>(extent) > -static_cast<integer_64_bit>(length_of_axis));
    return (static_cast<integer_64_bit>(coordinate) +
            static_cast<integer_64_bit>(extent) +
            static_cast<integer_64_bit>(length_of_axis)) % static_cast<integer_64_bit>(length_of_axis);
}

static integer_8_bit clip_shift(integer_8_bit const shift, natural_32_bit const origin,
                                natural_32_bit const length_of_axis, bool const is_it_torus_axis)
{
    if (is_it_torus_axis)
        return shift;

    integer_64_bit const shift64 = shift;
    integer_64_bit const origin64 = origin;
    integer_64_bit const destination = origin64 + shift64;
    integer_64_bit const length64 = length_of_axis;

    if (destination < 0ULL)
        return -origin64;

    if (destination >= length64)
        return (length64 - 1LL) - origin64;

    return shift;
}


namespace cellab {


typedef natural_16_bit kind_of_cell;

struct tissue_coordinates
{
    tissue_coordinates(natural_32_bit const coord_along_x_axis,
                       natural_32_bit const coord_along_y_axis,
                       natural_32_bit const coord_along_columnar_axis);
    natural_32_bit get_coord_along_x_axis() const;
    natural_32_bit get_coord_along_y_axis() const;
    natural_32_bit get_coord_along_columnar_axis() const;
private:
    natural_32_bit m_coord_along_x_axis;
    natural_32_bit m_coord_along_y_axis;
    natural_32_bit m_coord_along_columnar_axis;
};

struct shift_in_spatial_neighbourhood
{
    shift_in_spatial_neighbourhood(integer_8_bit const shift_along_x_axis,
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

struct spatial_neighbourhood
{
    spatial_neighbourhood(tissue_coordinates const& center,
                          shift_in_spatial_neighbourhood const& shift_to_low_corner,
                          shift_in_spatial_neighbourhood const& shift_to_high_corner);
private:
    tissue_coordinates m_center_of_neighbourhood;
    shift_in_spatial_neighbourhood m_shift_to_low_corner;
    shift_in_spatial_neighbourhood m_shift_to_high_corner;
};


static std::pair<bits_const_reference,kind_of_cell> get_synapse(
        std::shared_ptr<dynamic_state_of_neural_tissue const> const dynamic_state_of_tissue,
        tissue_coordinates const& target_cell,
        natural_32_bit const number_of_synapses_in_range,
        natural_32_bit const shift_to_start_index,
        natural_32_bit const shift_from_start_index)
{
}

static std::pair<bits_const_reference,kind_of_cell> get_signalling(
        std::shared_ptr<dynamic_state_of_neural_tissue const> const dynamic_state_of_tissue,
        spatial_neighbourhood const& neighbourhood,
        shift_in_spatial_neighbourhood const& shift)
{
}

static void thread_apply_transition_of_cells_of_tissue(
        std::shared_ptr<dynamic_state_of_neural_tissue const> const dynamic_state_of_tissue,
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue,
        std::function<
            void(
            bits_reference& bits_of_cell_to_be_updated,
            kind_of_cell kind_of_cell_to_be_updated,
            natural_32_bit num_of_synapses_connected_to_the_cell,
            std::function<std::tuple<bits_const_reference,kind_of_cell,kind_of_cell>(natural_32_bit)>
                get_connected_synapse_at_index,
            shift_in_spatial_neighbourhood const& shift_to_low_corner,
            shift_in_spatial_neighbourhood const& shift_to_high_corner,
            std::function<std::pair<bits_const_reference,kind_of_cell>(shift_in_spatial_neighbourhood const&)>
                get_signalling,
            )> single_threaded_transition_function_of_packed_dynamic_state_of_cell,
        natural_32_bit x_coord,
        natural_32_bit y_coord,
        natural_32_bit c_coord,
        natural_32_bit const extent_in_coordinates
        )
{
    do
    {
        bits_reference bits_of_cell =
            dynamic_state_of_tissue->find_bits_of_cell_in_tissue(x_coord,y_coord,c_coord);

        natural_16_bit const cell_kind =
            static_state_of_tissue->compute_kind_of_tissue_cell_from_its_position_along_columnar_axis(c_coord);

        tissue_coordinates const cell_coordinates(x_coord,y_coord,c_coord);

        shift_in_spatial_neighbourhood shift_to_low_corner(
            clip_shift(-static_state_of_tissue->get_x_radius_of_signalling_neighbourhood_of_cell(cell_kind),
                       cell_coordinates.get_coord_along_x_axis(),
                       static_state_of_tissue->num_tissue_cells_along_x_axis(),
                       static_state_of_tissue->is_x_axis_torus_axis()),
            clip_shift(-static_state_of_tissue->get_y_radius_of_signalling_neighbourhood_of_cell(cell_kind),
                       cell_coordinates.get_coord_along_y_axis(),
                       static_state_of_tissue->num_tissue_cells_along_y_axis(),
                       static_state_of_tissue->is_y_axis_torus_axis()),
            clip_shift(-static_state_of_tissue->get_columnar_radius_of_signalling_neighbourhood_of_cell(cell_kind),
                       cell_coordinates.get_coord_along_columnar_axis(),
                       static_state_of_tissue->num_tissue_cells_along_columnar_axis(),
                       static_state_of_tissue->is_columnar_axis_torus_axis())
            );

        shift_in_spatial_neighbourhood shift_to_high_corner(
            clip_shift(static_state_of_tissue->get_x_radius_of_signalling_neighbourhood_of_cell(cell_kind),
                       cell_coordinates.get_coord_along_x_axis(),
                       static_state_of_tissue->num_tissue_cells_along_x_axis(),
                       static_state_of_tissue->is_x_axis_torus_axis()),
            clip_shift(static_state_of_tissue->get_y_radius_of_signalling_neighbourhood_of_cell(cell_kind),
                       cell_coordinates.get_coord_along_y_axis(),
                       static_state_of_tissue->num_tissue_cells_along_y_axis(),
                       static_state_of_tissue->is_y_axis_torus_axis()),
            clip_shift(static_state_of_tissue->get_columnar_radius_of_signalling_neighbourhood_of_cell(cell_kind),
                       cell_coordinates.get_coord_along_columnar_axis(),
                       static_state_of_tissue->num_tissue_cells_along_columnar_axis(),
                       static_state_of_tissue->is_columnar_axis_torus_axis())
            );

        spatial_neighbourhood const cell_neighbourhood(cell_coordinates,shift_to_low_corner,shift_to_high_corner);

        natural_32_bit const number_of_connected_synapses =
            dynamic_state_of_tissue->get_num_of_synapses_connected_to_cell(x_coord,y_coord,c_coord);
        natural_32_bit const start_index_of_connected_synapses =
            dynamic_state_of_tissue->get_start_index_of_synapses_connected_to_cell(x_coord,y_coord,c_coord);

        single_threaded_transition_function_of_packed_dynamic_state_of_cell(
                    bits_of_cell,
                    cell_kind,
                    number_of_connected_synapses,
                    std::bind(&cellab::get_synapse,dynamic_state_of_tissue,std::cref(cell_coordinates),
                              number_of_connected_synapses,start_index_of_connected_synapses,
                              std::placeholders::_1),
                    shift_to_low_corner,
                    shift_to_high_corner,
                    std::bind(&cellab::get_signalling,dynamic_state_of_tissue,std::cref(cell_neighbourhood),
                              std::placeholders::_1)
                    );
    }
    while (seek_to_next_coordinates(
                    x_coord,y_coord,c_coord,
                    extent_in_coordinates,
                    static_state_of_tissue->num_tissue_cells_along_x_axis(),
                    static_state_of_tissue->num_tissue_cells_along_y_axis(),
                    static_state_of_tissue->num_tissue_cells_along_columnar_axis()
                    ));
}

void apply_transition_of_cells_of_tissue(
        std::shared_ptr<dynamic_state_of_neural_tissue const> const dynamic_state_of_tissue,
        std::function<
            void(
            bits_reference bits_of_cell_to_be_updated,
            kind_of_cell kind_of_cell_to_be_updated,
            natural_32_bit num_of_synapses_connected_to_the_cell,
            std::function<std::tuple<bits_const_reference,kind_of_cell,kind_of_cell>(natural_32_bit)>
                get_connected_synapse_at_index,
            shift_in_spatial_neighbourhood const& shift_to_low_corner,
            shift_in_spatial_neighbourhood const& shift_to_high_corner,
            std::function<std::pair<bits_const_reference,kind_of_cell>(shift_in_spatial_neighbourhood const&)>
                get_signalling,
            )> single_threaded_transition_function_of_packed_dynamic_state_of_cell,
        natural_32_bit num_avalilable_thread_for_creation_and_use
        )
{
    std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue =
            dynamic_state_of_tissue->get_static_state_of_neural_tissue();

    std::vector<std::thread> threads;
    for (natural_32_bit i = 0U; i < num_avalilable_thread_for_creation_and_use; ++i)
    {
        natural_32_bit x_coord = 0U;
        natural_32_bit y_coord = 0U;
        natural_32_bit c_coord = 0U;
        if (!seek_to_next_coordinates(
                    x_coord,y_coord,c_coord,
                    i,
                    static_state_of_tissue->num_tissue_cells_along_x_axis(),
                    static_state_of_tissue->num_tissue_cells_along_y_axis(),
                    static_state_of_tissue->num_tissue_cells_along_columnar_axis()
                    ))
            break;

        threads.push_back(
                    std::thread(
                        &cellab::thread_apply_transition_of_cells_of_tissue,
                        dynamic_state_of_tissue,
                        static_state_of_tissue,
                        single_threaded_transition_function_of_packed_dynamic_state_of_cell,
                        x_coord,y_coord,c_coord,
                        num_avalilable_thread_for_creation_and_use
                        )
                    );
    }

    for(std::thread& thread : threads)
        thread.join();
}


}
