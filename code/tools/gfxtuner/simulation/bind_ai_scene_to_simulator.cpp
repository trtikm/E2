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
            transform_point(node_ptr->get_coord_system()->origin(), to_parent_space_matrix),
            transform(node_ptr->get_coord_system()->orientation(), to_parent_space_matrix)
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
}


void  bind_ai_scene_to_simulator::erase_scene_node(node_id const&  nid)
{
    ASSUMPTION((
            m_simulator_ptr != nullptr &&
            [this, &nid]() -> bool {
                if (auto const  node_ptr = m_simulator_ptr->get_scene_node(nid))
                    return node_ptr->get_children().empty() && node_ptr->get_folders().empty();
                return false;
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
    auto const  collider_ptr = scn::get_collider(*node_ptr);
    ASSUMPTION(collider_ptr != nullptr && collider_ptr->ids().size() == 1UL);
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


void  bind_ai_scene_to_simulator::erase_rigid_body_from_scene_node(node_id const&  nid)
{
    ASSUMPTION(m_simulator_ptr != nullptr);
    m_simulator_ptr->erase_rigid_body_from_scene_node(nid);
}
