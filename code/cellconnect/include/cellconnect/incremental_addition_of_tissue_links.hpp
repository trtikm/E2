#ifndef CELLCONNECT_INCREMENTAL_ADDITION_OF_TISSUE_LINKS_HPP_INCLUDED
#   define CELLCONNECT_INCREMENTAL_ADDITION_OF_TISSUE_LINKS_HPP_INCLUDED

#   include <cellab/dynamic_state_of_neural_tissue.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <memory>
#   include <vector>
#   include <functional>

namespace cellconnect {


void  initialisation_of_incremental_addition_of_tissue_links(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        std::vector< std::function<float_32_bit()> > const&
            random_generators_of_polar_angles_to_centers_of_far_link_target_areas,
        std::vector< std::function<float_32_bit()> > const&
            random_generators_of_polar_radii_to_centers_of_far_link_target_areas,
        natural_32_bit const  num_threads_avalilable_for_computation
        );

void  apply_step_of_incremental_addition_of_tissue_links(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        std::function<cellab::kind_of_cell()> const&
            random_generator_of_cell_kind_from_whose_randomly_chosen_cell_will_grown_a_new_link,
        std::vector< std::function<bool()> > const&
            random_generators_of_selection_between_local_and_far_link_target_areas,
        std::vector< std::function<float_32_bit()> > const&
            random_generators_of_polar_angles_from_centers_of_local_link_target_areas,
        std::vector< std::function<float_32_bit()> > const&
            random_generators_of_polar_radii_from_centers_of_local_link_target_areas,
        std::vector< std::function<cellab::kind_of_cell()> > const&
            random_generators_of_link_target_cell_kinds_in_local_link_target_areas,
        std::vector< std::function<float_32_bit()> > const&
            random_generators_of_polar_angles_from_centers_of_far_link_target_areas,
        std::vector< std::function<float_32_bit()> > const&
            random_generators_of_polar_radii_from_centers_of_far_link_target_areas,
        std::vector< std::function<cellab::kind_of_cell()> > const&
            random_generators_of_link_target_cell_kinds_in_far_link_target_areas
        );


}

#endif
