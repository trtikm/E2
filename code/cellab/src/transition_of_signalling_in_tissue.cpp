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

namespace cellab {

typedef natural_16_bit kind_of_cell;

struct tissue_coordinates;
struct shift_in_spatial_neighbourhood;
struct spatial_neighbourhood;

static std::pair<bits_const_reference,kind_of_cell> get_cell(
        std::shared_ptr<dynamic_state_of_neural_tissue const> const dynamic_state_of_tissue,
        spatial_neighbourhood const& neighbourhood,
        shift_in_spatial_neighbourhood const& shift)
{
}

static void apply_transition_of_signalling_in_tissue(
        std::shared_ptr<dynamic_state_of_neural_tissue const> const dynamic_state_of_tissue,
        std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue,
        std::function<
            void(
            bits_reference& bits_of_signalling_data_to_be_updated,
            kind_of_cell kind_of_territory_cell,
            shift_in_spatial_neighbourhood const& shift_to_low_corner,
            shift_in_spatial_neighbourhood const& shift_to_high_corner,
            std::function<std::pair<bits_const_reference,kind_of_cell>(shift_in_spatial_neighbourhood const&)>
                get_cell,
            )> single_threaded_transition_function_of_packed_dynamic_state_of_signalling,
        natural_32_bit x_coord,
        natural_32_bit y_coord,
        natural_32_bit c_coord,
        natural_32_bit const extent_in_coordinates
        )
{
    do
    {
        bits_reference bits_of_signalling =
            dynamic_state_of_tissue->find_bits_of_signalling(x_coord,y_coord,c_coord);

        natural_16_bit const kind_of_territory_cell =
            static_state_of_tissue->compute_kind_of_tissue_cell_from_its_position_along_columnar_axis(c_coord);

        tissue_coordinates const territory_cell_coordinates(x_coord,y_coord,c_coord);

        shift_in_spatial_neighbourhood shift_to_low_corner(
            clip_shift(-static_state_of_tissue->get_x_radius_of_cellular_neighbourhood_of_signalling(
                           kind_of_territory_cell),
                       territory_cell_coordinates.get_coord_along_x_axis(),
                       static_state_of_tissue->num_tissue_cells_along_x_axis(),
                       static_state_of_tissue->is_x_axis_torus_axis()),
            clip_shift(-static_state_of_tissue->get_y_radius_of_cellular_neighbourhood_of_signalling(
                           kind_of_territory_cell),
                       territory_cell_coordinates.get_coord_along_y_axis(),
                       static_state_of_tissue->num_tissue_cells_along_y_axis(),
                       static_state_of_tissue->is_y_axis_torus_axis()),
            clip_shift(-static_state_of_tissue->get_columnar_radius_of_cellular_neighbourhood_of_signalling(
                           kind_of_territory_cell),
                       territory_cell_coordinates.get_coord_along_columnar_axis(),
                       static_state_of_tissue->num_tissue_cells_along_columnar_axis(),
                       static_state_of_tissue->is_columnar_axis_torus_axis())
            );

        shift_in_spatial_neighbourhood shift_to_high_corner(
            clip_shift(static_state_of_tissue->get_x_radius_of_cellular_neighbourhood_of_signalling(
                           kind_of_territory_cell),
                       territory_cell_coordinates.get_coord_along_x_axis(),
                       static_state_of_tissue->num_tissue_cells_along_x_axis(),
                       static_state_of_tissue->is_x_axis_torus_axis()),
            clip_shift(static_state_of_tissue->get_y_radius_of_cellular_neighbourhood_of_signalling(
                           kind_of_territory_cell),
                       territory_cell_coordinates.get_coord_along_y_axis(),
                       static_state_of_tissue->num_tissue_cells_along_y_axis(),
                       static_state_of_tissue->is_y_axis_torus_axis()),
            clip_shift(static_state_of_tissue->get_columnar_radius_of_cellular_neighbourhood_of_signalling(
                           kind_of_territory_cell),
                       territory_cell_coordinates.get_coord_along_columnar_axis(),
                       static_state_of_tissue->num_tissue_cells_along_columnar_axis(),
                       static_state_of_tissue->is_columnar_axis_torus_axis())
            );

        spatial_neighbourhood const signalling_neighbourhood(
                    territory_cell_coordinates,shift_to_low_corner,shift_to_high_corner
                    );

        single_threaded_transition_function_of_packed_dynamic_state_of_signalling(
                    bits_of_signalling,
                    kind_of_territory_cell,
                    shift_to_low_corner,
                    shift_to_high_corner,
                    std::bind(&cellab::get_cell,dynamic_state_of_tissue,std::cref(signalling_neighbourhood),
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

void apply_transition_of_signalling_in_tissue(
        std::shared_ptr<dynamic_state_of_neural_tissue const> const dynamic_state_of_tissue,
        std::function<
            void(
            bits_reference& bits_of_signalling_data_to_be_updated,
            kind_of_cell kind_of_territory_cell,
            shift_in_spatial_neighbourhood const& shift_to_low_corner,
            shift_in_spatial_neighbourhood const& shift_to_high_corner,
            std::function<std::pair<bits_const_reference,kind_of_cell>(shift_in_spatial_neighbourhood const&)>
                get_cell,
            )> single_threaded_transition_function_of_packed_dynamic_state_of_signalling,
        natural_32_bit num_avalilable_thread_for_creation_and_use
        )
{
    std::shared_ptr<static_state_of_neural_tissue const> const static_state_of_tissue =
            dynamic_state_of_tissue->get_static_state_of_neural_tissue();

    std::vector<std::thread> threads;
    for (natural_32_bit i = 0; i < num_avalilable_thread_for_creation_and_use; ++i)
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
                        &cellab::apply_transition_of_signalling_in_tissue,
                        dynamic_state_of_tissue,
                        static_state_of_tissue,
                        single_threaded_transition_function_of_packed_dynamic_state_of_signalling,
                        x_coord,y_coord,c_coord,
                        num_avalilable_thread_for_creation_and_use
                        )
                    );
    }

    for(std::thread& thread : threads)
        thread.join();
}


}
