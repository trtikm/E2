#include <gfxtuner/simulation/bind_ai_scene_to_simulator.hpp>
#include <gfxtuner/simulation/simulator.hpp>
#include <angeo/rigid_body_simulator.hpp>
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


void  bind_ai_scene_to_simulator::accept(request_ptr const  req, bool const  delay_processing_to_next_time_step)
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    m_simulator_ptr->accept_ai_request(req, delay_processing_to_next_time_step);
}


ai::scene::node_id  bind_ai_scene_to_simulator::get_aux_root_node(
        ai::OBJECT_KIND const  kind,
        node_id const&  nid,
        std::string const&  aux_root_node_name
        )
{
    std::stringstream  sstr;
    sstr << detail::get_ai_node_name_prefix();
    switch (kind)
    {
    case ai::OBJECT_KIND::AGENT: sstr << "agent."; break;
    case ai::OBJECT_KIND::DEVICE: sstr << "device."; break;
    case ai::OBJECT_KIND::SENSOR: sstr << "sensor."; break;
    default: UNREACHABLE();
    }
    for (auto const&  name : nid.path())
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
    auto const  node_ptr = m_simulator_ptr->insert_scene_simulation_node(nid);
    ASSUMPTION(node_ptr != nullptr);
    detail::relocate_node(node_ptr, frame, frame_is_in_parent_space);
}


void  bind_ai_scene_to_simulator::get_frame_of_scene_node(
        node_id const&  nid,
        bool const  frame_in_parent_space,
        angeo::coordinate_system&  frame
        ) const
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


vector3  bind_ai_scene_to_simulator::get_origin_of_scene_node(
        node_id const&  nid,
        bool const  frame_in_parent_space
        ) const
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    auto const  node_ptr = m_simulator_ptr->get_scene_node(nid);
    ASSUMPTION(node_ptr != nullptr);
    if (frame_in_parent_space || !node_ptr->has_parent())
        return node_ptr->get_coord_system()->origin();
    else
    {
        vector3  u;
        matrix33  R;
        decompose_matrix44(node_ptr->get_world_matrix(), u, R);
        return u;
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
    m_simulator_ptr->erase_scene_simulation_node(nid);
}


void  bind_ai_scene_to_simulator::insert_collision_sphere_to_scene_node(
        node_id const&  nid,
        float_32_bit const  radius,
        angeo::COLLISION_MATERIAL_TYPE const  material,
        angeo::COLLISION_CLASS const  collision_class,
        float_32_bit const  density_multiplier,
        bool const  as_dynamic
        )
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    ASSUMPTION(m_simulator_ptr->find_nearest_rigid_body_node(m_simulator_ptr->get_scene_node(nid)) == nullptr);
    m_simulator_ptr->insert_collision_sphere_to_scene_node(
            radius,
            material,
            collision_class,
            density_multiplier,
            as_dynamic,
            scn::make_collider_record_id(nid, angeo::as_string(angeo::COLLISION_SHAPE_TYPE::SPHERE))
            );
}


void  bind_ai_scene_to_simulator::insert_collision_capsule_to_scene_node(
        node_id const&  nid,
        float_32_bit const  half_distance_between_end_points,
        float_32_bit const  thickness_from_central_line,
        angeo::COLLISION_MATERIAL_TYPE const  material,
        angeo::COLLISION_CLASS const  collision_class,
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
            collision_class,
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


void  bind_ai_scene_to_simulator::enable_colliding_colliders_of_scene_nodes(
        node_id const&  nid_1,
        node_id const&  nid_2,
        bool const  state)
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    auto const  node_1_ptr = m_simulator_ptr->get_scene_node(nid_1);
    auto const  collider_1_ptr = scn::get_collider(*node_1_ptr);
    ASSUMPTION(collider_1_ptr != nullptr && collider_1_ptr->ids().size() == 1UL);
    auto const  node_2_ptr = m_simulator_ptr->get_scene_node(nid_2);
    auto const  collider_2_ptr = scn::get_collider(*node_2_ptr);
    ASSUMPTION(collider_2_ptr != nullptr && collider_2_ptr->ids().size() == 1UL);
    if (state)
        m_simulator_ptr->get_collision_scene()->enable_colliding(collider_1_ptr->id(), collider_2_ptr->id());
    else
        m_simulator_ptr->get_collision_scene()->disable_colliding(collider_1_ptr->id(), collider_2_ptr->id());
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
    m_simulator_ptr->insert_rigid_body_to_scene_node_direct(
            m_simulator_ptr->get_scene_node(nid),
            linear_velocity,
            angular_velocity,
            external_linear_acceleration,
            external_angular_acceleration,
            mass_inverted,
            inertia_tensor_inverted,
            false,
            nullptr
            );
}


