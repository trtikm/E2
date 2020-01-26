#ifndef AI_SENSORY_CONTROLLER_COLLISION_CONTACTS_HPP_INCLUDED
#   define AI_SENSORY_CONTROLLER_COLLISION_CONTACTS_HPP_INCLUDED

#   include <ai/blackboard_agent.hpp>
#   include <ai/scene.hpp>
#   include <angeo/tensor_math.hpp>
#   include <unordered_map>
#   include <array>
#   include <functional>
#   include <memory>

namespace ai {


struct  sensory_controller_collision_contacts final
{
    struct  config
    {
        natural_16_bit  num_cells_along_any_axis;   // "Touch" cells form a regular 2D grid, which is also rectangular;
                                                    // i.e. there is the same number of cells along both axes. Each contact is
                                                    // sensed by a cell corresponding to the contact point. A cell coordinates
                                                    // in the grid are 3D-polar coordinates associated with the local agent's
                                                    // Cartesian coord. system with 'forward', 'left', and 'up' being unit axis
                                                    // vectors respectively. Namely, given a vector to a contact point in the 
                                                    // mentioned Cartesian coordinates, its angle from the 'forward' vector in the
                                                    // ('forward', 'left') plane, scaled to the interval <0, 1>, is the x cell
                                                    // coodinate, and is angle from ('forward', 'left') plane, scaled to the
                                                    // interval <0, 1>, is the y cell coodinate.
        std::function<float_32_bit(float_32_bit)>  distribution_of_cells_along_y_axis;
                                                    // Although the cells form a regular 2D grid in the memory model, it might
                                                    // might still be useful to control a spatial distribution of the cells in
                                                    // the Cartesian coord. system. Typically, the density of cells inreases as
                                                    // we go further from the ('forward', 'left') plane. The function here defines
                                                    // that stretching. The argument is always in range <0.0, 1.0> and the return
                                                    // value must also always be in range <0.0, 1.0>. Further, these invariants
                                                    // must hold:
                                                    //      distribution_of_cells_along_y_axis(0.0f) == 0.0f
                                                    //      distribution_of_cells_along_y_axis(1.0f) == 1.0f
                                                    // It is typically desired the function being monotonic (increasing).

        config( natural_16_bit const  num_cells_along_any_axis_ = 10U,
                std::function<float_32_bit(float_32_bit)> const& distribution_of_cells_along_y_axis_ =
                    [](float_32_bit const  x) -> float_32_bit { return x * x; } // This is a quadratic stretch of the cell's grid.
                );
    };

    struct  collicion_contact_info
    {
        natural_32_bit  cell_x;
        natural_32_bit  cell_y;
        vector3  contact_point_in_local_space;
        scene::collicion_contant_info  data;

        collicion_contact_info(
                natural_32_bit const  cell_x_,
                natural_32_bit const  cell_y_,
                vector3 const&  contact_point_in_local_space_,
                scene::collicion_contant_info const&  data_
                );
    };

    using  collision_contacts_map = std::unordered_multimap<scene::node_id, collicion_contact_info>;

    sensory_controller_collision_contacts(blackboard_agent_weak_ptr const  blackboard_, config const&  config_);

    blackboard_agent_ptr  get_blackboard() const { return m_blackboard.lock(); }
    collision_contacts_map const&  get_collision_contacts_map() const { return  m_collision_contacts.front(); }
    config const&  get_config() const { return m_config; }

    void  next_round();
    void  on_collision_contact(
            scene::node_id const&  collider_nid,
            scene::collicion_contant_info const&  contact_info
            );

private:
    blackboard_agent_weak_ptr  m_blackboard;
    std::array<collision_contacts_map, 2>  m_collision_contacts;
    config  m_config;
};


using  sensory_controller_collision_contacts_ptr = std::shared_ptr<sensory_controller_collision_contacts>;


}

#endif
