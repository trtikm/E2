#ifndef AI_SENSOR_HPP_INCLUDED
#   define AI_SENSOR_HPP_INCLUDED

#   include <ai/sensor_kind.hpp>
#   include <ai/object_id.hpp>
#   include <ai/scene.hpp>
#   include <ai/property_map.hpp>
#   include <unordered_map>
#   include <unordered_set>
#   include <vector>

namespace ai {


struct  simulator;


struct  sensor
{
    struct  collision_contact_record
    {
        collision_contact_record(
                scene::node_id const&  collider_nid_,
                scene::collicion_contant_info const&  contact_info_,
                object_id const&  other_id_,
                scene::node_id const&  other_collider_nid_
                );

        scene::node_id  collider_nid;
        scene::collicion_contant_info  contact_info;
        object_id  other_id;
        scene::node_id  other_collider_nid;
    };

    sensor(simulator* const  simulator_,
           SENSOR_KIND const  kind_,
           object_id const&  self_id_,
           scene::record_id const&  self_rid_,
           object_id const&  owner_id_,
           std::shared_ptr<property_map> const  cfg_,
           std::shared_ptr<std::vector<scene::node_id> > const  collider_nids_
           );
    ~sensor();

    SENSOR_KIND  get_kind() const { return m_kind; }
    scene::record_id const&  get_self_rid() const { return m_self_rid; }
    object_id const&  get_owner_id() const { return m_owner_id; }
    property_map const&  get_config() const { return *m_cfg; }
    std::vector<collision_contact_record> const&  get_collision_contacts() const { return m_collision_contacts_buffer; }
    std::unordered_set<object_id> const&  get_touch_begin_ids() const { return m_touch_begin_ids; }
    std::unordered_set<object_id> const&  get_touching_ids() const { return m_touching_ids; }
    std::unordered_set<object_id> const&  get_touch_end_ids() const { return m_touch_end_ids; }

    void  set_owner_id(object_id const&  id);

    void  next_round(float_32_bit const  time_step_in_seconds);

    void  on_collision_contact(
            scene::node_id const&  collider_nid,
            scene::collicion_contant_info const&  contact_info,
            object_id const&  other_id,
            scene::node_id const&  other_collider_nid
            );

private:
    simulator*  m_simulator;
    SENSOR_KIND  m_kind;
    object_id  m_self_id;
    scene::record_id  m_self_rid;
    object_id  m_owner_id;

    std::shared_ptr<property_map>  m_cfg;

    std::shared_ptr<std::vector<scene::node_id> >  m_collider_nids;
    std::vector<collision_contact_record>  m_collision_contacts_buffer;
    std::unordered_set<object_id>  m_touch_begin_ids;
    std::unordered_set<object_id>  m_touching_ids;
    std::unordered_set<object_id>  m_old_touching_ids;
    std::unordered_set<object_id>  m_touch_end_ids;
};


std::unordered_map<SENSOR_KIND, property_map::default_config_records_map> const&  default_sensor_configs();


}

#endif
