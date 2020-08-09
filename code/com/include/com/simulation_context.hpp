#ifndef COM_SIMULATION_CONTEXT_HPP_INCLUDED
#   define COM_SIMULATION_CONTEXT_HPP_INCLUDED

#   include <com/object_guid.hpp>
#   include <com/frame_of_reference.hpp>
#   include <com/device_simulator.hpp>
#   include <com/import_scene_props.hpp>
#   include <com/collision_contact.hpp>
#   include <com/detail/import_scene.hpp>
#   include <gfx/batch.hpp>
#   include <gfx/batch_generators.hpp>
#   include <angeo/collision_object_id.hpp>
#   include <angeo/collision_shape_id.hpp>
#   include <angeo/collision_material.hpp>
#   include <angeo/collision_class.hpp>
#   include <angeo/collision_object_id.hpp>
#   include <angeo/rigid_body.hpp>
#   include <angeo/custom_constraint_id.hpp>
#   include <ai/agent_id.hpp>
#   include <ai/agent_kind.hpp>
#   include <ai/skeletal_motion_templates.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <utility/dynamic_array.hpp>
#   include <boost/filesystem/path.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <unordered_map>
#   include <vector>
#   include <list>
#   include <string>
#   include <functional>
#   include <algorithm>
#   include <memory>

namespace angeo {
    struct  collision_scene;
    struct  rigid_body_simulator;
}
namespace ai {
    struct  simulator;
}

namespace com {


struct  simulation_context;
using  simulation_context_ptr = std::shared_ptr<simulation_context>;
using  simulation_context_const_ptr = std::shared_ptr<simulation_context const>;


struct  simulation_context
{
    static simulation_context_ptr  create(
            std::shared_ptr<angeo::collision_scene> const  collision_scene_ptr_,
            std::shared_ptr<angeo::rigid_body_simulator> const  rigid_body_simulator_ptr_,
            std::shared_ptr<com::device_simulator> const  device_simulator_ptr_,
            std::shared_ptr<ai::simulator> const  ai_simulator_ptr_,
            std::string const&  data_root_dir_
            );
    ~simulation_context();

    template<OBJECT_KIND object_kind>
    struct  object_guid_iterator
    {
        using value_type      = object_guid;
        using difference_type = object_guid::index_type;
        using pointer         = value_type const*;  // Expected type for iterators, but not used here.
        using reference       = value_type const&;  // Expected type for iterators, but not used here.

        using container_type        = std::unordered_set<difference_type>;
        using indices_iterator_type = container_type::const_iterator;

        object_guid_iterator() noexcept : indices_it() {}
        object_guid_iterator(indices_iterator_type const it) noexcept : indices_it(it) {}

        value_type  operator*() const { return { object_kind, *indices_it }; }

        object_guid_iterator&  operator++() { ++indices_it; return *this; }
        object_guid_iterator  operator++(int) { object_guid_iterator  tmp = *this; ++indices_it; return tmp; }

        bool  operator==(object_guid_iterator const&  other) const { return indices_it == other.indices_it; }
        bool  operator!=(object_guid_iterator const&  other) const { return indices_it != other.indices_it; }

    private:

        indices_iterator_type  indices_it;
    };

    // INVARIANT:
    //      Iterators and objects obtained via refence or pointer from the API function below are guaranteed
    //      to be valid only in the current simulation round. Therefore, in each round you are supposed to call
    //      the API function again to obtain a valid iterator/refences/pointer for that round.

    // IMPORTANT NOTE:
    //      Non-const functions are NOT supposed to be accessible from modules managing objects of kinds 'OBJECT_KIND',
    //      like collison scene, physics, AI. If any of these modules needs an access to the context, then they should
    //      only be provided by a const reference to the context.

    // IMPORTANT NOTE:
    //      Each API function whose name starts with 'request_' only inserts the corresponding request to an internal 
    //      queue for later processing and then returns immediatelly. Requests from functions whose names start with
    //      'request_early_' should be applied right before physics simulator solves its constranit system for
    //      collision and joint resolving impulses. Requests from all other request functions should be applied
    //      in the end of the currently processed simulation round. In all cases, no module should observe an effect
    //      of any request sent in the current simulation rounds earlier than in the next simulation round (the only
    //      exception is the physics simulator which may observe the effect of the early requests, as explained
    //      above). Processing of requests from functions starting with 'request_late_', like request_late_insert_agent
    //      or request_late_import_scene_from_directory, may take several simulation rounds till the effect becomes
    //      observable.

    // INVARIANT:
    //      Function 'process_pending_early_requests()' applies requests from calls to functions whose names start
    //      with 'request_early_'. The requests are applied in exactly the same order as they were accepted.
    //      Function 'process_pending_late_requests()' applies requests from calls to functions whose names start
    //      with 'request_late_'. The requests are applied in a non-deterministic order. Finally, the function
    //      'process_pending_requests()' applies requests from calls to all other request functions. These
    //      requests are applied in exactly the same order as they were accepted.

    // IMPORTANT NOTE:
    //      No function in this module is thread safe. TODO: Change that!!!

    /////////////////////////////////////////////////////////////////////////////////////
    // FOLDER API
    /////////////////////////////////////////////////////////////////////////////////////

    struct  folder_content_type
    {
        using  names_to_guids_map = std::unordered_map<std::string, object_guid>;
        folder_content_type();
        folder_content_type(std::string const&  folder_name, object_guid const  parent_folder_guid);
        std::string  folder_name;
        object_guid  parent_folder;
        names_to_guids_map  child_folders;    // Only folder guids.
        names_to_guids_map  content;          // All other guids.
    };

    using  folder_visitor_type = std::function<bool(object_guid, folder_content_type const&)>;
    using  folder_guid_iterator = object_guid_iterator<OBJECT_KIND::FOLDER>;

