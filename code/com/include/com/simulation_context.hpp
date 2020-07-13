#ifndef COM_SIMULATION_CONTEXT_HPP_INCLUDED
#   define COM_SIMULATION_CONTEXT_HPP_INCLUDED

#   include <com/object_guid.hpp>
#   include <com/frame_of_reference.hpp>
#   include <gfx/batch.hpp>
#   include <gfx/batch_generators.hpp>
#   include <angeo/collision_scene.hpp>
#   include <angeo/rigid_body_simulator.hpp>
//#   include <ai/simulator.hpp>
#   include <ai/object_id.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <utility/dynamic_array.hpp>
#   include <boost/filesystem/path.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <unordered_map>
#   include <vector>
#   include <string>
#   include <functional>
#   include <memory>

namespace ai { struct simulator; }

namespace com {


struct simulation_context;
using  simulation_context_ptr = std::shared_ptr<simulation_context>;
using  simulation_context_const_ptr = std::shared_ptr<simulation_context const>;


enum struct BATCH_CLASS : natural_8_bit
{
    COMMON_OBJECT,
    COLLIDER_OF_RIGID_BODY,
    COLLIDER_OF_SENSOR,
    COLLIDER_OF_ACTIVATOR,
    COLLIDER_OF_AGENT,
    COLLISION_CONTACT,
    RAY_CAST,
    HELPER,
};


struct simulation_context
{
    static simulation_context_ptr  create(
            std::shared_ptr<angeo::collision_scene> const  collision_scene_ptr_,
            std::shared_ptr<angeo::rigid_body_simulator> const  rigid_body_simulator_ptr_,
            std::shared_ptr<ai::simulator> const  ai_simulator_ptr_
            );

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
    //      Non-const methods are NOT supposed to be accessible from modules managing objects of kinds 'OBJECT_KIND',
    //      like collison scene, physics, AI. If any of these modules needs an access to the context, then they should
    //      only be provided by a const reference to the context.

    // IMPORTANT NOTE:
    //      Each method whose name starts with 'request_' only inserts the corresponding request to an internal 
    //      queue for later processing and then returns immediatelly. An effect of most requests can be seen
    //      in the next simulation round. But for some requests, like request_import_scene_from_directory, there
    //      may pass several simulation rounds till the effect is actually visible.

    /////////////////////////////////////////////////////////////////////////////////////
    // REQUESTS PROCESSING API
    /////////////////////////////////////////////////////////////////////////////////////

    // Disabled (not const) for modules.
    void  process_pending_requests();
    void  process_pending_requests_import_scene();

    /////////////////////////////////////////////////////////////////////////////////////
    // SCENE IMPORT/EXPORT API
    /////////////////////////////////////////////////////////////////////////////////////

    void  request_import_scene_from_directory(std::string const&  directory_on_the_disk, object_guid const  under_folder_guid,
                                              bool const  cache_imported_scene) const;
    // Disabled (not const) for modules.
    void  import_scene(boost::property_tree::ptree const&  ptree, object_guid const  under_folder_guid);

    /////////////////////////////////////////////////////////////////////////////////////
    // ACCESS PATH API
    /////////////////////////////////////////////////////////////////////////////////////

