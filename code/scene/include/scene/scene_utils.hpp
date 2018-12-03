#ifndef E2_SCENE_SCENE_UTILS_HPP_INCLUDED
#   define E2_SCENE_SCENE_UTILS_HPP_INCLUDED

#   include <scene/scene.hpp>
#   include <scene/scene_node_id.hpp>
#   include <scene/scene_node_record_id.hpp>
#   include <scene/scene_record_id.hpp>
#   include <angeo/tensor_math.hpp>
#   include <angeo/axis_aligned_bounding_box.hpp>
#   include <utility/assumptions.hpp>
#   include <boost/any.hpp>
#   include <string>
#   include <map>
#   include <vector>
#   include <unordered_set>
#   include <functional>

namespace scn { namespace detail {


/**
 * The following 'get_record_*' functions return 'nullptr' if searched data of record 'id' are not present in the scene.
 */
scene_node::record_holder const*  get_record_holder_ptr(scene_node const&  n, scene_node_record_id const&  id);
scene_node::record_holder const*  get_record_holder_ptr(scene const&  s, scene_record_id const&  id);
scene_node::record_bbox_getter const*  get_record_bbox_getter_ptr(scene_node const&  n, scene_node_record_id const&  id);
scene_node::record_bbox_getter const*  get_record_bbox_getter_ptr(scene const&  s, scene_record_id const&  id);

inline scene_node::record_holder*  get_record_holder_ptr_non_const(scene_node&  n, scene_node_record_id const&  id)
{ return const_cast<scene_node::record_holder*>(get_record_holder_ptr(n,id)); }
inline scene_node::record_holder*  get_record_holder_ptr_non_const(scene&  s, scene_record_id const&  id)
{ return const_cast<scene_node::record_holder*>(get_record_holder_ptr(s, id)); }

/**
 * Calls the pointed bbox getter for 'out_box' and returns 'true', if the pointer is not 'nullptr'. Otherwise returns 'false'.
 */
bool  get_bbox(scene_node::record_bbox_getter const* const  bbox_getter_ptr, angeo::axis_aligned_bounding_box&  out_bbox);


}}

