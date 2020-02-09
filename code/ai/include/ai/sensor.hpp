#ifndef AI_SENSOR_HPP_INCLUDED
#   define AI_SENSOR_HPP_INCLUDED

#   include <ai/sensor_kind.hpp>
#   include <ai/object_id.hpp>
#   include <ai/scene.hpp>
#   include <ai/property_map.hpp>
#   include <unordered_map>

namespace ai {


struct  simulator;


struct  sensor
{
    sensor(simulator* const  simulator_,
           SENSOR_KIND const  kind_,
           scene::record_id const&  self_rid_,
           object_id const&  owner_id_,
           std::shared_ptr<property_map> const  cfg_
           );

    SENSOR_KIND  get_kind() const { return m_kind; }
    scene::record_id const&  get_self_rid() const { return m_self_rid; }
    object_id const&  get_owner_id() const { return m_owner_id; }
    property_map const&  get_config() const { return *m_cfg; }

    void  next_round(float_32_bit const  time_step_in_seconds);

private:
    simulator*  m_simulator;
    SENSOR_KIND  m_kind;
    scene::record_id  m_self_rid;
    object_id  m_owner_id;
    std::shared_ptr<property_map>  m_cfg;
};


std::unordered_map<SENSOR_KIND, property_map::default_config_records_map> const&  default_sensor_configs();


}

#endif
