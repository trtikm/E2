#ifndef E2_SCENE_RECORDS_DEVICE_DEVICE_HPP_INCLUDED
#   define E2_SCENE_RECORDS_DEVICE_DEVICE_HPP_INCLUDED

#   include <scene/records/skeleton_props.hpp>
#   include <aiold/device_id.hpp>
#   include <aiold/device_kind.hpp>
#   include <aiold/sensor_action.hpp>
#   include <vector>

namespace scn {


struct  device_props
{
    aiold::DEVICE_KIND  m_device_kind;
    skeleton_props_const_ptr  m_skeleton_props;
    aiold::from_sensor_record_to_sensor_action_map  m_sensor_action_map;
};


struct  device  final
{
    device(aiold::device_id const  id, device_props const&  props)
        : m_id(id)
        , m_props(props)
    {}

    aiold::device_id  id() const { return m_id; }
    device_props const&  get_props() const { return m_props; }

private:
    aiold::device_id  m_id;
    device_props  m_props;
};


}

#endif
