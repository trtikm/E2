#include <cellconnect/incremental_addition_of_tissue_links.hpp>
#include <cellconnect/utility/access_to_byte_buffer_of_tissue_cell.hpp>
#include <cellconnect/utility/choose_tissue_coordinates.hpp>
#include <utility/bit_count.hpp>
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
    natural_8_bit const num_bytes_per_coord =
            num_bytes_to_store_bits(dynamic_state_ptr->num_bits_per_source_cell_coordinate());
    std::shared_ptr<cellab::static_state_of_neural_tissue const> static_state_ptr =
            dynamic_state_ptr->get_static_state_of_neural_tissue();

    ASSUMPTION(random_generators_of_polar_angles_to_centers_of_far_link_target_areas.size() == static_state_ptr->num_kinds_of_tissue_cells());
    ASSUMPTION(random_generators_of_polar_radii_to_centers_of_far_link_target_areas.size() == static_state_ptr->num_kinds_of_tissue_cells());

    for (natural_32_bit x = 0U; x < static_state_ptr->num_cells_along_x_axis(); ++x)
        for (natural_32_bit y = 0U; y < static_state_ptr->num_cells_along_y_axis(); ++y)
            for (natural_32_bit c = 0U; c < static_state_ptr->num_cells_along_columnar_axis(); ++c)
            {
                cellab::kind_of_cell const  cell_kind =
                        static_state_ptr->compute_kind_of_cell_from_its_position_along_columnar_axis(c);

                std::pair<natural_32_bit,natural_32_bit>  const center_xy_coords =
                        choose_tissue_coordinates_in_area(
                                static_state_ptr,
                                x,y,
                                random_generators_of_polar_angles_to_centers_of_far_link_target_areas.at(cell_kind),
                                random_generators_of_polar_radii_to_centers_of_far_link_target_areas.at(cell_kind)
                                );
                write_to_byte_buffer_of_tissue_cell(
                            dynamic_state_ptr,
                            x,y,c,
                            0U,
                            num_bytes_per_coord,
                            center_xy_coords.first
                            );
                write_to_byte_buffer_of_tissue_cell(
                            dynamic_state_ptr,
                            x,y,c,
                            num_bytes_per_coord,
                            num_bytes_per_coord,
                            center_xy_coords.second
                            );
                write_to_byte_buffer_of_tissue_cell(
                            dynamic_state_ptr,
                            x,y,c,
                            num_bytes_per_coord + num_bytes_per_coord,
                            num_bytes_to_store_bits(
                                    compute_num_of_bits_to_store_number(
                                            static_state_ptr->num_synapses_in_territory_of_cell_kind(cell_kind))),
                            0U
                            );
            }
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