    bool  is_valid_folder_guid(object_guid const  folder_guid) const;
    object_guid  root_folder() const;
    bool  is_folder_empty(object_guid const  folder_guid) const;
    folder_content_type const&  folder_content(object_guid const  folder_guid) const;
    object_guid  folder_of_folder(object_guid const  guid) const;
    object_guid  folder_of(object_guid const  folder_guid) const;
    std::string const&  name_of_folder(object_guid const  guid) const;
    std::string const&  name_of(object_guid const  guid) const;
    folder_guid_iterator  folders_begin() const;
    folder_guid_iterator  folders_end() const;
    void  for_each_parent_folder(object_guid  folder_guid, bool const  including_passed_folder,
                                 folder_visitor_type const&  visitor) const;
    void  for_each_child_folder(object_guid const  folder_guid, bool const  recursively, bool const  including_passed_folder,
                                folder_visitor_type const&  visitor) const;
    object_guid  insert_folder(object_guid const  under_folder_guid, std::string  folder_name,
                               bool const  resolve_name_collision = true) const;
    void  request_erase_non_root_empty_folder(object_guid const  folder_guid) const;
    void  request_erase_non_root_folder(object_guid const  folder_guid) const;
    // Disabled (not const) for modules.
    void  erase_non_root_empty_folder(object_guid const  folder_guid);

    /////////////////////////////////////////////////////////////////////////////////////
    // FRAMES API
    /////////////////////////////////////////////////////////////////////////////////////

    using  frame_guid_iterator = object_guid_iterator<OBJECT_KIND::FRAME>;

    bool  is_valid_frame_guid(object_guid const  frame_guid) const;
    object_guid  folder_of_frame(object_guid const  frame_guid) const;
    std::string const&  name_of_frame(object_guid const  frame_guid) const;
    object_guid  to_frame_guid(frame_id const  frid) const;
    frame_guid_iterator  frames_begin() const;
    frame_guid_iterator  frames_end() const;
    object_guid  find_closest_frame(object_guid const  folder_guid, bool const  including_passed_folder) const;
    object_guid  parent_frame(object_guid const  frame_guid) const;
    void  direct_children_frames(object_guid const  frame_guid, std::vector<object_guid>&  direct_children) const;
    angeo::coordinate_system const&  frame_coord_system(object_guid const  frame_guid) const;
    angeo::coordinate_system_explicit const&  frame_explicit_coord_system(object_guid const  frame_guid) const;
    angeo::coordinate_system const&  frame_coord_system_in_world_space(object_guid const  frame_guid) const;
    angeo::coordinate_system_explicit const&  frame_explicit_coord_system_in_world_space(object_guid const  frame_guid) const;
    matrix44 const&  frame_world_matrix(object_guid const  frame_guid) const;
    object_guid  insert_frame(object_guid const  under_folder_guid, object_guid const  parent_frame_guid,
                              vector3 const&  origin, quaternion const&  orientation) const;
    void  request_erase_frame(object_guid const  frame_guid) const;
    void  request_relocate_frame(object_guid const  frame_guid, vector3 const&  new_origin,
                                 quaternion const&  new_orientation) const;
    void  request_relocate_frame_relative_to_parent(object_guid const  frame_guid, vector3 const&  new_origin,
                                                    quaternion const&  new_orientation) const;
    void  request_set_parent_frame(object_guid const  frame_guid, object_guid const  parent_frame_guid) const;
    // Disabled (not const) for modules.
    void  set_parent_frame(object_guid const  frame_guid, object_guid const  parent_frame_guid);
    object_guid  insert_frame(object_guid const  under_folder_guid);
    void  erase_frame(object_guid const  frame_guid);
    void  frame_translate(object_guid const  frame_guid, vector3 const&  shift);
    void  frame_rotate(object_guid const  frame_guid, quaternion const&  rotation);
    void  frame_set_origin(object_guid const  frame_guid, vector3 const&  new_origin);
    void  frame_set_orientation(object_guid const  frame_guid, quaternion const&  new_orientation);
    void  frame_relocate(object_guid const  frame_guid, angeo::coordinate_system const& new_coord_system);
    void  frame_relocate(object_guid const  frame_guid, vector3 const&  new_origin, quaternion const&  new_orientation);
    void  frame_relocate_relative_to_parent(object_guid const  frame_guid, vector3 const&  new_origin,
                                            quaternion const&  new_orientation);
    void  frame_relocate_relative_to_parent(object_guid const  frame_guid, object_guid const  relocation_frame_guid);

    /////////////////////////////////////////////////////////////////////////////////////
    // BATCHES API
    /////////////////////////////////////////////////////////////////////////////////////

    using  batch_guid_iterator = object_guid_iterator<OBJECT_KIND::BATCH>;

