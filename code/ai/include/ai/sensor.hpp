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
        collision_contact_record(scene::collicion_contant_info_ptr const  contact_info_, object_id const&  other_id_);
        scene::collicion_contant_info_ptr  contact_info;
        object_id  other_id;
    };

    struct  other_object_info
    {
        other_object_info()
            : sensor(nullptr)
            , rigid_body_id()
            , contact_infos()
        {}
        other_object_info(
                sensor const* const  sensor_,
                scene::record_id const&  rigid_body_id_,
                scene::collicion_contant_info_ptr const  contact_info
                )
            : sensor(sensor_)
            , rigid_body_id(rigid_body_id_)
            , contact_infos{contact_info}
        {}
        sensor const*  sensor;
        scene::record_id  rigid_body_id;
        std::vector<scene::collicion_contant_info_ptr>  contact_infos;
    };

    using  touch_map = std::unordered_map<scene::record_id, other_object_info>;

    sensor(simulator* const  simulator_,
           SENSOR_KIND const  kind_,
           object_id const&  self_id_,
           scene::record_id const&  self_rid_,
           object_id const&  owner_id_,
           bool const  enabled_,
           std::shared_ptr<property_map> const  cfg_,
           std::shared_ptr<std::vector<scene::node_id> > const  collider_nids_
           );
    ~sensor();

    SENSOR_KIND  get_kind() const { return m_kind; }
    scene::record_id const&  get_self_rid() const { return m_self_rid; }
    object_id const&  get_owner_id() const { return m_owner_id; }
    bool  is_enabled() const { return m_enabled; }
    void  set_enabled(bool const  state) { m_enabled = state; m_collision_contacts_buffer.clear(); }
    property_map const&  get_config() const { return *m_cfg; }
    std::vector<collision_contact_record> const&  get_collision_contacts() const { return m_collision_contacts_buffer; }
    touch_map const&  get_touch_begin() const { return m_touch_begin; }
    touch_map const&  get_touching() const { return m_touching; }
    touch_map const&  get_touch_end() const { return m_touch_end; }

    void  set_owner_id(object_id const&  id);

    void  next_round(float_32_bit const  time_step_in_seconds);

    void  on_collision_contact(scene::collicion_contant_info_ptr const  contact_info, object_id const&  other_id);

private:
    simulator*  m_simulator;
    SENSOR_KIND  m_kind;
    object_id  m_self_id;
    scene::record_id  m_self_rid;
    object_id  m_owner_id;
    bool  m_enabled;

    std::shared_ptr<property_map>  m_cfg;

    std::shared_ptr<std::vector<scene::node_id> >  m_collider_nids;
    std::vector<collision_contact_record>  m_collision_contacts_buffer;
    touch_map  m_touch_begin;
    touch_map  m_touching;
    touch_map  m_old_touching;
    touch_map  m_touch_end;
};


std::unordered_map<SENSOR_KIND, property_map::default_config_records_map> const&  default_sensor_configs();


}

#endif
