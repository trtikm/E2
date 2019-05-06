#include <gfxtuner/simulation/bind_ai_scene_to_simulator.hpp>
#include <gfxtuner/simulation/simulator.hpp>
#include <scene/scene_utils_specialised.hpp>
#include <utility/timeprof.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <string>
#include <sstream>

namespace detail {


inline std::string  get_ai_node_name_prefix() { return "@ai."; }


void  relocate_node(scn::scene_node_ptr const  node_ptr, angeo::coordinate_system const&  frame, bool const  frame_is_in_parent_space)
{
    if (frame_is_in_parent_space || !node_ptr->has_parent())
        node_ptr->relocate(frame.origin(), frame.orientation());
    else
    {
        matrix44  to_parent_space_matrix;
        {
            vector3  u;
            matrix33  R;
            decompose_matrix44(node_ptr->get_parent()->get_world_matrix(), u, R);
            compose_to_base_matrix(u, R, to_parent_space_matrix);
        }
        node_ptr->relocate(
            transform_point(frame.origin(), to_parent_space_matrix),
            normalised(transform(frame.orientation(), to_parent_space_matrix))
            );
    }
}


}


ai::scene::node_id  bind_ai_scene_to_simulator::get_aux_root_node_for_agent(
        node_id const&  agent_nid,
        std::string const&  aux_root_node_name
        )
{
    std::stringstream  sstr;
    sstr << detail::get_ai_node_name_prefix();
    for (auto const&  name : agent_nid.path())
        sstr << name << '.';
    sstr << aux_root_node_name;
    return node_id(sstr.str());
}


bool  bind_ai_scene_to_simulator::has_scene_node(node_id const&  nid) const
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    return m_simulator_ptr->get_scene_node(nid) != nullptr;
}


void  bind_ai_scene_to_simulator::insert_scene_node(
        node_id const&  nid,
        angeo::coordinate_system const&  frame,
        bool const  frame_is_in_parent_space
        )
{
    ASSUMPTION(m_simulator_ptr != nullptr && (!frame_is_in_parent_space || nid.path().size() > 1UL));
    auto const  node_ptr = m_simulator_ptr->insert_scene_node(nid);
    ASSUMPTION(node_ptr != nullptr);
    detail::relocate_node(node_ptr, frame, frame_is_in_parent_space);
}


void  bind_ai_scene_to_simulator::get_frame_of_scene_node(
        node_id const&  nid,
        bool const  frame_in_parent_space,
        angeo::coordinate_system&  frame
        )
{
    ASSUMPTION(m_simulator_ptr != nullptr && (!frame_in_parent_space || nid.path().size() > 1UL));
    auto const  node_ptr = m_simulator_ptr->get_scene_node(nid);
    ASSUMPTION(node_ptr != nullptr);
    if (frame_in_parent_space || !node_ptr->has_parent())
        frame = *node_ptr->get_coord_system();
    else
    {
        vector3  u;
        matrix33  R;
        decompose_matrix44(node_ptr->get_world_matrix(), u, R);
        frame.set_origin(u);
        frame.set_orientation(normalised(rotation_matrix_to_quaternion(R)));
    }
}


void  bind_ai_scene_to_simulator::set_frame_of_scene_node(
        node_id const&  nid,
        bool const  frame_is_in_parent_space,
        angeo::coordinate_system const&  frame
        )
{
    ASSUMPTION(m_simulator_ptr != nullptr && (!frame_is_in_parent_space || nid.path().size() > 1UL));
    auto const  node_ptr = m_simulator_ptr->get_scene_node(nid);
    ASSUMPTION(node_ptr != nullptr);
    detail::relocate_node(node_ptr, frame, frame_is_in_parent_space);

    if (auto const  collider_ptr = scn::get_collider(*node_ptr))
    {
        ASSUMPTION(m_simulator_ptr->find_nearest_rigid_body_node(node_ptr) == nullptr);
        for (auto const  coid : collider_ptr->ids())
            m_simulator_ptr->get_collision_scene()->on_position_changed(coid, node_ptr->get_world_matrix());
    }
    else if (auto  rb_ptr = scn::get_rigid_body(*node_ptr))
    {
        INVARIANT(!node_ptr->has_parent());
        m_simulator_ptr->get_rigid_body_simulator()->set_position_of_mass_centre(rb_ptr->id(), node_ptr->get_coord_system()->origin());
        m_simulator_ptr->get_rigid_body_simulator()->set_orientation(rb_ptr->id(), node_ptr->get_coord_system()->orientation());
    }
}