    bool  is_valid_batch_guid(object_guid const  batch_guid) const;
    object_guid  folder_of_batch(object_guid const  batch_guid) const;
    std::string const&  name_of_batch(object_guid const  batch_guid) const;
    object_guid  to_batch_guid(gfx::batch const  batch) const;
    batch_guid_iterator  batches_begin() const;
    batch_guid_iterator  batches_end() const;
    std::vector<object_guid> const&  frames_of_batch(object_guid const  batch_guid) const;
    std::vector<matrix44> const&  matrices_to_pose_bones_of_batch(object_guid const  batch_guid) const;
    void  request_erase_batch(object_guid const  batch_guid) const;
    // Disabled (not const) for modules.
    std::string const&  from_batch_guid(object_guid const  batch_guid);
    gfx::batch  from_batch_guid_to_batch(object_guid const  batch_guid);
    object_guid  insert_batch(object_guid const  folder_guid, std::string const&  name, gfx::batch const  batch,
                              std::vector<object_guid> const&  frame_guids = {},
                              std::vector<matrix44> const&  matrices_to_pose_bones = {}
                              );
    void  erase_batch(object_guid const  batch_guid);
    object_guid  load_batch(
            object_guid const  folder_guid, std::string const&  name,
            std::string const&  relative_disk_path,
            std::string const&  skin_name = "default",
            std::vector<object_guid> const&  frame_guids = {}
            );
    object_guid  insert_batch_lines3d(
            object_guid const  folder_guid, std::string const&  name,
            std::vector<std::pair<vector3,vector3> > const&  lines,
            vector4 const&  common_colour
            );
    object_guid  insert_batch_wireframe_box(
            object_guid const  folder_guid, std::string const&  name,
            vector3 const&  half_sizes_along_axes,
            vector4 const&  colour
            );
    object_guid  insert_batch_solid_box(
            object_guid const  folder_guid, std::string const&  name,
            vector3 const&  half_sizes_along_axes,
            vector4 const&  colour
            );
    object_guid  insert_batch_wireframe_capsule(
            object_guid const  folder_guid, std::string const&  name,
            float_32_bit const  half_distance_between_end_points,
            float_32_bit const  thickness_from_central_line,
            natural_8_bit const  num_lines_per_quarter_of_circle,
            vector4 const&  colour
            );
    object_guid  insert_batch_solid_capsule(
            object_guid const  folder_guid, std::string const&  name,
            float_32_bit const  half_distance_between_end_points,
            float_32_bit const  thickness_from_central_line,
            natural_8_bit const  num_lines_per_quarter_of_circle,
            vector4 const&  colour
            );
    object_guid  insert_batch_wireframe_sphere(
            object_guid const  folder_guid, std::string const&  name,
            float_32_bit const  radius,
            natural_8_bit const  num_lines_per_quarter_of_circle,
            vector4 const&  colour
            );
    object_guid  insert_batch_solid_smooth_sphere(
            object_guid const  folder_guid, std::string const&  name,
            float_32_bit const  radius,
            natural_8_bit const  num_lines_per_quarter_of_circle,
            vector4 const&  colour
            );
    object_guid  insert_batch_solid_sphere(
            object_guid const  folder_guid, std::string const&  name,
            float_32_bit const  radius,
            natural_8_bit const  num_lines_per_quarter_of_circle,
            vector4 const&  colour
            );
    object_guid  insert_batch_triangle_mesh(
            object_guid const  folder_guid, std::string const&  name,
            std::vector< std::array<float_32_bit, 3> > const& vertices,
            std::vector< std::array<float_32_bit, 3> > const& normals,
            vector4 const&  colour
            );
    object_guid  insert_batch_wireframe_perspective_frustum(
            object_guid const  folder_guid, std::string const&  name,
            float_32_bit const  near_plane,
            float_32_bit const  far_plane,
            float_32_bit const  left_plane,
            float_32_bit const  right_plane,
            float_32_bit const  top_plane,
            float_32_bit const  bottom_plane,
            vector4 const&  colour,
            bool const  with_axis
            );

    /////////////////////////////////////////////////////////////////////////////////////
    // COLLIDERS API
    /////////////////////////////////////////////////////////////////////////////////////

    using  collider_guid_iterator = object_guid_iterator<OBJECT_KIND::COLLIDER>;

    bool  is_valid_collider_guid(object_guid const  collider_guid) const;
    object_guid  folder_of_collider(object_guid const  collider_guid) const;
    std::string const&  name_of_collider(object_guid const  collider_guid) const;
    object_guid  to_collider_guid(angeo::collision_object_id const  coid) const;
    collider_guid_iterator  colliders_begin() const;
    collider_guid_iterator  colliders_end() const;
    bool  is_collider_moveable(object_guid const  collider_guid) const;
    collider_guid_iterator  moveable_colliders_begin() const;
    collider_guid_iterator  moveable_colliders_end() const;
    object_guid  frame_of_collider(object_guid const  collider_guid) const;
    object_guid  owner_of_collider(object_guid const  collider_guid) const;
    object_guid  rigid_body_of_collider(object_guid const  collider_guid) const;
    angeo::COLLISION_MATERIAL_TYPE  collision_material_of(object_guid const  collider_guid) const;
    angeo::COLLISION_CLASS  collision_class_of(object_guid const  collider_guid) const;
    angeo::COLLISION_SHAPE_TYPE  collider_shape_type(object_guid const  collider_guid) const;
    vector3 const&  collider_box_half_sizes_along_axes(object_guid const  collider_guid) const;
    float_32_bit  collider_capsule_half_distance_between_end_points(object_guid const  collider_guid) const;
    float_32_bit  collider_capsule_thickness_from_central_line(object_guid const  collider_guid) const;
    float_32_bit  collider_sphere_radius(object_guid const  collider_guid) const;
    bool  is_collider_enabled(object_guid const  collider_guid) const;
    object_guid  insert_collider_box(
            object_guid const  under_folder_guid, std::string const&  name,
            vector3 const&  half_sizes_along_axes,
            angeo::COLLISION_MATERIAL_TYPE const  material,
            angeo::COLLISION_CLASS const  collision_class
            ) const;
    object_guid  insert_collider_capsule(
            object_guid const  under_folder_guid, std::string const&  name,
            float_32_bit const  half_distance_between_end_points,
            float_32_bit const  thickness_from_central_line,
            angeo::COLLISION_MATERIAL_TYPE const  material,
            angeo::COLLISION_CLASS const  collision_class
            ) const;
    object_guid  insert_collider_sphere(
            object_guid const  under_folder_guid, std::string const&  name,
            float_32_bit const  radius,
            angeo::COLLISION_MATERIAL_TYPE const  material,
            angeo::COLLISION_CLASS const  collision_class
            ) const;
    object_guid  insert_collider_triangle_mesh(
            object_guid const  under_folder_guid, std::string const&  name_prefix,
            natural_32_bit const  num_triangles,
            std::function<vector3(natural_32_bit, natural_8_bit)> const&  getter_of_end_points_in_model_space,
            angeo::COLLISION_MATERIAL_TYPE const  material,
            angeo::COLLISION_CLASS const  collision_class
            ) const;
    object_guid  ray_cast_to_nearest_collider(
            vector3 const&  ray_origin,
            vector3 const&  ray_unit_direction_vector,
            float_32_bit const  ray_length,
            bool const  search_static,
            bool const  search_dynamic,
            float_32_bit*  ray_parameter_to_nearest_collider = nullptr,
            std::unordered_set<object_guid> const* const  ignored_collider_guids = nullptr
            ) const;
    void  request_enable_collider(object_guid const  collider_guid, bool const  state) const;
    void  request_enable_colliding(object_guid const  collider_1, object_guid const  collider_2, const bool  state) const;
    void  request_erase_collider(object_guid const  collider_guid) const;
    // Disabled (not const) for modules.
    void  enable_collider(object_guid const  collider_guid, bool const  state);
    void  enable_colliding(object_guid const  collider_1, object_guid const  collider_2, const bool  state);
    std::vector<angeo::collision_object_id> const&  from_collider_guid(object_guid const  collider_guid);
    void  relocate_collider(object_guid const  collider_guid, matrix44 const&  world_matrix);
    object_guid  insert_collider(
            object_guid const  under_folder_guid, std::string const&  name,
            std::function<void(matrix44 const&, bool, std::vector<angeo::collision_object_id>&)> const&  coids_builder
            );
    void  erase_collider(object_guid const  collider_guid);

