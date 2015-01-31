#include <cellconnect/incremental_addition_of_tissue_links.hpp>
#include <utility/development.hpp>

namespace cellconnect {


void  initialisation_of_incremental_addition_of_tissue_links(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        std::vector< std::function<float_32_bit()> > const&
            random_generators_of_polar_angles_to_centers_of_far_link_target_areas,
        std::vector< std::function<float_32_bit()> > const&
            random_generators_of_polar_radii_to_centers_of_far_link_target_areas
        )
{
    NOT_IMPLEMENTED_YET();
}


void  apply_step_of_incremental_addition_of_tissue_links(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        std::function<natural_16_bit()> const&
            random_generator_of_cell_kind_from_whose_randomly_chosen_cell_will_grown_a_new_link,
        std::vector< std::function<natural_8_bit()> > const&
            random_generators_of_selection_between_local_and_far_link_target_areas,
        std::vector< std::function<float_32_bit()> > const&
            random_generators_of_polar_angles_from_centers_of_local_link_target_areas,
        std::vector< std::function<float_32_bit()> > const&
            random_generators_of_polar_radii_from_centers_of_local_link_target_areas,
        std::function<natural_16_bit()> const&
            random_generator_of_link_target_cell_kinds_in_local_link_target_areas,
        std::vector< std::function<float_32_bit()> > const&
            random_generators_of_polar_angles_from_centers_of_far_link_target_areas,
        std::vector< std::function<float_32_bit()> > const&
            random_generators_of_polar_radii_from_centers_of_far_link_target_areas,
        std::function<natural_16_bit()> const&
            random_generator_of_link_target_cell_kinds_in_far_link_target_areas
        )
{
    NOT_IMPLEMENTED_YET();
}


}
