#ifndef AI_SENSOR_ACTION_DEFAULT_PROCESSOR_HPP_INCLUDED
#   define AI_SENSOR_ACTION_DEFAULT_PROCESSOR_HPP_INCLUDED

#   include <ai/sensor_action.hpp>
#   include <ai/sensor.hpp>
#   include <ai/simulator.hpp>
#   include <ai/scene.hpp>
#   include <angeo/tensor_math.hpp>

namespace ai {


bool  process_sensor_event_using_default_procedure(
        scene::record_id const&  self_rid,
        sensor_action&  action,
        sensor::other_object_info const&  other,
        simulator* const  simulator,
        scene_ptr const  scene
        );


}

#endif