    /////////////////////////////////////////////////////////////////////////////////////
    // RIGID BODIES API
    /////////////////////////////////////////////////////////////////////////////////////

    using  rigid_body_guid_iterator = object_guid_iterator<OBJECT_KIND::RIGID_BODY>;

    bool  is_valid_rigid_body_guid(object_guid const  rigid_body_guid) const;
    object_guid  folder_of_rigid_body(object_guid const  rigid_body_guid) const;
    std::string const&  name_of_rigid_body(object_guid const  rigid_body_guid) const;
    object_guid  to_rigid_body_guid(angeo::rigid_body_id const  rbid) const;
    rigid_body_guid_iterator  rigid_bodies_begin() const;
    rigid_body_guid_iterator  rigid_bodies_end() const;
    bool  is_rigid_body_moveable(object_guid const  rigid_body_guid) const;
    rigid_body_guid_iterator  moveable_rigid_bodies_begin() const;
    rigid_body_guid_iterator  moveable_rigid_bodies_end() const;
    object_guid  frame_of_rigid_body(object_guid const  rigid_body_guid) const;
    float_32_bit  inverted_mass_of_rigid_body(object_guid const  rigid_body_guid) const;
    vector3 const&  mass_centre_of_rigid_body(object_guid const  rigid_body_guid) const;
    quaternion const&  orientation_of_rigid_body(object_guid const  rigid_body_guid) const;
    std::vector<object_guid> const&  colliders_of_rigid_body(object_guid const  rigid_body_guid) const;
    vector3 const&  linear_velocity_of_rigid_body(object_guid const  rigid_body_guid) const;
    vector3 const&  angular_velocity_of_rigid_body(object_guid const  rigid_body_guid) const;
    vector3 const&  linear_acceleration_of_rigid_body(object_guid const  rigid_body_guid) const;
    vector3 const&  angular_acceleration_of_rigid_body(object_guid const  rigid_body_guid) const;
    angeo::custom_constraint_id  acquire_fresh_custom_constraint_id_from_physics() const;
    void  release_acquired_custom_constraint_id_back_to_physics(angeo::custom_constraint_id const  ccid) const;
    vector3  compute_velocity_of_point_of_rigid_body(object_guid const  rigid_body_guid, vector3 const&  point_in_world_space) const;
    object_guid  insert_rigid_body(
            object_guid const  under_folder_guid,
            bool const  is_moveable,
            vector3 const&  linear_velocity = vector3_zero(),
            vector3 const&  angular_velocity = vector3_zero(),
            vector3 const&  linear_acceleration = vector3_zero(),
            vector3 const&  angular_acceleration = vector3_zero(),
            float_32_bit const  inverted_mass = 0.0f,
            matrix33 const&  inverted_inertia_tensor = matrix33_zero()
            ) const;
    void  request_erase_rigid_body(object_guid const  rigid_body_guid) const;
    void  request_set_rigid_body_linear_velocity(object_guid const  rigid_body_guid, vector3 const&  velocity) const;
    void  request_set_rigid_body_angular_velocity(object_guid const  rigid_body_guid,  vector3 const&  velocity) const;
    void  request_set_rigid_body_linear_acceleration_from_source(object_guid const  rigid_body_guid, object_guid const  source_guid,
                                                                 vector3 const&  acceleration) const;
    void  request_set_rigid_body_angular_acceleration_from_source(object_guid const  rigid_body_guid, object_guid const  source_guid,
                                                                  vector3 const&  acceleration) const;
    void  request_remove_rigid_body_linear_acceleration_from_source(object_guid const  rigid_body_guid, object_guid const  source_guid) const;
    void  request_remove_rigid_body_angular_acceleration_from_source(object_guid const  rigid_body_guid, object_guid const  source_guid) const;
    void  request_early_insertion_of_custom_constraint_to_physics(
            angeo::custom_constraint_id const  ccid,
            object_guid const  rigid_body_0, vector3 const&  linear_component_0, vector3 const&  angular_component_0,
            object_guid const  rigid_body_1, vector3 const&  linear_component_1, vector3 const&  angular_component_1,
            float_32_bit const  bias,
            float_32_bit const  variable_lower_bound, float_32_bit const  variable_upper_bound,
            float_32_bit const  initial_value_for_cache_miss
            ) const;
    void  request_early_insertion_of_instant_constraint_to_physics(
            object_guid const  rigid_body_0, vector3 const&  linear_component_0, vector3 const&  angular_component_0,
            object_guid const  rigid_body_1, vector3 const&  linear_component_1, vector3 const&  angular_component_1,
            float_32_bit const  bias,
            float_32_bit const  variable_lower_bound, float_32_bit const  variable_upper_bound,
            float_32_bit const  initial_value
            ) const;
    // Disabled (not const) for modules.
    angeo::rigid_body_id  from_rigid_body_guid(object_guid const  rigid_body_guid);
    void  erase_rigid_body(object_guid const  rigid_body_guid);
    void  set_rigid_body_mass_centre(object_guid const  rigid_body_guid, vector3 const&  position);
    void  set_rigid_body_orientation(object_guid const  rigid_body_guid, quaternion const&  orientation);
    void  set_rigid_body_inverted_mass(object_guid const  rigid_body_guid, float_32_bit const  inverted_mass);
    void  set_rigid_body_inverted_inertia_tensor(object_guid const  rigid_body_guid, matrix33 const&  inverted_inertia_tensor);
    void  set_rigid_body_linear_velocity(object_guid const  rigid_body_guid, vector3 const&  velocity);
    void  set_rigid_body_angular_velocity(object_guid const  rigid_body_guid, vector3 const&  velocity);
    void  set_rigid_body_linear_acceleration_from_source(object_guid const  rigid_body_guid, object_guid const  source_guid,
                                                         vector3 const&  accel);
    void  set_rigid_body_angular_acceleration_from_source(object_guid const  rigid_body_guid, object_guid const  source_guid,
                                                          vector3 const&  accel);
    void  remove_rigid_body_linear_acceleration_from_source(object_guid const  rigid_body_guid, object_guid const  source_guid);
    void  remove_rigid_body_angular_acceleration_from_source(object_guid const  rigid_body_guid, object_guid const  source_guid);
    void  insert_custom_constraint_to_physics(
            angeo::custom_constraint_id const  ccid,
            object_guid const  rigid_body_0, vector3 const&  linear_component_0, vector3 const&  angular_component_0,
            object_guid const  rigid_body_1, vector3 const&  linear_component_1, vector3 const&  angular_component_1,
            float_32_bit const  bias,
            float_32_bit const  variable_lower_bound, float_32_bit const  variable_upper_bound,
            float_32_bit const  initial_value_for_cache_miss
            );
    void  insert_instant_constraint_to_physics(
            object_guid const  rigid_body_0, vector3 const&  linear_component_0, vector3 const&  angular_component_0,
            object_guid const  rigid_body_1, vector3 const&  linear_component_1, vector3 const&  angular_component_1,
            float_32_bit const  bias,
            float_32_bit const  variable_lower_bound, float_32_bit const  variable_upper_bound,
            float_32_bit const  initial_value
            );

