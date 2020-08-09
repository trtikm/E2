#ifndef AI_SIGHT_CONTROLLER_HPP_INCLUDED
#   define AI_SIGHT_CONTROLLER_HPP_INCLUDED

#   include <ai/skeletal_motion_templates.hpp>
#   include <ai/scene_binding.hpp>
#   include <angeo/tensor_math.hpp>
#   include <gfx/camera.hpp>
#   include <com/object_guid.hpp>
#   include <utility/random.hpp>
#   include <unordered_map>
#   include <functional>
#   include <map>

namespace ai {


struct  sight_controller
{
    using  camera_perspective = gfx::camera_perspective;
    using  camera_perspective_ptr = gfx::camera_perspective_ptr;

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

    struct  ray_cast_config
    {
        natural_32_bit  num_raycasts_per_second;
        float_32_bit  max_ray_cast_info_life_time_in_seconds;
        natural_16_bit  num_cells_along_any_axis;   // Sight cells in "retina" form a regular 2D grid, which is also rectangular;
                                                    // i.e. there is the same number of cells along both axes. Each ray-cast is
                                                    // performed for some (randomly chosen) cell.
        std::function<float_32_bit(float_32_bit)>  distribution_of_cells_in_camera_space; // Although the cells form a regular 2D grid
                                                    // in the memory model, it still might be useful to define a spatial distribution
                                                    // of the cells in the camera's projection plane. Typically, the density of cells
                                                    // inreases as we approach the origin of the projection plane. So, the grid should
                                                    // be stretched towards the origin. The function here defines the stretching.
                                                    // The argument is always in range <0.0, 1.0> and the return value must also
                                                    // always be in range <0.0, 1.0>. Further, these invariants must hold:
                                                    //      distribution_of_cells_in_camera_space(0.0f) == 0.0f
                                                    //      distribution_of_cells_in_camera_space(1.0f) == 1.0f
                                                    // It is typically desired the function being monotonic (increasing).

        ray_cast_config(
                natural_32_bit const  num_raycasts_per_second_ = 600U,
                float_32_bit const  max_ray_cast_info_life_time_in_seconds_ = 0.1f,
                natural_16_bit const  num_cells_along_any_axis_ = 100U,
                std::function<float_32_bit(float_32_bit)> const&  distribution_of_cells_in_camera_space_ =
                    [](float_32_bit const  x) -> float_32_bit { return x * x; } // This is a quadratic stretch of the cell's grid.
                );
    };

    struct  ray_cast_info
    {
        natural_32_bit  cell_x;
        natural_32_bit  cell_y;
        vector2  camera_coords_of_cell_coords;
        vector3  ray_origin_in_world_space;
        vector3  ray_unit_direction_in_world_space;
        float_32_bit  parameter_to_coid;
        com::object_guid  collider_guid;

        ray_cast_info(
                natural_32_bit const  cell_x_,
                natural_32_bit const  cell_y_,
                vector2 const&  camera_coords_of_cell_coords_,
                vector3 const&  ray_origin_in_world_space_,
                vector3 const&  ray_unit_direction_in_world_space_,
                float_32_bit const  parameter_to_coid_,
                com::object_guid const  collider_guid_
                );
    };

    using  ray_casts_in_time = std::multimap<float_64_bit, ray_cast_info>;

    sight_controller(
            camera_config const&  camera_config_,
            ray_cast_config const& ray_cast_config_,
            skeletal_motion_templates const  motion_templates,
            scene_binding_ptr const  binding
            );

    void  next_round(float_32_bit const  time_step_in_seconds);

    camera_config const&  get_camera_config() const { return m_camera_config; }
    // NOTE: Camera's coord. system is in the world space.
    camera_perspective_ptr  get_camera() const { return m_camera; }

    ray_cast_config const&  get_ray_cast_config() const { return m_ray_cast_config; }
    ray_casts_in_time const&  get_ray_casts_in_time() const { return m_ray_casts_in_time; }

    skeletal_motion_templates  get_motion_templates() const { return m_motion_templates; }
    scene_binding_ptr  get_binding() const { return m_binding; }
    com::simulation_context const&  ctx() const { return *m_binding->context; }

private:
    void  update_camera(float_32_bit const  time_step_in_seconds);

    camera_config  m_camera_config;
    camera_perspective_ptr  m_camera;
    ray_cast_config  m_ray_cast_config;
    ray_casts_in_time  m_ray_casts_in_time;
    float_64_bit  m_current_time;
    random_generator_for_natural_32_bit  m_generator;
    float_32_bit  m_time_buffer;
    skeletal_motion_templates  m_motion_templates;
    scene_binding_ptr  m_binding;
};


}

#endif
