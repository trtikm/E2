#include <cellconnect/compute_link_terminal_coordinates.hpp>
#include <utility/random.hpp>
#include <cmath>

namespace cellconnect {


natural_32_bit  clip_link_terminal_coordinate(
        integer_64_bit  target_coord,
        natural_32_bit const  num_cells_along_the_axis,
        bool const  is_torus_axis
        )
{
    if (is_torus_axis)
    {
        while (target_coord < 0LL)
            target_coord += num_cells_along_the_axis;
        return target_coord % num_cells_along_the_axis;
    }

    if (target_coord < 0LL)
        return 0U;
    if (target_coord > num_cells_along_the_axis)
        return num_cells_along_the_axis - 1U;
    return target_coord;
}


std::pair<integer_64_bit,integer_64_bit>  from_polar_to_tissue_coordinates(
        float_32_bit const  angle,
        float_32_bit const  radius
        )
{
    return std::pair<integer_64_bit,integer_64_bit>(
                std::round(radius * std::cos(angle)),
                std::round(radius * std::sin(angle))
                );
}


cellab::tissue_coordinates  compute_link_terminal_coordinates(
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const  static_state_ptr,
        natural_32_bit const  x_coord_of_center_of_target_area,
        natural_32_bit const  y_coord_of_center_of_target_area,
        cellab::kind_of_cell const  kind_of_target_cell,
        std::function<float_32_bit()> const&  random_generator_for_angle_of_shift_vector,
        std::function<float_32_bit()> const&  random_generator_for_radius_of_shift_vector
        )
{
    std::pair<integer_64_bit,integer_64_bit> const  shift =
            from_polar_to_tissue_coordinates(
                    random_generator_for_angle_of_shift_vector(),
                    random_generator_for_radius_of_shift_vector()
                    );
    return cellab::tissue_coordinates(
                clip_link_terminal_coordinate(
                        x_coord_of_center_of_target_area + shift.first,
                        static_state_ptr->num_cells_along_x_axis(),
                        static_state_ptr->is_x_axis_torus_axis()
                        ),
                clip_link_terminal_coordinate(
                        y_coord_of_center_of_target_area + shift.second,
                        static_state_ptr->num_cells_along_y_axis(),
                        static_state_ptr->is_y_axis_torus_axis()
                        ),
                get_random_natural_32_bit_in_range(
                        static_state_ptr->compute_columnar_coord_of_first_tissue_cell_of_kind(kind_of_target_cell),
                        static_state_ptr->num_cells_along_columnar_axis() - 1U
                        )
                );
}



}