vector3  bind_ai_scene_to_simulator::get_linear_velocity_of_rigid_body_of_scene_node(node_id const&  nid) const
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


vector3  bind_ai_scene_to_simulator::get_angular_velocity_of_rigid_body_of_scene_node(node_id const&  nid) const
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


vector3  bind_ai_scene_to_simulator::get_linear_acceleration_of_rigid_body_of_scene_node(node_id const&  nid) const
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


vector3  bind_ai_scene_to_simulator::get_angular_acceleration_of_rigid_body_of_scene_node(node_id const&  nid) const
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


float_32_bit  bind_ai_scene_to_simulator::get_inverted_mass_of_rigid_body_of_scene_node(node_id const&  nid) const
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


matrix33  bind_ai_scene_to_simulator::get_inverted_inertia_tensor_of_rigid_body_of_scene_node(node_id const&  nid) const
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


ai::scene::node_id  bind_ai_scene_to_simulator::get_scene_node_of_rigid_body_associated_with_collider(
        collision_object_id const  coid
        ) const
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    angeo::rigid_body_id  rb_id;
    if (!m_simulator_ptr->get_rigid_body_of_collider(coid, &rb_id))
        return {};
    scn::scene_node_ptr const  node_ptr = m_simulator_ptr->get_rigid_body_node(rb_id);
    INVARIANT(node_ptr != nullptr);
    return node_ptr->get_id();
}


ai::scene::record_id  bind_ai_scene_to_simulator::get_scene_record_of_rigid_body_associated_with_collider(
        collision_object_id const  coid
        ) const
{
    return scn::make_rigid_body_record_id(get_scene_node_of_rigid_body_associated_with_collider(coid));
}


ai::scene::node_id  bind_ai_scene_to_simulator::get_scene_node_of_rigid_body_associated_with_collider_node(
        node_id const&  collider_node_id
        ) const
{
    scn::scene_node_ptr const  node_ptr = m_simulator_ptr->find_nearest_rigid_body_node(collider_node_id);
    return node_ptr == nullptr ? node_id{} : node_ptr->get_id();
}


ai::scene::record_id  bind_ai_scene_to_simulator::get_scene_record_of_rigid_body_associated_with_collider_node(
        node_id const& collider_node_id
        ) const
{
    return scn::make_rigid_body_record_id(get_scene_node_of_rigid_body_associated_with_collider_node(collider_node_id));
}


vector3  bind_ai_scene_to_simulator::get_initial_external_linear_acceleration_at_point(vector3 const&  position_in_world_space) const
{
    return vector3_zero();
}


vector3  bind_ai_scene_to_simulator::get_initial_external_angular_acceleration_at_point(vector3 const&  position_in_world_space) const
{
    return vector3_zero();
}


vector3  bind_ai_scene_to_simulator::get_external_linear_acceleration_of_rigid_body_of_scene_node(node_id const&  nid) const
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    return m_simulator_ptr->get_rigid_body_external_linear_acceleration(nid);
}


vector3  bind_ai_scene_to_simulator::get_external_angular_acceleration_of_rigid_body_of_scene_node(node_id const&  nid) const
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    return m_simulator_ptr->get_rigid_body_external_angular_acceleration(nid);
}