    object_guid  from_absolute_path(std::string const&  path) const;
    std::string  to_absolute_path(object_guid const  guid) const;
    object_guid  from_relative_path(object_guid const  base_guid, std::string const&  relative_path) const;
    std::string  to_relative_path(object_guid const  guid, object_guid const  relative_base_guid) const;

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
    object_guid  folder_of(object_guid const  guid) const;
    std::string const&  name_of(object_guid const  guid) const;
    folder_guid_iterator  folders_begin() const;
    folder_guid_iterator  folders_end() const;
    void  for_each_parent_folder(object_guid  folder_guid, bool const  including_passed_folder,
                                 folder_visitor_type const&  visitor) const;
    void  for_each_child_folder(object_guid const  folder_guid, bool const  recursively, bool const  including_passed_folder,
                                folder_visitor_type const&  visitor) const;
    void  request_insert_folder(object_guid const  under_folder_guid, std::string const&  folder_name) const;
    void  request_erase_non_root_empty_folder(object_guid const  folder_guid) const;
    // Disabled (not const) for modules.
    object_guid  insert_folder(object_guid const  under_folder_guid, std::string const&  folder_name);
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
    void  request_insert_frame(object_guid const  under_folder_guid) const;
    void  request_erase_frame(object_guid const  frame_guid) const;
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
    BATCH_CLASS  batch_class(object_guid const  batch_guid) const;
    std::vector<object_guid> const&  frames_of_batch(object_guid const  batch_guid) const;
    // Disabled (not const) for modules.
    std::string const&  from_batch_guid(object_guid const  batch_guid);
    gfx::batch  from_batch_guid_to_batch(object_guid const  batch_guid);
    object_guid  insert_batch(object_guid const  folder_guid, std::string const&  name, BATCH_CLASS const  cls,
                              gfx::batch const  batch);
    void  erase_batch(object_guid const  batch_guid);
    object_guid  load_batch(
            object_guid const  folder_guid, std::string const&  name, BATCH_CLASS const  cls,
            std::string const&  disk_path,
            gfx::effects_config  effects_config,
            std::string const&  skin_name = "default"
            );
    object_guid  insert_batch_lines3d(
            object_guid const  folder_guid, std::string const&  name, BATCH_CLASS const  cls,
            std::vector<std::pair<vector3,vector3> > const&  lines,
            vector4 const&  common_colour
            );
    object_guid  insert_batch_wireframe_box(
            object_guid const  folder_guid, std::string const&  name, BATCH_CLASS const  cls,
            vector3 const&  half_sizes_along_axes,
            vector4 const&  colour
            );
    object_guid  insert_batch_solid_box(
            object_guid const  folder_guid, std::string const&  name, BATCH_CLASS const  cls,
            vector3 const&  half_sizes_along_axes,
            vector4 const&  colour
            );
    object_guid  insert_batch_wireframe_capsule(
            object_guid const  folder_guid, std::string const&  name, BATCH_CLASS const  cls,
            float_32_bit const  half_distance_between_end_points,
            float_32_bit const  thickness_from_central_line,
            natural_8_bit const  num_lines_per_quarter_of_circle,
            vector4 const&  colour
            );
    object_guid  insert_batch_solid_capsule(
            object_guid const  folder_guid, std::string const&  name, BATCH_CLASS const  cls,
            float_32_bit const  half_distance_between_end_points,
            float_32_bit const  thickness_from_central_line,
            natural_8_bit const  num_lines_per_quarter_of_circle,
            vector4 const&  colour
            );
    object_guid  insert_batch_wireframe_sphere(
            object_guid const  folder_guid, std::string const&  name, BATCH_CLASS const  cls,
            float_32_bit const  radius,
            natural_8_bit const  num_lines_per_quarter_of_circle,
            vector4 const&  colour
            );
    object_guid  insert_batch_solid_smooth_sphere(
            object_guid const  folder_guid, std::string const&  name, BATCH_CLASS const  cls,
            float_32_bit const  radius,
            natural_8_bit const  num_lines_per_quarter_of_circle,
            vector4 const&  colour
            );
    object_guid  insert_batch_solid_sphere(
            object_guid const  folder_guid, std::string const&  name, BATCH_CLASS const  cls,
            float_32_bit const  radius,
            natural_8_bit const  num_lines_per_quarter_of_circle,
            vector4 const&  colour
            );
    object_guid  insert_batch_triangle_mesh(
            object_guid const  folder_guid, std::string const&  name, BATCH_CLASS const  cls,
            std::vector< std::array<float_32_bit, 3> > const& vertices,
            std::vector< std::array<float_32_bit, 3> > const& normals,
            vector4 const&  colour
            );
    object_guid  insert_batch_wireframe_perspective_frustum(
            object_guid const  folder_guid, std::string const&  name, BATCH_CLASS const  cls,
            float_32_bit const  near_plane,
            float_32_bit const  far_plane,
            float_32_bit const  left_plane,
            float_32_bit const  right_plane,
            float_32_bit const  top_plane,
            float_32_bit const  bottom_plane,
            vector4 const&  colour,
            bool const  with_axis
            );
    object_guid  insert_batch_grid(
            object_guid const  folder_guid, std::string const&  name, BATCH_CLASS const  cls,
            float_32_bit const  max_x_coordinate,
            float_32_bit const  max_y_coordinate,
            float_32_bit const  max_z_coordinate,
            float_32_bit const  step_along_x_axis,
            float_32_bit const  step_along_y_axis,
            std::array<float_32_bit, 4> const&  colour_for_x_lines,
            std::array<float_32_bit, 4> const&  colour_for_y_lines,
            std::array<float_32_bit, 4> const&  colour_for_highlighted_x_lines,
            std::array<float_32_bit, 4> const&  colour_for_highlighted_y_lines,
            std::array<float_32_bit, 4> const&  colour_for_central_x_line,
            std::array<float_32_bit, 4> const&  colour_for_central_y_line,
            std::array<float_32_bit, 4> const&  colour_for_central_z_line,
            natural_32_bit const  highlight_every,
            gfx::GRID_MAIN_AXES_ORIENTATION_MARKER_TYPE const  main_exes_orientation_marker_type
            );
    object_guid  insert_batch_default_grid(object_guid const  folder_guid, std::string const&  name, BATCH_CLASS const  cls);

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
    angeo::COLLISION_MATERIAL_TYPE  collision_material_of(object_guid const  collider_guid) const;
    // Disabled (not const) for modules.
    angeo::collision_object_id  from_collider_guid(object_guid const  collider_guid);
    void  relocate_collider(object_guid const  collider_guid, matrix44 const&  world_matrix);
    //object_guid  insert_collider_box(
    //        vector3 const&  half_sizes_along_axes,
    //        matrix44 const&  from_base_matrix,
    //        angeo::COLLISION_MATERIAL_TYPE const  material,
    //        angeo::COLLISION_CLASS const  collision_class,
    //        bool const  is_dynamic
    //        );
    //object_guid  insert_collider_capsule(
    //        float_32_bit const  half_distance_between_end_points,
    //        float_32_bit const  thickness_from_central_line,
    //        matrix44 const&  from_base_matrix,
    //        angeo::COLLISION_MATERIAL_TYPE const  material,
    //        angeo::COLLISION_CLASS const  collision_class,
    //        bool const  is_dynamic
    //        );
    //object_guid  insert_collider_sphere(
    //        float_32_bit const  radius,
    //        matrix44 const&  from_base_matrix,
    //        angeo::COLLISION_MATERIAL_TYPE const  material,
    //        angeo::COLLISION_CLASS const  collision_class,
    //        bool const  is_dynamic
    //        );
    //void  insert_collider_triangle_mesh(
    //        natural_32_bit const  num_triangles,
    //        std::function<vector3(natural_32_bit, natural_8_bit)> const&  getter_of_end_points_in_model_space,
    //        matrix44 const&  from_base_matrix,
    //        angeo::COLLISION_MATERIAL_TYPE const  material,
    //        angeo::COLLISION_CLASS const  collision_class,
    //        bool const  is_dynamic,
    //        std::vector<object_guid>&  output_guids_of_individual_triangles
    //        );
    //void  erase_collider(object_guid const  collider_guid);

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
    vector3 const&  mass_centre_of_rigid_body(object_guid const  rigid_body_guid) const;
    quaternion const&  orientation_of_rigid_body(object_guid const  rigid_body_guid) const;
    std::vector<object_guid> const&  colliders_of_rigid_body(object_guid const  rigid_body_guid) const;
    // Disabled (not const) for modules.
    angeo::rigid_body_id  from_rigid_body_guid(object_guid const  rigid_body_guid);