    /////////////////////////////////////////////////////////////////////////////////////
    // TIMERS API
    /////////////////////////////////////////////////////////////////////////////////////

    using  timer_guid_iterator = object_guid_iterator<OBJECT_KIND::TIMER>;

    bool  is_valid_timer_guid(object_guid const  timer_guid) const;
    bool  is_timer_enabled(object_guid const  timer_guid) const;
    object_guid  folder_of_timer(object_guid const  timer_guid) const;
    std::string const&  name_of_timer(object_guid const  timer_guid) const;
    object_guid  to_timer_guid(com::device_simulator::timer_id const  tid) const;
    timer_guid_iterator  timers_begin() const;
    timer_guid_iterator  timers_end() const;
    void  request_erase_timer(object_guid const  timer_guid) const;
    // Disabled (not const) for modules.
    object_guid  insert_timer(object_guid const  under_folder_guid, std::string const&  name, float_32_bit const  period_in_seconds_,
                              natural_8_bit const target_enable_level_ = 1, natural_8_bit const  current_enable_level_ = 0);
    void  erase_timer(object_guid const  timer_guid);

    /////////////////////////////////////////////////////////////////////////////////////
    // SENSORS API
    /////////////////////////////////////////////////////////////////////////////////////

    using  sensor_guid_iterator = object_guid_iterator<OBJECT_KIND::SENSOR>;

    bool  is_valid_sensor_guid(object_guid const  sensor_guid) const;
    bool  is_sensor_enabled(object_guid const  sensor_guid) const;
    object_guid  folder_of_sensor(object_guid const  sensor_guid) const;
    std::string const&  name_of_sensor(object_guid const  sensor_guid) const;
    object_guid  to_sensor_guid(com::device_simulator::sensor_id const  sid) const;
    sensor_guid_iterator  sensors_begin() const;
    sensor_guid_iterator  sensors_end() const;
    void  request_erase_sensor(object_guid const  sensor_guid) const;
    // Disabled (not const) for modules.
    object_guid  insert_sensor(object_guid const  under_folder_guid, std::string const&  name, object_guid const  collider_,
                               std::unordered_set<object_guid> const&  triggers_ = {},
                               natural_8_bit const target_enable_level_ = 1, natural_8_bit const  current_enable_level_ = 0);
    void  erase_sensor(object_guid const  sensor_guid);
    void  insert_trigger_collider_to_sensor(object_guid const  sensor_guid, object_guid const  collider_guid);
    void  erase_trigger_collider_to_sensor(object_guid const  sensor_guid, object_guid const  collider_guid);

    /////////////////////////////////////////////////////////////////////////////////////
    // TIMER & SENSOR REQUEST INFOS API
    /////////////////////////////////////////////////////////////////////////////////////

    enum struct DEVICE_EVENT_TYPE : natural_8_bit
    {
        // Sensor events
        TOUCHING        = 0U,
        TOUCH_BEGIN     = 1U,
        TOUCH_END       = 2U,
        // Timer event
        TIME_OUT        = 3U,
    };

    struct  device_request_info_id
    {
        object_guid const  owner_guid;  // Either timer or sensor guid.
        DEVICE_EVENT_TYPE  event_type;  // TIME_OUT for timer, any other for sensor.
    };

    // Disabled (not const) for modules.
    void  register_request_info(device_request_info_id const&  drid, com::device_simulator::request_info_id  rid);
    void  insert_request_info_increment_enable_level_of_timer(device_request_info_id const&  drid, object_guid const  timer_guid);
    void  insert_request_info_decrement_enable_level_of_timer(device_request_info_id const&  drid, object_guid const  timer_guid);
    void  insert_request_info_reset_timer(device_request_info_id const&  drid, object_guid const  timer_guid);
    void  insert_request_info_increment_enable_level_of_sensor(device_request_info_id const&  drid, object_guid const  sensor_guid);
    void  insert_request_info_decrement_enable_level_of_sensor(device_request_info_id const&  drid, object_guid const  sensor_guid);
    void  insert_request_info_import_scene(device_request_info_id const&  drid, import_scene_props const&  props);
    void  insert_request_info_erase_folder(device_request_info_id const&  drid, object_guid const  folder_guid);
    void  insert_request_info_rigid_body_set_linear_velocity(device_request_info_id const&  drid, object_guid const  rb_guid,
                                                             vector3 const&  linear_velocity);
    void  insert_request_info_rigid_body_set_angular_velocity(device_request_info_id const&  drid, object_guid const  rb_guid,
                                                              vector3 const&  angular_velocity);
    void  insert_request_info_update_radial_force_field(
            device_request_info_id const&  drid,
            float_32_bit const  multiplier = 1.0f,
            float_32_bit const  exponent = 1.0f,
            float_32_bit const  min_radius = 0.001f,
            bool const  use_mass = true
            );
    void  insert_request_info_update_linear_force_field(
            device_request_info_id const&  drid,
            vector3 const&  acceleration = vector3(0.0f, 0.0f, -9.81f),
            bool const  use_mass = true
            );
    void  insert_request_info_leave_force_field(device_request_info_id const&  drid);

