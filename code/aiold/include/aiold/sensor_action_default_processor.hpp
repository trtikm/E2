#ifndef AIOLD_SENSOR_ACTION_DEFAULT_PROCESSOR_HPP_INCLUDED
#   define AIOLD_SENSOR_ACTION_DEFAULT_PROCESSOR_HPP_INCLUDED

#   include <aiold/sensor_action.hpp>
#   include <aiold/sensor.hpp>
#   include <aiold/simulator.hpp>
#   include <aiold/scene.hpp>
#   include <angeo/tensor_math.hpp>

namespace aiold {


bool  process_sensor_event_using_default_procedure(
        scene::record_id const&  self_rid,
        sensor_action&  action,
        sensor::other_object_info const&  other,
        simulator* const  simulator,
        scene_ptr const  scene
        );


}

#endif