void  bind_ai_scene_to_simulator::erase_scene_node(node_id const&  nid)
{
    ASSUMPTION((
            m_simulator_ptr != nullptr &&
            [this, &nid]() -> bool {
                auto const  node_ptr = m_simulator_ptr->get_scene_node(nid);
                if (node_ptr == nullptr || !node_ptr->get_children().empty())
                    return false;
                for (auto const&  folder : node_ptr->get_folders())
                    if (!folder.second.get_records().empty())
                        return false;
                return true;
            }()
        ));
    m_simulator_ptr->erase_scene_node(nid);
}


void  bind_ai_scene_to_simulator::insert_collision_capsule_to_scene_node(
        node_id const&  nid,
        float_32_bit const  half_distance_between_end_points,
        float_32_bit const  thickness_from_central_line,
        angeo::COLLISION_MATERIAL_TYPE const  material,
        float_32_bit const  density_multiplier,
        bool const  as_dynamic
        )
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    ASSUMPTION(m_simulator_ptr->find_nearest_rigid_body_node(m_simulator_ptr->get_scene_node(nid)) == nullptr);
    m_simulator_ptr->insert_collision_capsule_to_scene_node(
            half_distance_between_end_points,
            thickness_from_central_line,
            material,
            density_multiplier,
            as_dynamic,
            scn::make_collider_record_id(nid, angeo::as_string(angeo::COLLISION_SHAPE_TYPE::CAPSULE))
            );
}


void  bind_ai_scene_to_simulator::erase_collision_object_from_scene_node(node_id const&  nid)
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    auto const  node_ptr = m_simulator_ptr->get_scene_node(nid);
    ASSUMPTION(node_ptr != nullptr);
    ASSUMPTION(m_simulator_ptr->find_nearest_rigid_body_node(node_ptr) == nullptr);
    auto const  collider_ptr = scn::get_collider(*node_ptr);
    ASSUMPTION(collider_ptr != nullptr && collider_ptr->ids().size() == 1UL);
    ASSUMPTION(m_collision_contacts_stream.count(collider_ptr->id()) == 0UL);
    m_simulator_ptr->erase_collision_object_from_scene_node(
            scn::make_collider_record_id(nid, angeo::as_string(angeo::get_shape_type(collider_ptr->id())))
            );
}


void  bind_ai_scene_to_simulator::insert_rigid_body_to_scene_node(
        node_id const&  nid,
        vector3 const&  linear_velocity,
        vector3 const&  angular_velocity,
        vector3 const&  external_linear_acceleration,
        vector3 const&  external_angular_acceleration,
        float_32_bit const  mass_inverted,
        matrix33 const&  inertia_tensor_inverted
        )
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    m_simulator_ptr->insert_rigid_body_to_scene_node_ex(
            linear_velocity,
            angular_velocity,
            external_linear_acceleration,
            external_angular_acceleration,
            mass_inverted,
            inertia_tensor_inverted,
            nid
            );
}


vector3  bind_ai_scene_to_simulator::get_linear_velocity_of_rigid_body_of_scene_node(node_id const&  nid)
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    auto const  node_ptr = m_simulator_ptr->get_scene_node(nid);
    ASSUMPTION(node_ptr != nullptr);
    auto const  rb_ptr = scn::get_rigid_body(*node_ptr);
    ASSUMPTION(rb_ptr != nullptr);
    return m_simulator_ptr->get_rigid_body_simulator()->get_linear_velocity(rb_ptr->id());
}


void  bind_ai_scene_to_simulator::set_linear_velocity_of_rigid_body_of_scene_node(node_id const&  nid, vector3 const&  linear_velocity)
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    auto const  node_ptr = m_simulator_ptr->get_scene_node(nid);
    ASSUMPTION(node_ptr != nullptr);
    auto const  rb_ptr = scn::get_rigid_body(*node_ptr);
    ASSUMPTION(rb_ptr != nullptr);
    m_simulator_ptr->get_rigid_body_simulator()->set_linear_velocity(rb_ptr->id(), linear_velocity);
}


vector3  bind_ai_scene_to_simulator::get_angular_velocity_of_rigid_body_of_scene_node(node_id const&  nid)
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    auto const  node_ptr = m_simulator_ptr->get_scene_node(nid);
    ASSUMPTION(node_ptr != nullptr);
    auto const  rb_ptr = scn::get_rigid_body(*node_ptr);
    ASSUMPTION(rb_ptr != nullptr);
    return m_simulator_ptr->get_rigid_body_simulator()->get_angular_velocity(rb_ptr->id());
}


