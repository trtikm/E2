#ifndef CELLCONNECT_UTILITY_CHOOSE_TISSUE_COORDINATES_HPP_INCLUDED
#   define CELLCONNECT_UTILITY_CHOOSE_TISSUE_COORDINATES_HPP_INCLUDED

#   include <cellab/static_state_of_neural_tissue.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <memory>
#   include <functional>
#   include <utility>

namespace cellconnect {


std::pair<natural_32_bit,natural_32_bit>  choose_tissue_coordinates_in_area(
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const  static_state_ptr,
        natural_32_bit const  x_coord_of_center_of_area,
        natural_32_bit const  y_coord_of_center_of_area,
        std::function<float_32_bit()> const&  random_generator_for_angle_of_shift_vector,
        std::function<float_32_bit()> const&  random_generator_for_radius_of_shift_vector
        );


natural_32_bit  choose_tissue_columnar_coordinate(
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const  static_state_ptr,
        cellab::kind_of_cell const  kind_of_target_cell
        );


}

#endif