vector3  bind_ai_scene_to_simulator::get_linear_velocity_of_collider_at_point(
        collision_object_id const  coid,
        vector3 const&  point_in_world_space
        ) const
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    angeo::rigid_body_id  rb_id;
    if (!m_simulator_ptr->get_rigid_body_of_collider(coid, &rb_id))
        return vector3_zero();
    return angeo::compute_velocity_of_point_of_rigid_body(
            m_simulator_ptr->get_rigid_body_simulator()->get_position_of_mass_centre(rb_id),
            m_simulator_ptr->get_rigid_body_simulator()->get_linear_velocity(rb_id),
            m_simulator_ptr->get_rigid_body_simulator()->get_angular_velocity(rb_id),
            point_in_world_space
            );
}


vector3  bind_ai_scene_to_simulator::get_linear_velocity_of_rigid_body_at_point(
        node_id const&  rb_nid,
        vector3 const&  point_in_world_space
        ) const
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    scn::scene_node_ptr const  node_ptr = m_simulator_ptr->get_scene_node(rb_nid);
    ASSUMPTION(node_ptr != nullptr);
    scn::rigid_body const* const  rb_ptr = scn::get_rigid_body(*node_ptr);
    ASSUMPTION(rb_ptr != nullptr);
    angeo::rigid_body_id const  rb_id = rb_ptr->id();
    return angeo::compute_velocity_of_point_of_rigid_body(
            m_simulator_ptr->get_rigid_body_simulator()->get_position_of_mass_centre(rb_id),
            m_simulator_ptr->get_rigid_body_simulator()->get_linear_velocity(rb_id),
            m_simulator_ptr->get_rigid_body_simulator()->get_angular_velocity(rb_id),
            point_in_world_space
            );
}


void  bind_ai_scene_to_simulator::register_to_collision_contacts_stream(
        node_id const&  collider_nid,
        ai::object_id const&  oid
        )
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    auto const  node_ptr = m_simulator_ptr->get_scene_node(collider_nid);
    ASSUMPTION(node_ptr != nullptr);
    auto const  collider_ptr = scn::get_collider(*node_ptr);
    ASSUMPTION(collider_ptr != nullptr && collider_ptr->ids().size() == 1UL);
    auto  it = m_collision_contacts_stream.find(collider_ptr->id());
    if (it == m_collision_contacts_stream.end())
        it = m_collision_contacts_stream.insert({ collider_ptr->id() , {collider_nid, {}} }).first;
    ASSUMPTION(it->second.second.count(oid) == 0UL);
    it->second.second.insert(oid);
}


void  bind_ai_scene_to_simulator::unregister_from_collision_contacts_stream(
        node_id const&  collider_nid,
        ai::object_id const&  oid
        )
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    auto const  node_ptr = m_simulator_ptr->get_scene_node(collider_nid);
    if (node_ptr == nullptr)
        return;
    auto const  collider_ptr = scn::get_collider(*node_ptr);
    if (collider_ptr == nullptr)
        return;
    ASSUMPTION(collider_ptr->ids().size() == 1UL);
    auto  it = m_collision_contacts_stream.find(collider_ptr->id());
    INVARIANT(it != m_collision_contacts_stream.end() && it->second.second.count(oid) != 0UL);
    it->second.second.erase(oid);
    if (it->second.second.empty())
        m_collision_contacts_stream.erase(it);
}


bool  bind_ai_scene_to_simulator::do_tracking_collision_contact_of_collision_object(angeo::collision_object_id const  coid) const
{
    return m_collision_contacts_stream.find(coid) != m_collision_contacts_stream.cend();
}


