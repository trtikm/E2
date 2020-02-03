#ifndef AI_DEVICES_HPP_INCLUDED
#   define AI_DEVICES_HPP_INCLUDED

#   include <ai/device.hpp>
#   include <ai/device_id.hpp>
#   include <ai/sensor.hpp>
#   include <ai/sensor_id.hpp>
#   include <ai/scene.hpp>
#   include <ai/blackboard_device.hpp>
#   include <ai/skeletal_motion_templates.hpp>
#   include <ai/sensor_action.hpp>
#   include <memory>
#   include <vector>

namespace ai {


struct  simulator;


struct  devices
{
    explicit devices(simulator* const  simulator_, scene_ptr const  scene_);

    device_id  insert(
            scene::node_id const&  device_nid,
            skeletal_motion_templates const  motion_templates, // can be empty, when the device does not use skeletal animations.
            DEVICE_KIND const  device_kind,
            from_sensor_record_to_sensor_action_map const&  sensor_actions
            );
    void  erase(device_id const  id) { m_devices.at(id) = nullptr; }

    void  clear() { m_devices.clear(); }

    bool  ready(device_id const  id) { return m_devices.at(id)->device_ptr.operator bool(); }
    device&  at(device_id const  id) { return *m_devices.at(id)->device_ptr; }
    device const&  at(device_id const  id) const { return *m_devices.at(id)->device_ptr; }

    scene_ptr  get_scene_ptr() const { return m_scene; }

    void  next_round(float_32_bit const  time_step_in_seconds);

    void  on_collision_contact(
            device_id const  id,
            scene::node_id const&  collider_nid,
            scene::collicion_contant_info const&  contact_info,
            object_id const&  other_id,
            scene::node_id const&  other_collider_nid
            );

    void  on_sensor_event(device_id const  id, sensor const&  s);

private:

    struct  device_props
    {
        std::unique_ptr<device>  device_ptr;
        scene::node_id  device_nid;
        skeletal_motion_templates  motion_templates;
        DEVICE_KIND  device_kind;
        std::shared_ptr<from_sensor_record_to_sensor_action_map>  m_sensor_actions;
    };

    void  construct_device(device_id const  id, device_props&  props);

    std::vector<std::shared_ptr<device_props> >  m_devices; // Should be 'std::vector<std::unique_ptr<device_props> >', but does not compile :-(
    simulator*  m_simulator;
    scene_ptr  m_scene;
};


}

#endif