    /////////////////////////////////////////////////////////////////////////////////////
    // AGENTS API
    /////////////////////////////////////////////////////////////////////////////////////

    using  agent_guid_iterator = object_guid_iterator<OBJECT_KIND::AGENT>;

    bool  is_valid_agent_guid(object_guid const  agent_guid) const;
    object_guid  folder_of_agent(object_guid const  agent_guid) const;
    std::string const&  name_of_agent(object_guid const  agent_guid) const;
    object_guid  to_agent_guid(ai::agent_id const  agid) const;
    agent_guid_iterator  agents_begin() const;
    agent_guid_iterator  agents_end() const;
    void  request_late_insert_agent(
            object_guid const  under_folder_guid,
            ai::AGENT_KIND const  kind,
            vector3 const&  skeleton_frame_origin,
            quaternion const&  skeleton_frame_orientation,
            gfx::batch const  skeleton_attached_batch
            ) const;
    void  request_erase_agent(object_guid const  agent_guid) const;
    // Disabled (not const) for modules.
    ai::agent_id  from_agent_guid(object_guid const  agent_guid);
    object_guid  insert_agent(
            object_guid const  under_folder_guid,
            ai::AGENT_KIND const  kind,
            ai::skeletal_motion_templates const  motion_templates,
            vector3 const&  skeleton_frame_origin,
            quaternion const&  skeleton_frame_orientation,
            gfx::batch const  skeleton_attached_batch
            );
    void  erase_agent(object_guid const  agent_guid);

    /////////////////////////////////////////////////////////////////////////////////////
    // COLLISION CONTACTS API
    /////////////////////////////////////////////////////////////////////////////////////

    using  collision_contacts_iterator = dynamic_array<collision_contact, natural_32_bit>::const_iterator;

    bool  is_valid_collision_contact_index(natural_32_bit const  contact_index) const;
    std::vector<natural_32_bit> const&  collision_contacts_of_collider(object_guid const  collider_guid) const;
    collision_contact const&  get_collision_contact(natural_32_bit const  contact_index) const;
    natural_32_bit  num_collision_contacts() const;
    collision_contacts_iterator  collision_contacts_begin() const;
    collision_contacts_iterator  collision_contacts_end() const;
    // Disabled (not const) for modules.
    natural_32_bit  insert_collision_contact(collision_contact const&  cc);
    void  clear_collision_contacts();

    /////////////////////////////////////////////////////////////////////////////////////
    // ACCESS PATH API
    /////////////////////////////////////////////////////////////////////////////////////

    bool  is_absolute_path(std::string const&  path) const;
    bool  is_path_to_folder(std::string const&  path) const;
    object_guid  from_absolute_path(std::string const&  path) const;
    std::string  to_absolute_path(object_guid const  guid) const;
    object_guid  from_relative_path(object_guid const  base_guid, std::string const&  relative_path) const;
    std::string  to_relative_path(object_guid const  guid, object_guid const  relative_base_guid) const;

    /////////////////////////////////////////////////////////////////////////////////////
    // SCENE IMPORT/EXPORT API
    /////////////////////////////////////////////////////////////////////////////////////

    std::string const&  get_data_root_dir() const;
    std::string  get_animation_root_dir() const;
    std::string  get_batch_root_dir() const;
    std::string  get_font_root_dir() const;
    std::string  get_icon_root_dir() const;
    std::string  get_import_root_dir() const;
    std::string  get_mesh_root_dir() const;
    std::string  get_scene_root_dir() const;
    std::string  get_texture_root_dir() const;
    void  request_late_import_scene_from_directory(import_scene_props const&  props) const;
    // Disabled (not const) for modules.
    void  insert_imported_batch_to_cache(gfx::batch const  batch);

    /////////////////////////////////////////////////////////////////////////////////////
    // REQUESTS PROCESSING API
    /////////////////////////////////////////////////////////////////////////////////////

    // Disabled (not const) for modules.
    void  process_rigid_bodies_with_invalidated_shape();
    void  clear_rigid_bodies_with_invalidated_shape();

    void  process_pending_early_requests();
    void  clear_pending_early_requests();

    void  process_pending_requests();
    void  clear_pending_requests();

    void  process_pending_late_requests();
    void  clear_pending_late_requests();

    /////////////////////////////////////////////////////////////////////////////////////
    // SCENE CLEAR API
    /////////////////////////////////////////////////////////////////////////////////////

    void  clear(bool const  also_caches = false);

private:

    using  index_type = object_guid::index_type;

    template<typename T>
    struct  folder_element
    {
        using  module_specific_id = T;
        using  base_type = folder_element<module_specific_id>;
        folder_element() : id(), folder_index(0), element_name() {}
        folder_element(module_specific_id const&  id_, index_type const  folder_index_, std::string const&  element_name_)
            : id(id_)
            , folder_index(folder_index_)
            , element_name(element_name_)
        {}
        ~folder_element() {}
        module_specific_id  id;
        index_type  folder_index;
        std::string  element_name;
    };

    using  folder_element_frame = folder_element<frame_id>;

    struct  folder_element_batch : public folder_element<std::string>
    {
        folder_element_batch()
            : base_type()
            , batch()
            , frames()
            , matrices_to_pose_bones()
        {}
        folder_element_batch(module_specific_id const&  id_, index_type const  folder_index_, std::string const&  element_name_,
                gfx::batch const  batch_
                )
            : base_type(id_, folder_index_, element_name_)
            , batch(batch_)
            , frames()
            , matrices_to_pose_bones()
        {}
        gfx::batch  batch;
        std::vector<object_guid>  frames;
        std::vector<matrix44>  matrices_to_pose_bones;
    };

