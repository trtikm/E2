#ifndef AI_ENVIRONMENT_MODELS_HPP_INCLUDED
#   define AI_ENVIRONMENT_MODELS_HPP_INCLUDED

#   include <ai/agent_id.hpp>
#   include <angeo/collision_scene.hpp>
#   include <angeo/rigid_body_simulator.hpp>
#   include <memory>

namespace ai {


struct  environment_models
{
    using  collision_scene = angeo::collision_scene;
    using  collision_scene_ptr = std::shared_ptr<angeo::collision_scene>;

    using  rigid_body_simulator = angeo::rigid_body_simulator;
    using  rigid_body_simulator_ptr = std::shared_ptr<angeo::rigid_body_simulator>;
    
    using  scene_action_name = std::string;

    using  create_scene_object_callback = std::function<void(agent_id, scene_action_name const&)>;
    using  destroy_scene_object_callback = std::function<void(angeo::rigid_body_id)>;  

    environment_models(
            collision_scene_ptr const  collisions_,
            rigid_body_simulator_ptr const  physics_,
            create_scene_object_callback const&  create_scene_object_handler_,
            destroy_scene_object_callback const&  destroy_scene_object_handler_
            )
        : collisions(collisions_)
        , physics(physics_)
        , create_scene_object_handler(create_scene_object_handler_)
        , destroy_scene_object_handler(destroy_scene_object_handler_)
    {}

    collision_scene_ptr  collisions;
    rigid_body_simulator_ptr  physics;
    create_scene_object_callback   create_scene_object_handler;
    destroy_scene_object_callback  destroy_scene_object_handler;
};


using  environment_models_ptr = std::shared_ptr<environment_models>;
using  environment_models_const_ptr = std::shared_ptr<environment_models const>;


}

#endif
