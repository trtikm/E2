#include <cellconnect/utility/choose_tissue_coordinates.hpp>
#include <cellconnect/utility/clip_tissue_coordinate.hpp>
#include <cellconnect/utility/from_polar_to_tissue_coordinates.hpp>
#include <utility/random.hpp>

namespace cellconnect {


std::pair<natural_32_bit,natural_32_bit>  choose_tissue_coordinates_in_area(
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const  static_state_ptr,
        natural_32_bit const  x_coord_of_center_of_area,
        natural_32_bit const  y_coord_of_center_of_area,
        std::function<float_32_bit()> const&  random_generator_for_angle_of_shift_vector,
        std::function<float_32_bit()> const&  random_generator_for_radius_of_shift_vector
        )
{
    std::pair<integer_64_bit,integer_64_bit> const  shift =
            from_polar_to_tissue_coordinates(
                    random_generator_for_angle_of_shift_vector(),
                    random_generator_for_radius_of_shift_vector()
                    );
    return std::pair<natural_32_bit,natural_32_bit>(
                    clip_tissue_coordinate(
                            x_coord_of_center_of_area + shift.first,
                            static_state_ptr->num_cells_along_x_axis(),
                            static_state_ptr->is_x_axis_torus_axis()
                            ),
                    clip_tissue_coordinate(
                            y_coord_of_center_of_area + shift.second,
                            static_state_ptr->num_cells_along_y_axis(),
                            static_state_ptr->is_y_axis_torus_axis()
                            )
                    );
}


natural_32_bit  choose_tissue_columnar_coordinate(
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const  static_state_ptr,
        cellab::kind_of_cell const  kind_of_target_cell
        )
{
    natural_32_bit const  min_columnar_coord =
            static_state_ptr->compute_columnar_coord_of_first_tissue_cell_of_kind(kind_of_target_cell);
    natural_32_bit const  max_columnar_coord =
            min_columnar_coord + static_state_ptr->num_tissue_cells_of_cell_kind(kind_of_target_cell) - 1U;
    return get_random_natural_32_bit_in_range(min_columnar_coord,max_columnar_coord);
}


}
