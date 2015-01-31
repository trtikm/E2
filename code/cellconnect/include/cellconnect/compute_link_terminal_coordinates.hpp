#ifndef CELLCONNECT_COMPUTE_LINK_TERMINAL_COORDINATES_HPP_INCLUDED
#   define CELLCONNECT_COMPUTE_LINK_TERMINAL_COORDINATES_HPP_INCLUDED

#   include <cellab/static_state_of_neural_tissue.hpp>
#   include <cellab/utilities_for_transition_algorithms.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <memory>
#   include <functional>
#   include <utility>

namespace cellconnect {


natural_32_bit  clip_link_terminal_coordinate(
        integer_64_bit  target_coord,
        natural_32_bit const  num_cells_along_the_axis,
        bool const  is_torus_axis
        );


std::pair<integer_64_bit,integer_64_bit>  from_polar_to_tissue_coordinates(
        float_32_bit const  angle,
        float_32_bit const  radius
        );


cellab::tissue_coordinates  compute_link_terminal_coordinates(
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const  static_state_ptr,
        natural_32_bit const  x_coord_of_center_of_target_area,
        natural_32_bit const  y_coord_of_center_of_target_area,
        cellab::kind_of_cell const  kind_of_target_cell,
        std::function<integer_64_bit()> const&  x_coord_shift_distribution,
        std::function<integer_64_bit()> const&  y_coord_shift_distribution
        );


}

#endif
