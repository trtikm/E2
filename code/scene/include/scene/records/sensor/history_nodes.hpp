#ifndef E2_SCENE_RECORDS_SENSOR_HISTORY_NODES_HPP_INCLUDED
#   define E2_SCENE_RECORDS_SENSOR_HISTORY_NODES_HPP_INCLUDED

#   include <scene/scene_history_nodes_default.hpp>
#   include <scene/scene_record_id.hpp>
#   include <scene/records/sensor/sensor.hpp>

namespace scn {


struct  scene_history_sensor_insert final : public scene_history_record_insert<scene_history_sensor_insert>
{
    using super_type = scene_history_record_insert<scene_history_sensor_insert>;

    scene_history_sensor_insert(
            scene_record_id const&  id,
            scn::sensor_props const&  props,
            bool const  as_inverse_operation    // pass 'true', if the operation should represent 'erase'
            )
        : super_type(id, as_inverse_operation)
        , m_props(props)
    {}

    scn::sensor_props const&  get_props() const { return m_props; }

private:
    scn::sensor_props  m_props;
};


struct  scene_history_sensor_update_props final : public scene_history_record_update<scene_history_sensor_update_props>
{
    using super_type = scene_history_record_update<scene_history_sensor_update_props>;

    scene_history_sensor_update_props(
            scene_record_id const&  id,
            scn::sensor_props const&  old_props,
            scn::sensor_props const&  new_props,
            bool const  as_inverse_operation    // pass 'true', if the operation should represent 'return-to-old-props'
            )
        : super_type(id, as_inverse_operation)
        , m_old_props(old_props)
        , m_new_props(new_props)
    {}

    scn::sensor_props const&  get_old_props() const { return m_old_props; }
    scn::sensor_props const&  get_new_props() const { return m_new_props; }

private:
    scn::sensor_props  m_old_props;
    scn::sensor_props  m_new_props;
};


}

#endif
