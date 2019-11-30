#include <ai/sensory_controller_collision_contacts.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


sensory_controller_collision_contacts::sensory_controller_collision_contacts()
    : m_collision_contacts()
{}


void  sensory_controller_collision_contacts::next_round()
{
    std::swap(m_collision_contacts.front(), m_collision_contacts.back());
    m_collision_contacts.back().clear();
}


void  sensory_controller_collision_contacts::on_collision_contact(
        scene::node_id const&  collider_nid,
        scene::collicion_contant_info const&  contact_info
        )
{
    m_collision_contacts.back().insert({ collider_nid, contact_info });
}


}
