#ifndef AI_SENSORY_CONTROLLER_COLLISION_CONTACTS_HPP_INCLUDED
#   define AI_SENSORY_CONTROLLER_COLLISION_CONTACTS_HPP_INCLUDED

#   include <ai/scene.hpp>
#   include <angeo/tensor_math.hpp>
#   include <unordered_map>
#   include <array>
#   include <memory>

namespace ai {


struct  sensory_controller_collision_contacts final
{
    using  collision_contacts_map = std::unordered_multimap<scene::node_id, scene::collicion_contant_info>;

    sensory_controller_collision_contacts();

    collision_contacts_map const&  get_collision_contacts_map() const { return  m_collision_contacts.front(); }

    void  next_round();
    void  on_collision_contact(
            scene::node_id const&  collider_nid,
            scene::collicion_contant_info const&  contact_info
            );

private:
    std::array<collision_contacts_map, 2>  m_collision_contacts;
};


using  sensory_controller_collision_contacts_ptr = std::shared_ptr<sensory_controller_collision_contacts>;


}

#endif