    struct  folder_element_collider : public folder_element<std::vector<angeo::collision_object_id> >
    {
        folder_element_collider()
            : base_type()
            , frame(invalid_object_guid())
            , owner(invalid_object_guid())
        {}
        folder_element_collider(module_specific_id const&  id_, index_type const  folder_index_, std::string const&  name,
                object_guid const  frame_, object_guid const  owner_, object_guid const  rigid_body_
                )
            : base_type(id_, folder_index_, name)
            , frame(frame_)
            , owner(owner_)
            , rigid_body(rigid_body_)
        {}
        object_guid  frame;
        object_guid  owner; // Can be either a rigid body, a device, or an agent.
        object_guid  rigid_body; // Can also be equal to the 'invalid_object_guid()'.
    };

    struct  folder_element_rigid_body : public folder_element<angeo::rigid_body_id>
    {
        folder_element_rigid_body()
            : base_type()
            , frame(invalid_object_guid())
            , colliders()
        {}
        folder_element_rigid_body(module_specific_id const  id_, index_type const  folder_index_,
                object_guid const  frame_
                )
            : base_type(id_, folder_index_, to_string(OBJECT_KIND::RIGID_BODY))
            , frame(frame_)
            , colliders()
        {}
        object_guid  frame;
        std::vector<object_guid>  colliders;
    };

    using  folder_element_timer = folder_element<com::device_simulator::timer_id>;

    struct  folder_element_sensor : public folder_element<com::device_simulator::sensor_id>
    {
        folder_element_sensor()
            : base_type()
            , collider(invalid_object_guid())
        {}
        folder_element_sensor(module_specific_id const  id_, index_type const  folder_index_, std::string const&  name_,
                object_guid const  collider_
                )
            : base_type(id_, folder_index_, name_)
            , collider(collider_)
        {}
        object_guid  collider;
    };

    using  folder_element_agent = folder_element<ai::agent_id>;

    object_guid  m_root_folder;
    dynamic_array<folder_content_type, index_type>  m_folders;
    dynamic_array<folder_element_frame, index_type>  m_frames;
    dynamic_array<folder_element_batch, index_type>  m_batches;
    dynamic_array<folder_element_collider, index_type>  m_colliders;
    dynamic_array<folder_element_rigid_body, index_type>  m_rigid_bodies;
    dynamic_array<folder_element_timer, index_type>  m_timers;
    dynamic_array<folder_element_sensor, index_type>  m_sensors;
    dynamic_array<folder_element_agent, index_type>  m_agents;

    frames_provider  m_frames_provider;
    std::shared_ptr<angeo::collision_scene>  m_collision_scene_ptr;
    std::shared_ptr<angeo::rigid_body_simulator>  m_rigid_body_simulator_ptr;
    std::shared_ptr<com::device_simulator>  m_device_simulator_ptr;
    std::shared_ptr<ai::simulator>  m_ai_simulator_ptr;

    std::unordered_map<frame_id, object_guid>  m_frids_to_guids;
    std::unordered_map<std::string, object_guid>  m_batches_to_guids;
    std::unordered_map<angeo::collision_object_id, object_guid>  m_coids_to_guids;
    std::unordered_map<angeo::rigid_body_id, object_guid>  m_rbids_to_guids;
    std::unordered_map<com::device_simulator::timer_id, object_guid>  m_tmids_to_guids;
    std::unordered_map<com::device_simulator::sensor_id, object_guid>  m_seids_to_guids;
    std::unordered_map<ai::agent_id, object_guid>  m_agids_to_guids;

    std::unordered_set<index_type>  m_moveable_colliders;
    std::unordered_set<index_type>  m_moveable_rigid_bodies;

    dynamic_array<collision_contact, natural_32_bit>  m_collision_contacts;
    std::unordered_map<object_guid, std::vector<natural_32_bit> >  m_from_colliders_to_contacts;

    std::string  m_data_root_dir;

    /////////////////////////////////////////////////////////////////////////////////////
    // CACHES
    /////////////////////////////////////////////////////////////////////////////////////

    std::unordered_map<async::key_type, detail::imported_scene>  m_cache_of_imported_scenes;
    std::unordered_map<async::key_type, gfx::batch>  m_cache_of_imported_batches;
    std::unordered_map<async::key_type, ai::skeletal_motion_templates>  m_cache_of_imported_motion_templates;

    /////////////////////////////////////////////////////////////////////////////////////
    // EARLY REQUESTS HANDLING
    /////////////////////////////////////////////////////////////////////////////////////

    std::unordered_set<object_guid>  m_rigid_bodies_with_invalidated_shape;

    enum REQUEST_EARLY_KIND
    {
        REQUEST_INSERT_CUSTOM_CONSTRAINT     = 0,
        REQUEST_INSERT_INSTANT_CONSTRAINT    = 1,
    };

    struct request_data_insertion_of_custom_constraint {
        angeo::custom_constraint_id  ccid;
        object_guid  rigid_body_0; vector3  linear_component_0; vector3  angular_component_0;
        object_guid  rigid_body_1; vector3  linear_component_1; vector3  angular_component_1;
        float_32_bit  bias;
        float_32_bit  variable_lower_bound; float_32_bit  variable_upper_bound;
        float_32_bit  initial_value_for_cache_miss;
    };

    struct request_data_insertion_of_instant_constraint {
        object_guid  rigid_body_0; vector3  linear_component_0; vector3  angular_component_0;
        object_guid  rigid_body_1; vector3  linear_component_1; vector3  angular_component_1;
        float_32_bit  bias;
        float_32_bit  variable_lower_bound; float_32_bit  variable_upper_bound;
        float_32_bit  initial_value;
    };

    mutable std::list<REQUEST_EARLY_KIND>  m_pending_requests_early;
    mutable std::list<request_data_insertion_of_custom_constraint> m_requests_early_insert_custom_constraint;
    mutable std::list<request_data_insertion_of_instant_constraint> m_requests_early_insert_instant_constraint;

    /////////////////////////////////////////////////////////////////////////////////////
    // REQUESTS HANDLING
    /////////////////////////////////////////////////////////////////////////////////////