    /////////////////////////////////////////////////////////////////////////////////////
    // SENSORS API
    /////////////////////////////////////////////////////////////////////////////////////

    using  sensor_guid_iterator = object_guid_iterator<OBJECT_KIND::SENSOR>;

    bool  is_valid_sensor_guid(object_guid const  sensor_guid) const;
    object_guid  folder_of_sensor(object_guid const  sensor_guid) const;
    std::string const&  name_of_sensor(object_guid const  sensor_guid) const;
    object_guid  to_sensor_guid(ai::object_id const  seid) const;
    sensor_guid_iterator  sensors_begin() const;
    sensor_guid_iterator  sensors_end() const;

    /////////////////////////////////////////////////////////////////////////////////////
    // ACTIVATORS API
    /////////////////////////////////////////////////////////////////////////////////////

    using  activator_guid_iterator = object_guid_iterator<OBJECT_KIND::ACTIVATOR>;

    bool  is_valid_activator_guid(object_guid const  activator_guid) const;
    object_guid  folder_of_activator(object_guid const  activator_guid) const;
    std::string const&  name_of_activator(object_guid const  activator_guid) const;
    object_guid  to_activator_guid(ai::object_id const  acid) const;
    activator_guid_iterator  activators_begin() const;
    activator_guid_iterator  activators_end() const;