void  bind_ai_scene_to_simulator::on_collision_contact(
        vector3 const&  contact_point_in_world_space,
        vector3 const&  unit_normal_in_world_space,
        angeo::collision_object_id const  coid,
        angeo::COLLISION_MATERIAL_TYPE const  material,
        angeo::COLLISION_CLASS const  self_collision_class,
        angeo::collision_object_id const  other_coid,
        angeo::COLLISION_MATERIAL_TYPE const  other_material,
        angeo::COLLISION_CLASS const  other_collision_class
        ) const
{
    collision_contacts_stream_type::const_iterator const  it = m_collision_contacts_stream.find(coid);
    collision_contacts_stream_type::const_iterator const  other_it =
            other_coid == angeo::get_invalid_collision_object_id() ?
                    m_collision_contacts_stream.cend() :
                    m_collision_contacts_stream.find(other_coid)
                    ;

    ASSUMPTION(m_simulator_ptr != nullptr && it != m_collision_contacts_stream.cend());

    auto const  collision_info =
            std::make_shared<ai::scene::collicion_contant_info>(
                    contact_point_in_world_space,
                    unit_normal_in_world_space,
                    it->second.first,
                    coid,
                    material,
                    self_collision_class,
                    other_it != m_collision_contacts_stream.cend() ? other_it->second.first : node_id{},
                    other_coid,
                    other_material,
                    other_collision_class
                    );

    if (other_it == m_collision_contacts_stream.cend())
        for (auto const&  oid : it->second.second)
            m_simulator_ptr->get_ai_simulator()->on_collision_contact(oid, collision_info, ai::object_id::make_invalid());
    else
        for (auto const&  oid : it->second.second)
            for (auto const& other_oid : other_it->second.second)
                m_simulator_ptr->get_ai_simulator()->on_collision_contact(oid, collision_info, other_oid);
}


angeo::collision_scene const&  bind_ai_scene_to_simulator::get_collision_scene() const
{
    return *m_simulator_ptr->get_collision_scene();
}


void  bind_ai_scene_to_simulator::get_coids_under_scene_node(
        scn::scene_node_ptr const  node_ptr,
        std::function<bool(collision_object_id)> const&  acceptor
        ) const
{
    auto const  collider_ptr = node_ptr != nullptr ? scn::get_collider(*node_ptr) : nullptr;
    if (collider_ptr != nullptr)
        for (auto  coid : collider_ptr->ids())
            acceptor(coid);
}


void  bind_ai_scene_to_simulator::get_coids_under_scene_node(node_id const&  nid, std::function<bool(collision_object_id)> const&  acceptor) const
{
    get_coids_under_scene_node(m_simulator_ptr->get_scene_node(nid), acceptor);
}


void  bind_ai_scene_to_simulator::get_coids_under_scene_node_subtree(node_id const&  nid, std::function<bool(collision_object_id)> const&  acceptor) const
{
    auto const  node_ptr = m_simulator_ptr->get_scene_node(nid);
    if (node_ptr != nullptr)
    {
        get_coids_under_scene_node(node_ptr, acceptor);
        node_ptr->foreach_child([this, &acceptor](scn::scene_node_ptr const  n) { get_coids_under_scene_node(n, acceptor); return true; }, true);
    }
}


bind_ai_scene_to_simulator::custom_constraint_id  bind_ai_scene_to_simulator::acquire_fresh_custom_constraint_id()
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    return m_simulator_ptr->get_rigid_body_simulator()->gen_fresh_custom_constraint_id();
}


void  bind_ai_scene_to_simulator::release_generated_custom_constraint_id(custom_constraint_id const  ccid)
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    m_simulator_ptr->get_rigid_body_simulator()->release_generated_custom_constraint_id(ccid);
}


void  bind_ai_scene_to_simulator::insert_custom_constraint(
        custom_constraint_id const  ccid,
        node_id const&  rb_nid_0,
        vector3 const&  linear_component_0,
        vector3 const&  angular_component_0,
        node_id const&  rb_nid_1,
        vector3 const&  linear_component_1,
        vector3 const&  angular_component_1,
        float_32_bit const  bias,
        float_32_bit const  variable_lower_bound,
        float_32_bit const  variable_upper_bound,
        float_32_bit const  initial_value_for_cache_miss
        )
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    scn::scene_node_ptr const  rb_0_node_ptr = m_simulator_ptr->get_scene_node(rb_nid_0);
    ASSUMPTION(rb_0_node_ptr != nullptr);
    scn::scene_node_ptr const  rb_1_node_ptr = m_simulator_ptr->get_scene_node(rb_nid_1);
    ASSUMPTION(rb_1_node_ptr != nullptr);
    scn::rigid_body const* const  rb_0 = scn::get_rigid_body(*rb_0_node_ptr);
    ASSUMPTION(rb_0 != nullptr);
    scn::rigid_body const* const  rb_1 = scn::get_rigid_body(*rb_1_node_ptr);
    ASSUMPTION(rb_1 != nullptr);
    m_simulator_ptr->get_rigid_body_simulator()->insert_custom_constraint(
            ccid,
            rb_0->id(),
            linear_component_0,
            angular_component_0,
            rb_1->id(),
            linear_component_1,
            angular_component_1,
            bias,
            [variable_lower_bound](std::vector<float_32_bit> const&) { return variable_lower_bound; },
            [variable_upper_bound](std::vector<float_32_bit> const&) { return variable_upper_bound; },
            initial_value_for_cache_miss
            );
}


