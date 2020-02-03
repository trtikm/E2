#ifndef E2_SCENE_RECORDS_DEVICE_DEVICE_HPP_INCLUDED
#   define E2_SCENE_RECORDS_DEVICE_DEVICE_HPP_INCLUDED

#   include <scene/records/skeleton_props.hpp>
#   include <ai/device_id.hpp>
#   include <ai/device_kind.hpp>
#   include <ai/sensor_action.hpp>
#   include <vector>

namespace scn {


struct  device_props
{
    ai::DEVICE_KIND  m_device_kind;
    skeleton_props_const_ptr  m_skeleton_props;
    ai::from_sensor_record_to_sensor_action_map  m_sensor_action_map;
};


struct  device  final
{
    device(ai::device_id const  id, device_props const&  props)
        : m_id(id)
        , m_props(props)
    {}

    ai::device_id  id() const { return m_id; }
    device_props const&  get_props() const { return m_props; }

private:
    ai::device_id  m_id;
    device_props  m_props;
};


}

#endif
