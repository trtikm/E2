#ifndef AI_SENSORY_CONTROLLER_SIGHT_HPP_INCLUDED
#   define AI_SENSORY_CONTROLLER_SIGHT_HPP_INCLUDED

#   include <ai/blackboard.hpp>
#   include <angeo/tensor_math.hpp>
#   include <qtgl/camera.hpp>
#   include <memory>

namespace ai {


struct  sensory_controller_sight
{
    using  camera_perspective = qtgl::camera_perspective;
    using  camera_perspective_ptr = qtgl::camera_perspective_ptr;

    struct  camera_config
    {
        float_32_bit  fov_angle;
        float_32_bit  near_plane;
        float_32_bit  far_plane;
        float_32_bit  origin_z_shift;

        camera_config(
                float_32_bit const  fov_angle_ = PI() / 2.0f,
                float_32_bit const  near_plane_ = 0.05f,
                float_32_bit const  far_plane_ = 50.0f,
                float_32_bit const  origin_z_shift_ = 0.0f
                );
    };

    sensory_controller_sight(
            blackboard_weak_ptr const  blackboard_,
            camera_config const&  camera_config_
            );

    virtual ~sensory_controller_sight() {}

    virtual void  next_round(float_32_bit const  time_step_in_seconds);

    blackboard_ptr  get_blackboard() const { return m_blackboard.lock(); }

    camera_config const&  get_camera_config() const { return m_camera_config; }
    // NOTE: Camera's coord. system is in the world space, i.e. NOT in agent's local space!
    camera_perspective_ptr  get_camera() const { return m_camera; }

private:
    blackboard_weak_ptr  m_blackboard;
    camera_config  m_camera_config;
    camera_perspective_ptr  m_camera;
};


using  sensory_controller_sight_ptr = std::shared_ptr<sensory_controller_sight>;


}

#endif