void  bind_ai_scene_to_simulator::insert_immediate_constraint(
        node_id const&  rb_nid_0,
        vector3 const&  linear_component_0,
        vector3 const&  angular_component_0,
        node_id const&  rb_nid_1,
        vector3 const&  linear_component_1,
        vector3 const&  angular_component_1,
        float_32_bit const  bias,
        float_32_bit const  variable_lower_bound,
        float_32_bit const  variable_upper_bound,
        float_32_bit const  initial_value
        )
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    scn::scene_node_ptr const  rb_0_node_ptr = m_simulator_ptr->get_scene_node(rb_nid_0);
    ASSUMPTION(rb_0_node_ptr != nullptr);
    scn::scene_node_ptr const  rb_1_node_ptr = m_simulator_ptr->get_scene_node(rb_nid_1);
    ASSUMPTION(rb_1_node_ptr != nullptr);
    scn::rigid_body const* const  rb_0 = scn::get_rigid_body(*rb_0_node_ptr);
    ASSUMPTION(rb_0 != nullptr);
    scn::rigid_body const* const  rb_1 = scn::get_rigid_body(*rb_1_node_ptr);
    ASSUMPTION(rb_1 != nullptr);
    m_simulator_ptr->get_rigid_body_simulator()->get_constraint_system().insert_constraint(
            rb_0->id(),
            linear_component_0,
            angular_component_0,
            rb_1->id(),
            linear_component_1,
            angular_component_1,
            bias,
            [variable_lower_bound](std::vector<float_32_bit> const&) { return variable_lower_bound; },
            [variable_upper_bound](std::vector<float_32_bit> const&) { return variable_upper_bound; },
            initial_value
            );
}


bind_ai_scene_to_simulator::record_id  bind_ai_scene_to_simulator::__dbg_insert_sketch_box(
        node_id const&  nid,
        std::string const&  name,
        vector3 const&  half_sizes_along_axes,
        vector4 const&  colour
        )
{
    ASSUMPTION(
            nid.valid()
            && half_sizes_along_axes(0) > 0.0001f
            && half_sizes_along_axes(1) > 0.0001f
            && half_sizes_along_axes(2) > 0.0001f
            && m_simulator_ptr != nullptr
            );
    scn::scene_node_ptr const  node_ptr = m_simulator_ptr->get_scene_node(nid);
    ASSUMPTION(node_ptr != nullptr);
    m_simulator_ptr->insert_batch_to_scene_node(
            name,
            qtgl::get_sketch_id_prefix() + qtgl::make_box_id_without_prefix(half_sizes_along_axes, colour, qtgl::FOG_TYPE::NONE, false),
            "default",
            m_simulator_ptr->get_effects_config(),
            nid
            );
    return record_id(nid, scn::get_batches_folder_name(), name);
}


void  bind_ai_scene_to_simulator::__dbg_erase_sketch_batch(record_id const&  rid)
{
    ASSUMPTION(
            rid.valid()
            && !rid.is_folder_reference()
            && !rid.is_node_reference()
            && rid.get_folder_name() == scn::get_batches_folder_name()
            && m_simulator_ptr != nullptr
            );
    m_simulator_ptr->erase_batch_from_scene_node(rid.get_record_name(), rid.get_node_id());
}