    /////////////////////////////////////////////////////////////////////////////////////
    // DEVICES API
    /////////////////////////////////////////////////////////////////////////////////////

    using  device_guid_iterator = object_guid_iterator<OBJECT_KIND::DEVICE>;

    bool  is_valid_device_guid(object_guid const  device_guid) const;
    object_guid  folder_of_device(object_guid const  device_guid) const;
    std::string const&  name_of_device(object_guid const  device_guid) const;
    object_guid  to_device_guid(ai::object_id const  deid) const;
    device_guid_iterator  devices_begin() const;
    device_guid_iterator  devices_end() const;

    /////////////////////////////////////////////////////////////////////////////////////
    // AGENTS API
    /////////////////////////////////////////////////////////////////////////////////////

    using  agent_guid_iterator = object_guid_iterator<OBJECT_KIND::AGENT>;

    bool  is_valid_agent_guid(object_guid const  agent_guid) const;
    object_guid  folder_of_agent(object_guid const  agent_guid) const;
    std::string const&  name_of_agent(object_guid const  agent_guid) const;
    object_guid  to_agent_guid(ai::object_id const  agid) const;
    agent_guid_iterator  agents_begin() const;
    agent_guid_iterator  agents_end() const;

private:

    using  index_type = object_guid::index_type;

    template<typename T>
    struct  folder_element
    {
        using  module_specific_id = T;
        using  base_type = folder_element<module_specific_id>;
        folder_element() : id(), folder_index(0), element_name() {}
        folder_element(module_specific_id const  id_, index_type const  folder_index_, std::string const&  element_name_)
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
            , batch_class(BATCH_CLASS::COMMON_OBJECT)
            , frames()
        {}
        folder_element_batch(module_specific_id const  id_, index_type const  folder_index_, std::string const&  element_name_,
                gfx::batch const  batch_,
                BATCH_CLASS const  batch_class_
                )
            : base_type(id_, folder_index_, element_name_)
            , batch(batch_)
            , batch_class(batch_class_)
            , frames()
        {}
        gfx::batch  batch;
        BATCH_CLASS  batch_class;
        std::vector<object_guid>  frames;
    };

    struct  folder_element_collider : public folder_element<angeo::collision_object_id>
    {
        folder_element_collider()
            : base_type()
            , frame(invalid_object_guid())
            , owner(invalid_object_guid())
        {}
        folder_element_collider(module_specific_id const  id_, index_type const  folder_index_, std::string const&  element_name_,
                object_guid const  frame_,
                object_guid const  owner_
                )
            : base_type(id_, folder_index_, element_name_)
            , frame(frame_)
            , owner(owner_)
        {}
        object_guid  frame;
        object_guid  owner; // Can be either a rigid body, a sensor, or an activator.
    };