void  bind_ai_scene_to_simulator::set_angular_velocity_of_rigid_body_of_scene_node(node_id const&  nid, vector3 const&  angular_velocity)
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    auto const  node_ptr = m_simulator_ptr->get_scene_node(nid);
    ASSUMPTION(node_ptr != nullptr);
    auto const  rb_ptr = scn::get_rigid_body(*node_ptr);
    ASSUMPTION(rb_ptr != nullptr);
    m_simulator_ptr->get_rigid_body_simulator()->set_angular_velocity(rb_ptr->id(), angular_velocity);
}


vector3  bind_ai_scene_to_simulator::get_linear_acceleration_of_rigid_body_of_scene_node(node_id const&  nid)
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    auto const  node_ptr = m_simulator_ptr->get_scene_node(nid);
    ASSUMPTION(node_ptr != nullptr);
    auto const  rb_ptr = scn::get_rigid_body(*node_ptr);
    ASSUMPTION(rb_ptr != nullptr);
    return m_simulator_ptr->get_rigid_body_simulator()->get_external_linear_acceleration(rb_ptr->id());
}


void  bind_ai_scene_to_simulator::set_linear_acceleration_of_rigid_body_of_scene_node(node_id const&  nid, vector3 const&  linear_acceleration)
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    auto const  node_ptr = m_simulator_ptr->get_scene_node(nid);
    ASSUMPTION(node_ptr != nullptr);
    auto const  rb_ptr = scn::get_rigid_body(*node_ptr);
    ASSUMPTION(rb_ptr != nullptr);
    m_simulator_ptr->get_rigid_body_simulator()->set_external_linear_acceleration(rb_ptr->id(), linear_acceleration);
}


void  bind_ai_scene_to_simulator::add_to_linear_acceleration_of_rigid_body_of_scene_node(node_id const&  nid, vector3 const&  linear_acceleration)
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    auto const  node_ptr = m_simulator_ptr->get_scene_node(nid);
    ASSUMPTION(node_ptr != nullptr);
    auto const  rb_ptr = scn::get_rigid_body(*node_ptr);
    ASSUMPTION(rb_ptr != nullptr);
    m_simulator_ptr->get_rigid_body_simulator()->set_external_linear_acceleration(
            rb_ptr->id(),
            m_simulator_ptr->get_rigid_body_simulator()->get_external_linear_acceleration(rb_ptr->id()) + linear_acceleration
            );
}


vector3  bind_ai_scene_to_simulator::get_angular_acceleration_of_rigid_body_of_scene_node(node_id const&  nid)
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    auto const  node_ptr = m_simulator_ptr->get_scene_node(nid);
    ASSUMPTION(node_ptr != nullptr);
    auto const  rb_ptr = scn::get_rigid_body(*node_ptr);
    ASSUMPTION(rb_ptr != nullptr);
    return m_simulator_ptr->get_rigid_body_simulator()->get_external_angular_acceleration(rb_ptr->id());
}


void  bind_ai_scene_to_simulator::set_angular_acceleration_of_rigid_body_of_scene_node(node_id const&  nid, vector3 const&  angular_acceleration)
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    auto const  node_ptr = m_simulator_ptr->get_scene_node(nid);
    ASSUMPTION(node_ptr != nullptr);
    auto const  rb_ptr = scn::get_rigid_body(*node_ptr);
    ASSUMPTION(rb_ptr != nullptr);
    m_simulator_ptr->get_rigid_body_simulator()->set_external_angular_acceleration(rb_ptr->id(), angular_acceleration);
}


void  bind_ai_scene_to_simulator::add_to_angular_acceleration_of_rigid_body_of_scene_node(node_id const&  nid, vector3 const&  angular_acceleration)
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    auto const  node_ptr = m_simulator_ptr->get_scene_node(nid);
    ASSUMPTION(node_ptr != nullptr);
    auto const  rb_ptr = scn::get_rigid_body(*node_ptr);
    ASSUMPTION(rb_ptr != nullptr);
    m_simulator_ptr->get_rigid_body_simulator()->set_external_angular_acceleration(
            rb_ptr->id(),
            m_simulator_ptr->get_rigid_body_simulator()->get_external_angular_acceleration(rb_ptr->id()) + angular_acceleration
            );
}


float_32_bit  bind_ai_scene_to_simulator::get_inverted_mass_of_rigid_body_of_scene_node(node_id const&  nid)
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    auto const  node_ptr = m_simulator_ptr->get_scene_node(nid);
    ASSUMPTION(node_ptr != nullptr);
    auto const  rb_ptr = scn::get_rigid_body(*node_ptr);
    ASSUMPTION(rb_ptr != nullptr);
    return m_simulator_ptr->get_rigid_body_simulator()->get_inverted_mass(rb_ptr->id());
}