    enum REQUEST_KIND
    {
        REQUEST_ERASE_FOLDER                        = 0,
        REQUEST_ERASE_FRAME                         = 1,
        REQUEST_RELOCATE_FRAME                      = 2,
        REQUEST_RELOCATE_FRAME_RELATIVE_TO_PARENT   = 3,
        REQUEST_SET_PARENT_FRAME                    = 4,
        REQUEST_ERASE_BATCH                         = 5,
        REQUEST_ENABLE_COLLIDER                     = 6,
        REQUEST_ENABLE_COLLIDING                    = 7,
        REQUEST_ERASE_COLLIDER                      = 8,
        REQUEST_ERASE_RIGID_BODY                    = 9,
        REQUEST_SET_LINEAR_VELOCITY                 = 10,
        REQUEST_SET_ANGULAR_VELOCITY                = 11,
        REQUEST_SET_LINEAR_ACCEL                    = 12,
        REQUEST_SET_ANGULAR_ACCEL                   = 13,
        REQUEST_DEL_LINEAR_ACCEL                    = 14,
        REQUEST_DEL_ANGULAR_ACCEL                   = 15,
        REQUEST_ERASE_TIMER                         = 16,
        REQUEST_ERASE_SENSOR                        = 17,
        REQUEST_ERASE_AGENT                         = 18,
    };

    struct  request_data_relocate_frame { object_guid  frame_guid; vector3  position; quaternion  orientation; };
    struct  request_data_set_parent_frame { object_guid  frame_guid; object_guid  parent_frame_guid; };
    struct  request_data_enable_collider { object_guid  collider_guid; bool  state; };
    struct  request_data_enable_colliding { object_guid  collider_1; object_guid  collider_2; bool  state; };
    struct  request_data_set_velocity { object_guid  rb_guid; vector3  velocity; };
    struct  request_data_set_acceleration_from_source { object_guid  rb_guid; object_guid  source_guid; vector3  acceleration; };
    struct  request_data_del_acceleration_from_source { object_guid  rb_guid; object_guid  source_guid; };

    mutable std::list<REQUEST_KIND>  m_pending_requests;
    mutable std::list<object_guid>  m_requests_erase_folder;
    mutable std::list<object_guid>  m_requests_erase_frame;
    mutable std::list<request_data_relocate_frame>  m_requests_relocate_frame;
    mutable std::list<request_data_relocate_frame>  m_requests_relocate_frame_relative_to_parent;
    mutable std::list<request_data_set_parent_frame>  m_requests_set_parent_frame;
    mutable std::list<object_guid>  m_requests_erase_batch;
    mutable std::list<request_data_enable_collider>  m_requests_enable_collider;
    mutable std::list<request_data_enable_colliding>  m_requests_enable_colliding;
    mutable std::list<object_guid>  m_requests_erase_collider;
    mutable std::list<object_guid>  m_requests_erase_rigid_body;
    mutable std::list<request_data_set_velocity>  m_requests_set_linear_velocity;
    mutable std::list<request_data_set_velocity>  m_requests_set_angular_velocity;
    mutable std::list<request_data_set_acceleration_from_source>  m_requests_set_linear_acceleration_from_source;
    mutable std::list<request_data_set_acceleration_from_source>  m_requests_set_angular_acceleration_from_source;
    mutable std::list<request_data_del_acceleration_from_source>  m_requests_del_linear_acceleration_from_source;
    mutable std::list<request_data_del_acceleration_from_source>  m_requests_del_angular_acceleration_from_source;
    mutable std::list<object_guid>  m_requests_erase_timer;
    mutable std::list<object_guid>  m_requests_erase_sensor;
    mutable std::list<object_guid>  m_requests_erase_agent;

    /////////////////////////////////////////////////////////////////////////////////////
    // LATE REQUESTS HANDLING
    /////////////////////////////////////////////////////////////////////////////////////

    template<typename T>
    struct  requests_late_queue
    {
        using  data_type = T;
        using  is_ready_func = std::function<bool(data_type&)>;
        using  process_data_func = std::function<void(data_type&)>;

        void  push_back(data_type const&  value) { data.push_back(value); }
        bool  empty() const { return data.empty(); }
        void  clear() { data.clear(); }
        void  update(is_ready_func const&  is_ready, process_data_func const&  process_data)
        {
            for (natural_32_bit  i = 0U; i < (natural_32_bit)data.size(); )
                if (is_ready(data.at(i)))
                {
                    std::swap(data.at(i), data.back());
                    data_type  value = data.back();
                    data.pop_back();
                    process_data(value);
                }
                else
                    ++i;
        }
    private:
        std::vector<data_type>  data;
    };

    struct  request_data_imported_scene
    {
        detail::imported_scene  scene;
        import_scene_props  props;
    };

    struct  request_data_insert_agent
    {
        object_guid  under_folder_guid;
        ai::AGENT_KIND  kind;
        vector3  skeleton_frame_origin;
        quaternion  skeleton_frame_orientation;
        gfx::batch  skeleton_attached_batch;
        ai::skeletal_motion_templates  motion_templates;
    };

    mutable requests_late_queue<request_data_imported_scene>  m_requests_late_scene_import;
    mutable requests_late_queue<request_data_insert_agent>  m_requests_late_insert_agent;

    /////////////////////////////////////////////////////////////////////////////////////
    // IMPLEMENTATION DETAILS
    /////////////////////////////////////////////////////////////////////////////////////

    simulation_context(
            std::shared_ptr<angeo::collision_scene> const  collision_scene_ptr_,
            std::shared_ptr<angeo::rigid_body_simulator> const  rigid_body_simulator_ptr_,
            std::shared_ptr<com::device_simulator> const  device_simulator_ptr_,
            std::shared_ptr<ai::simulator> const  ai_simulator_ptr_,
            std::string const&  data_root_dir_
            );

    simulation_context(simulation_context const&) = delete;
    simulation_context(simulation_context const&&) = delete;
    simulation_context& operator=(simulation_context const&) = delete;
    simulation_context& operator=(simulation_context const&&) = delete;
};


}

#endif