    struct  folder_element_rigid_body : public folder_element<angeo::rigid_body_id>
    {
        folder_element_rigid_body()
            : base_type()
            , frame(invalid_object_guid())
            , colliders()
        {}
        folder_element_rigid_body(module_specific_id const  id_, index_type const  folder_index_, std::string const&  element_name_,
                object_guid const  frame_
                )
            : base_type(id_, folder_index_, element_name_)
            , frame(frame_)
            , colliders()
        {}
        object_guid  frame;
        std::vector<object_guid>  colliders; // Only those whose owner is this rigid body.
    };

    // TODO: folder_element_* below are NOT properly implemented, i.e. they have been ignored so far.
    using  folder_element_sensor = folder_element<ai::object_id>;
    using  folder_element_activator = folder_element<ai::object_id>;
    using  folder_element_device = folder_element<ai::object_id>;
    using  folder_element_agent = folder_element<ai::object_id>;

    object_guid  m_root_folder;
    dynamic_array<folder_content_type, index_type>  m_folders;
    dynamic_array<folder_element_frame, index_type>  m_frames;
    dynamic_array<folder_element_batch, index_type>  m_batches;
    dynamic_array<folder_element_collider, index_type>  m_colliders;
    dynamic_array<folder_element_rigid_body, index_type>  m_rigid_bodies;
    dynamic_array<folder_element_sensor, index_type>  m_sensors;
    dynamic_array<folder_element_activator, index_type>  m_activators;
    dynamic_array<folder_element_device, index_type>  m_devices;
    dynamic_array<folder_element_agent, index_type>  m_agents;

    frames_provider  m_frames_provider;
    std::shared_ptr<angeo::collision_scene>  m_collision_scene_ptr;
    std::shared_ptr<angeo::rigid_body_simulator>  m_rigid_body_simulator_ptr;
    std::shared_ptr<ai::simulator>  m_ai_simulator_ptr;

    std::unordered_map<frame_id, object_guid>  m_frids_to_guids;
    std::unordered_map<std::string, object_guid>  m_batches_to_guids;
    std::unordered_map<angeo::collision_object_id, object_guid>  m_coids_to_guids;
    std::unordered_map<angeo::rigid_body_id, object_guid>  m_rbids_to_guids;
    std::unordered_map<ai::object_id, object_guid>  m_seids_to_guids;
    std::unordered_map<ai::object_id, object_guid>  m_acids_to_guids;
    std::unordered_map<ai::object_id, object_guid>  m_deids_to_guids;
    std::unordered_map<ai::object_id, object_guid>  m_agids_to_guids;

    std::unordered_set<index_type>  m_moveable_colliders;
    std::unordered_set<index_type>  m_moveable_rigid_bodies;

    struct  imported_scene_data
    {
        imported_scene_data(async::finalise_load_on_destroy_ptr const  finaliser);
        boost::property_tree::ptree const&  ptree() const { return m_ptree; }
    private:
        boost::property_tree::ptree  m_ptree;
    };

    struct  imported_scene : public async::resource_accessor<imported_scene_data>
    {
        imported_scene() : async::resource_accessor<imported_scene_data>() {}
        imported_scene(boost::filesystem::path const&  path);
        boost::property_tree::ptree const&  ptree() const { return resource().ptree(); }
    };

    struct  request_props_imported_scene
    {
        imported_scene  scene;
        object_guid  folder_guid;
        bool  store_in_cache;
    };

    mutable std::vector<request_props_imported_scene> m_requests_queue_scene_import;
    std::unordered_map<std::string, imported_scene> m_cache_of_imported_scenes;

    simulation_context(
            std::shared_ptr<angeo::collision_scene> const  collision_scene_ptr_,
            std::shared_ptr<angeo::rigid_body_simulator> const  rigid_body_simulator_ptr_,
            std::shared_ptr<ai::simulator> const  ai_simulator_ptr_
            );

    simulation_context(simulation_context const&) = delete;
    simulation_context(simulation_context const&&) = delete;
    simulation_context& operator=(simulation_context const&) = delete;
    simulation_context& operator=(simulation_context const&&) = delete;
};


}

#endif
