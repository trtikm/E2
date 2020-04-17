#include <ai/sensory_controller_collision_contacts.hpp>
#include <ai/action_controller.hpp>
#include <angeo/coordinate_system.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


sensory_controller_collision_contacts::config::config(
        natural_16_bit const  num_cells_along_any_axis_,
        std::function<float_32_bit(float_32_bit)> const& distribution_of_cells_along_y_axis_
        )
    : num_cells_along_any_axis(num_cells_along_any_axis_)
    , distribution_of_cells_along_y_axis(distribution_of_cells_along_y_axis_)
{
    ASSUMPTION(num_cells_along_any_axis > 0U && distribution_of_cells_along_y_axis.operator bool());
}


sensory_controller_collision_contacts::collicion_contact_info::collicion_contact_info(
        natural_32_bit const  cell_x_,
        natural_32_bit const  cell_y_,
        vector3 const&  contact_point_in_local_space_,
        scene::collicion_contant_info_ptr const  data_
        )
    : cell_x(cell_x_)
    , cell_y(cell_y_)
    , contact_point_in_local_space(contact_point_in_local_space_)
    , data(data_)
{}


sensory_controller_collision_contacts::sensory_controller_collision_contacts(
        blackboard_agent_weak_ptr const  blackboard_,
        config const&  config_
        )
    : m_blackboard(blackboard_)
    , m_collision_contacts()
    , m_config(config_)
{}


void  sensory_controller_collision_contacts::next_round()
{
    std::swap(m_collision_contacts.front(), m_collision_contacts.back());
    m_collision_contacts.back().clear();
}


void  sensory_controller_collision_contacts::on_collision_contact(scene::collicion_contant_info_ptr const  contact_info)
{
    auto const&  motion_ref = get_blackboard()->m_action_controller->get_motion_object_motion();
    vector3 const  contact_vector = point3_to_orthonormal_base(
            contact_info->contact_point_in_world_space,
            motion_ref.frame.origin(),
            motion_ref.forward,
            cross_product(motion_ref.up, motion_ref.forward),
            motion_ref.up
            );
    float_32_bit const  len = length(contact_vector);
    vector2 const  raw_cell_coords_in_m1p1(
            std::atan2f(contact_vector(1), contact_vector(0)) / PI(),
            len < 1e-5f ? 0.0f : 2.0f * std::asinf(contact_vector(2) / len) / PI()
            );

    auto const  from_m1p1_to_01 = [](float_32_bit const  x) -> float_32_bit {
        return std::max(0.0f, std::min(1.0f, 0.5f * x + 0.5f));
    };

    auto const  stretch_y_coordinate_in_m1p1 = [this](float_32_bit const  y) -> float_32_bit {
        return m_config.distribution_of_cells_along_y_axis(std::fabs(y)) * (y >= 0.0f ? 1.0f : -1.0f);
    };

    vector2 const  raw_cell_coords_in_01(
            from_m1p1_to_01(raw_cell_coords_in_m1p1(0)),
            from_m1p1_to_01(stretch_y_coordinate_in_m1p1(raw_cell_coords_in_m1p1(1)))
            );

    auto const  to_cell_coord_in_01 = [this](float_32_bit const  coord) -> natural_32_bit {
        return (natural_32_bit)((float_32_bit)(m_config.num_cells_along_any_axis - 1U) * coord + 0.5f);
    };

    m_collision_contacts.back().insert({
            contact_info->self_collider_nid,
            collicion_contact_info {
                to_cell_coord_in_01(raw_cell_coords_in_01(0)),
                to_cell_coord_in_01(raw_cell_coords_in_01(1)),
                contact_vector,
                contact_info
                }
            });
}


}
