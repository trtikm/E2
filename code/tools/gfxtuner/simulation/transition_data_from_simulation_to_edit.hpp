#ifndef E2_TOOL_GFXTUNER_TRANSITION_DATA_FROM_SIMULATION_TO_EDIT_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_TRANSITION_DATA_FROM_SIMULATION_TO_EDIT_HPP_INCLUDED

#   include <angeo/rigid_body.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <scene/scene_node_id.hpp>
#   include <scene/scene_record_id.hpp>
#   include <scene/records/collider/collider.hpp>
#   include <scene/records/rigid_body/rigid_body.hpp>
#   include <boost/filesystem/path.hpp>
#   include <unordered_map>
#   include <unordered_set>
#   include <map>
#   include <set>


struct  transition_data_from_simulation_to_edit
{
    std::unordered_map<angeo::rigid_body_id, scn::rigid_body_props>  m_static_rigid_body_backups;
    std::unordered_map<angeo::rigid_body_id, scn::rigid_body_props>  m_dynamic_rigid_body_backups;

    std::set<scn::scene_node_id>  m_nodes_inserted;
    std::map<scn::scene_node_id, angeo::coordinate_system>  m_nodes_erased;

    std::unordered_set<scn::scene_record_id>  m_batches_inserted;
    std::unordered_map<scn::scene_record_id, boost::filesystem::path>  m_batches_erased;

    std::unordered_set<scn::scene_record_id>  m_colliders_inserted;
    std::unordered_map<scn::scene_record_id, scn::collider_props>  m_colliders_erased;

    std::unordered_set<scn::scene_record_id>  m_rigid_bodies_inserted;
    std::unordered_set<scn::scene_record_id>  m_rigid_bodies_erased;


    void  clear(bool const  also_rigid_body_backups = false);

    void  insert_backup_for_rigid_body(angeo::rigid_body_id const  id, scn::rigid_body_props const&  backup, bool const  is_static);
    void  erase_backup_for_rigid_body(angeo::rigid_body_id const  id);
    bool  has_static_backup(angeo::rigid_body_id const  id) const { return m_static_rigid_body_backups.count(id) != 0UL; }

    void  on_node_relocation(scn::scene_node_id const  id, angeo::coordinate_system const&  orig_coord_system);

    void  on_rigid_body_props_changed(scn::scene_record_id const&  id);
};


#endif
