#ifndef CELLCONNECT_ADD_LINKS_FROM_SENSORY_CELLS_HPP_INCLUDED
#   define CELLCONNECT_ADD_LINKS_FROM_SENSORY_CELLS_HPP_INCLUDED

#   include <cellab/dynamic_state_of_neural_tissue.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <memory>
#   include <vector>
#   include <functional>

namespace cellconnect {


void  add_links_from_sensory_cells(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        std::vector<natural_32_bit> const& out_degrees_of_sensory_cell_kinds,
        std::vector<natural_32_bit> const& x_coords_of_centers_of_link_target_areas,
        std::vector<natural_32_bit> const& y_coords_of_centers_of_link_target_areas,
        std::vector< std::function<float_32_bit()> > const&
            random_generators_of_polar_angles_from_centers_of_link_target_areas,
        std::vector< std::function<float_32_bit()> > const&
            random_generators_of_polar_radii_from_centers_of_link_target_areas,
        std::vector< std::function<natural_16_bit()> > const&
            random_generators_of_tissue_cell_kinds_for_targets_of_links_in_link_target_areas
        );


}

#endif
