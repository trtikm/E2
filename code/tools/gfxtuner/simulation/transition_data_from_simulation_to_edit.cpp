#include <gfxtuner/simulation/transition_data_from_simulation_to_edit.hpp>


void  transition_data_from_simulation_to_edit::clear(bool const  also_rigid_body_backups)
{
    if (also_rigid_body_backups)
    {
        m_static_rigid_body_backups.clear();
        m_dynamic_rigid_body_backups.clear();
    }

    m_nodes_inserted.clear();
    m_nodes_erased.clear();

    m_batches_inserted.clear();
    m_batches_erased.clear();

    m_colliders_inserted.clear();
    m_colliders_erased.clear();

    m_rigid_bodies_inserted.clear();
    m_rigid_bodies_erased.clear();
}


void  transition_data_from_simulation_to_edit::insert_backup_for_rigid_body(
        angeo::rigid_body_id const  id,
        scn::rigid_body_props const&  backup,
        bool const  is_static)
{
    if (is_static)
        m_static_rigid_body_backups.insert({ id, backup });
    else
        m_dynamic_rigid_body_backups.insert({ id, backup });
}


void  transition_data_from_simulation_to_edit::erase_backup_for_rigid_body(angeo::rigid_body_id const  id)
{
    m_static_rigid_body_backups.erase(id);
    m_dynamic_rigid_body_backups.erase(id);
}


void  transition_data_from_simulation_to_edit::on_node_relocation(
        scn::scene_node_id const  id,
        angeo::coordinate_system const&  orig_coord_system
        )
{
    auto const  it = m_nodes_erased.find(id);
    if (it == m_nodes_erased.end())
        m_nodes_erased.insert({ id, orig_coord_system });
    m_nodes_inserted.insert(id);
}


void  transition_data_from_simulation_to_edit::on_rigid_body_props_changed(scn::scene_record_id const&  id)
{
    auto const  it = m_rigid_bodies_erased.find(id);
    if (it == m_rigid_bodies_erased.end())
        m_rigid_bodies_erased.insert(id);
    m_rigid_bodies_inserted.insert(id);
}
