#ifndef CELLCONNECT_ADD_LINKS_TO_SYNAPSES_TO_MUSCLES_HPP_INCLUDED
#   define CELLCONNECT_ADD_LINKS_TO_SYNAPSES_TO_MUSCLES_HPP_INCLUDED

#   include <cellab/dynamic_state_of_neural_tissue.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <memory>
#   include <vector>
#   include <functional>

namespace cellconnect {


void  add_links_to_synapses_to_muscles(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        std::vector<natural_32_bit> const& x_coords_of_centers_in_tissue,
        std::vector<natural_32_bit> const& y_coords_of_centers_in_tissue,
        std::vector< std::function<float_32_bit()> > const&
            random_generators_of_polar_angles_from_centers_in_tissue,
        std::vector< std::function<float_32_bit()> > const&
            random_generators_of_polar_radii_from_centers_in_tissue,
        std::vector< std::function<cellab::kind_of_cell()> > const&
            random_generators_of_tissue_cell_kinds_for_sources_of_links
        );


}


#endif