namespace scn {


inline std::unordered_map<scene_node::node_name, scene_node_ptr> const&  get_root_nodes(scene const& s)
{
    return s.get_root_nodes();
}

inline scene_node_ptr  get_node(scene const&  s, scene_node_id const&  id)
{
    return s.get_scene_node(id);
}

inline bool  has_node(scene const&  s, scene_node_id const&  id)
{
    return get_node(s,id) != nullptr;
}

/**
* The following 'get_folder_*' functions return either genuine map (if the folder exists) or a dummy empty map (otherwise).
*/
scene_node::folder_content::records_map const&  get_folder_records_map(
        scene_node const&  node,
        scene_node::folder_name const&  folder_name
        );
scene_node::folder_content::bbox_getters_map const&  get_folder_bbox_getters_map(
        scene_node const&  node,
        scene_node::folder_name const&  folder_name
        );


template<typename TRecordValueType>
inline TRecordValueType const&  get_record(scene_node const&  n, scene_node_record_id const&  id)
{
    return *boost::any_cast<TRecordValueType const>(detail::get_record_holder_ptr(n, id));
}

inline bool  has_record(scene_node const&  n, scene_node_record_id const&  id)
{
    return detail::get_record_holder_ptr(n, id) != nullptr;
}

template<typename TRecordValueType>
inline TRecordValueType const&  get_record(scene const&  s, scene_record_id const&  id)
{
    return *boost::any_cast<TRecordValueType const>(detail::get_record_holder_ptr(s, id));
}

inline bool  has_record(scene const&  s, scene_record_id const&  id)
{
    return detail::get_record_holder_ptr(s, id) != nullptr;
}

template<typename TRecordValueType>
TRecordValueType const&  record_cast(scene_node::record_holder const&  holder)
{
    return *boost::any_cast<TRecordValueType const>(&holder);
}

template<typename TRecordValueType>
TRecordValueType&  record_cast(scene_node::record_holder&  holder)
{
    return *boost::any_cast<TRecordValueType>(&holder);
}

inline bool  get_bbox(scene_node const&  n, angeo::axis_aligned_bounding_box&  out_bbox)
{
    out_bbox.min_corner = vector3(-0.1f, -0.1f, -0.1f);
    out_bbox.max_corner = vector3(0.1f, 0.1f, 0.1f);
    return true;
}

/**
 * If the record identified by 'id' has defined a bounding box, then the 'out_box' is set
 * to that bounding box and the function returns 'true'. Otherwise, the 'out_bbox' is not
 * modified and the function returns 'false'.
 */
inline bool  get_bbox(scene_node const&  n, scene_node_record_id const&  id, angeo::axis_aligned_bounding_box&  out_bbox)
{
    return detail::get_bbox(detail::get_record_bbox_getter_ptr(n, id), out_bbox);
}

inline bool  has_bbox(scene_node const&  n, scene_node_record_id const&  id)
{
    return detail::get_record_bbox_getter_ptr(n, id) != nullptr;
}

/**
 * If the record identified by 'id' has defined a bounding box, then the 'out_box' is set
 * to that bounding box and the function returns 'true'. Otherwise, the 'out_bbox' is not
 * modified and the function returns 'false'.
 */
inline bool  get_bbox(scene const&  s, scene_record_id const&  id, angeo::axis_aligned_bounding_box&  out_bbox)
{
    return detail::get_bbox(detail::get_record_bbox_getter_ptr(s, id), out_bbox);
}

inline bool  has_bbox(scene const&  s, scene_record_id const&  id)
{
    return detail::get_record_bbox_getter_ptr(s, id) != nullptr;
}

inline scene_node_ptr  insert_node(
        scene&  s,
        scene_node_id const&  id,
        vector3 const&  origin = vector3_zero(),
        quaternion const&  orientation = quaternion_identity()
        )
{
    return s.insert_scene_node(id, origin, orientation);
}

inline void  erase_node(scene&  s, scene_node_id const&  id)
{
    return s.erase_scene_node(id);
}

template<typename TRecordValueType>
void  insert_record(
        scene_node&  n,
        scene_node_record_id const&  record_id,
        TRecordValueType const&  value
        )
{
    auto const  folder_it = n.insert_folder(record_id.get_folder_name());
    folder_it->second.insert_record<TRecordValueType>(record_id.get_record_name(), value);
}

template<typename TRecordValueType>
void  insert_record(
        scene_node&  n,
        scene_node_record_id const&  record_id,
        TRecordValueType const&  value,
        scene_node::record_bbox_getter const&  bbox_getter
        )
{
    auto const  folder_it = n.insert_folder(record_id.get_folder_name());
    folder_it->second.insert_record<TRecordValueType>(record_id.get_record_name(), value, bbox_getter);
}

template<typename TRecordValueType>
void  insert_record(
        scene_node&  n,
        scene_node_record_id const&  record_id,
        TRecordValueType const&  value,
        angeo::axis_aligned_bounding_box const&  bbox
        )
{
    auto const  folder_it = n.insert_folder(record_id.get_folder_name());
    folder_it->second.insert_record<TRecordValueType>(record_id.get_record_name(), value, bbox);
}

template<typename TRecordValueType>
void  insert_record(
        scene&  s,
        scene_record_id const&  record_id,
        TRecordValueType const&  value
        )
{
    scene_node_ptr  node_ptr = get_node(s, record_id.get_node_id());
    ASSUMPTION(node_ptr != nullptr);
    insert_record<TRecordValueType>(*node_ptr, { record_id.get_folder_name(), record_id.get_record_name() }, value);
}

template<typename TRecordValueType>
void  insert_record(
        scene&  s,
        scene_record_id const&  record_id,
        TRecordValueType const&  value,
        scene_node::record_bbox_getter const&  bbox_getter
        )
{
    scene_node_ptr  node_ptr = get_node(s, record_id.get_node_id());
    ASSUMPTION(node_ptr != nullptr);
    insert_record<TRecordValueType>(*node_ptr, { record_id.get_folder_name(), record_id.get_record_name() }, value, bbox_getter);
}

template<typename TRecordValueType>
void  insert_record(
        scene&  s,
        scene_record_id const&  record_id,
        TRecordValueType const&  value,
        angeo::axis_aligned_bounding_box const&  bbox
        )
{
    scene_node_ptr  node_ptr = get_node(s, record_id.get_node_id());
    ASSUMPTION(node_ptr != nullptr);
    insert_record<TRecordValueType>(*node_ptr, { record_id.get_folder_name(), record_id.get_record_name() }, value, bbox);
}

inline void  erase_folder(scene_node&  n, scene_node::folder_name const&  name)
{
    n.erase_folder(name);
}

inline void  erase_record(scene_node&  n, scene_node_record_id const&  record_id)
{
    auto const  folder_it = n.find_folder(record_id.get_folder_name());
    folder_it->second.erase_record(record_id.get_record_name());
}

inline void  erase_record(scene&  s, scene_record_id const&  record_id)
{
    scene_node_ptr  node_ptr = get_node(s, record_id.get_node_id());
    ASSUMPTION(node_ptr != nullptr);
    erase_record(*node_ptr, { record_id.get_folder_name(), record_id.get_record_name() });
}


void  transform_origin_and_orientation(
        matrix44 const  transformation,
        vector3&  origin,
        quaternion&  orientation
        );

inline void  transform_origin_and_orientation_from_world_to_scene_node(
        scene_node_const_ptr const  node,
        vector3&  origin,
        quaternion&  orientation
        )
{
    transform_origin_and_orientation(inverse44(node->get_world_matrix()), origin, orientation);
}


inline void  transform_origin_and_orientation_from_scene_node_to_world(
        scene_node_ptr const  node,
        vector3&  origin,
        quaternion&  orientation
        )
{
    transform_origin_and_orientation(node->get_world_matrix(), origin, orientation);
}


inline vector3  transform_point_from_scene_node_to_world(
        scene_node const&  node,
        vector3 const&  point
        )
{
    return transform_point(point, node.get_world_matrix());
}


inline vector3  transform_vector_from_scene_node_to_world(
        scene_node const&  node,
        vector3 const&  vector
        )
{
    return transform_vector(vector, node.get_world_matrix());
}


inline vector3  transform_point_from_world_to_scene_node(
        scene_node const&  node,
        vector3 const&  point
        )
{
    return transform_point(point, inverse44(node.get_world_matrix()));
}


inline vector3  transform_vector_from_world_to_scene_node(
        scene_node const&  node,
        vector3 const&  vector
        )
{
    return transform_vector(vector, inverse44(node.get_world_matrix()));
}


inline quaternion  transform_orientation_from_scene_node_to_world(
        scene_node const&  node,
        quaternion const&  orientation
        )
{
    return transform(orientation, node.get_world_matrix());
}


inline quaternion  transform_orientation_from_world_to_scene_node(
        scene_node const&  node,
        quaternion const&  orientation
        )
{
    return transform(orientation, inverse44(node.get_world_matrix()));
}


vector3  get_center_of_scene_nodes(std::unordered_set<scene_node_ptr> const&  nodes);



/**
 * Keys in the output map represent parameters on the passed line to collision points
 * of the line with bounding boxes the corresponding scene object (i.e. node or a record).
 */
void  collision_scene_vs_line(
        scene const&  scene,
        vector3 const&  line_start_point,
        vector3 const&  line_end_point,
        std::multimap<scalar, scn::scene_node_id>&  output_nodes
        );


/**
 * Keys in both output maps represent parameters on the passed line to collision points
 * of the line with bounding boxes of the corresponding scene object (i.e. node or a record).
 * If a passed pointer to an output map is 'nullptr', then related scene object will not be
 * considered for collision tests with the line.
 */
void  collision_scene_vs_line(
        scene const&  scene,
        vector3 const&  line_start_point,
        vector3 const&  line_end_point,
        std::multimap<scalar, scn::scene_node_id>* const  output_nodes_ptr,
        std::multimap<scalar, scn::scene_record_id>* const  output_records_ptr
        );

/**
 * Processed the output from the function 'collision_scene_vs_line' above so that
 * it writes to the output all scene objects whose parameter on the line lies within
 * the range <t0,t0+param_region_size>, where t0 represents the smallest parameter (key)
 * in both maps. Any of both pointers to the input maps can be 'nullptr', which means,
 * that the map won't be considered. Also, elements collected from the map pointed to by
 * 'nodes_on_line_ptr' will be converted to 'scn::scene_record_id' instances returning
 * 'true' for the method 'is_node_reference'. If the pointer 'output_params_of_records_in_range_ptr'
 * is not nullptr, then the vector will be filled in by parameters on the line of the
 * corresponding records written to 'output_nearnest_records_in_range' (for each record)
 * exactly one parameter).
 */
void  collect_nearest_scene_objects_on_line_within_parameter_range(
        std::multimap<scalar, scn::scene_node_id> const* const  nodes_on_line_ptr,
        std::multimap<scalar, scn::scene_record_id> const* const  records_on_line_ptr,
        float_32_bit const  param_region_size,
        std::vector<scn::scene_record_id>&  output_nearnest_records_in_range,
        std::vector<scalar>*  output_params_of_records_in_range_ptr
        );


}

#endif