void  bind_ai_scene_to_simulator::set_inverted_mass_of_rigid_body_of_scene_node(node_id const&  nid, float_32_bit const  inverted_mass)
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    auto const  node_ptr = m_simulator_ptr->get_scene_node(nid);
    ASSUMPTION(node_ptr != nullptr);
    auto const  rb_ptr = scn::get_rigid_body(*node_ptr);
    ASSUMPTION(rb_ptr != nullptr);
    return m_simulator_ptr->get_rigid_body_simulator()->set_inverted_mass(rb_ptr->id(), inverted_mass);
}


matrix33  bind_ai_scene_to_simulator::get_inverted_inertia_tensor_of_rigid_body_of_scene_node(node_id const&  nid)
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    auto const  node_ptr = m_simulator_ptr->get_scene_node(nid);
    ASSUMPTION(node_ptr != nullptr);
    auto const  rb_ptr = scn::get_rigid_body(*node_ptr);
    ASSUMPTION(rb_ptr != nullptr);
    return m_simulator_ptr->get_rigid_body_simulator()->get_inverted_inertia_tensor_in_local_space(rb_ptr->id());
}


void  bind_ai_scene_to_simulator::set_inverted_inertia_tensor_of_rigid_body_of_scene_node(node_id const&  nid, matrix33 const&  inverted_inertia_tensor)
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    auto const  node_ptr = m_simulator_ptr->get_scene_node(nid);
    ASSUMPTION(node_ptr != nullptr);
    auto const  rb_ptr = scn::get_rigid_body(*node_ptr);
    ASSUMPTION(rb_ptr != nullptr);
    return m_simulator_ptr->get_rigid_body_simulator()->set_inverted_inertia_tensor_in_local_space(rb_ptr->id(), inverted_inertia_tensor);
}


void  bind_ai_scene_to_simulator::erase_rigid_body_from_scene_node(node_id const&  nid)
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    m_simulator_ptr->erase_rigid_body_from_scene_node(nid);
}


vector3  bind_ai_scene_to_simulator::get_gravity_acceleration_at_point(vector3 const&  position) const
{
    return -9.81f * vector3_unit_z();
}


void  bind_ai_scene_to_simulator::register_to_collision_contacts_stream(
        node_id const&  collider_nid,
        ai::agent_id const  agent_id
        )
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    auto const  node_ptr = m_simulator_ptr->get_scene_node(collider_nid);
    ASSUMPTION(node_ptr != nullptr);
    auto const  collider_ptr = scn::get_collider(*node_ptr);
    ASSUMPTION(collider_ptr != nullptr && collider_ptr->ids().size() == 1UL);
    ASSUMPTION(m_collision_contacts_stream.count(collider_ptr->id()) == 0UL);
    m_collision_contacts_stream.insert({ collider_ptr->id() , {collider_nid, agent_id} });
}


void  bind_ai_scene_to_simulator::unregister_to_collision_contacts_stream(
        node_id const&  collider_nid,
        ai::agent_id const  agent_id
        )
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    auto const  node_ptr = m_simulator_ptr->get_scene_node(collider_nid);
    ASSUMPTION(node_ptr != nullptr);
    auto const  collider_ptr = scn::get_collider(*node_ptr);
    ASSUMPTION(collider_ptr != nullptr && collider_ptr->ids().size() == 1UL);
    ASSUMPTION(m_collision_contacts_stream.count(collider_ptr->id()) != 0UL);
    m_collision_contacts_stream.erase(collider_ptr->id());
}


bool  bind_ai_scene_to_simulator::do_tracking_collision_contact_of_collision_object(angeo::collision_object_id const  coid) const
{
    return m_collision_contacts_stream.find(coid) != m_collision_contacts_stream.cend();
}


void  bind_ai_scene_to_simulator::on_collision_contact(
        angeo::collision_object_id const  coid,
        vector3 const&  contact_point,
        vector3 const&  unit_normal,
        angeo::COLLISION_MATERIAL_TYPE const  material,
        float_32_bit const  normal_force_magnitude
        ) const
{
    ASSUMPTION(m_simulator_ptr != nullptr);

    auto  it = m_collision_contacts_stream.find(coid);
    ASSUMPTION(it != m_collision_contacts_stream.cend());

    m_simulator_ptr->get_agents()->on_collision_contact(
            it->second.second,
            it->second.first,
            ai::scene::collicion_contant_info(contact_point, unit_normal, material, normal_force_magnitude)
            );
}
