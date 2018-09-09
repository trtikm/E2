#ifndef ANGEO_PROXIMITY_MAP_HPP_INCLUDED
#   define ANGEO_PROXIMITY_MAP_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <angeo/collide.hpp>
#   include <utility/assumptions.hpp>
#   include <utility/invariants.hpp>
#   include <utility/timeprof.hpp>
#   include <unordered_set>
#   include <functional>
#   include <memory>
#   include <vector>
#   include <array>
#   include <mutex>

namespace angeo {


/**
 * The proximity map provides fast search for objects distributed in 3D space.
 * A shape of an object is approximated by an axis aligned bounding box. The
 * map this assumes, there is a fast algorithm obtaining a bounding box for
 * any objects inserted into the map. Instead of requiring the type of objects
 * contain a method for obtaining a bounding box, the map rather accepts two
 * algorithms in the initialisation providing minimal and maximal corner points
 * of a bounding box for any object in the map. For example, let type of our
 * objects looks like this:
 *
 *      struct my_object3d_type
 *      {
 *          vector3 lo, hi; // min. and max. corner points of the bounding box.
 *      };
 *
 * Then we can initialise a proximity map of these objects as follows:
 *
 *      angeo::proximity_map<my_object3d_type*> map(
 *              [](my_object3d_type* obj) { return obj->lo; },  // getter for min. corner
 *              [](my_object3d_type* obj) { return obj->hi; }   // getter for max. corner
 *              );
 *
 * The typical usage of the map in a 3d simulation is as follows. In the initial
 * step, we insert objects into the map. 
 *
 *      std::vector<my_object3d_type*>  my_objects_to_simulate;
 *      ...  // Create and initialise your objects
 *      for (my_object3d_type* obj :  my_objects_to_simulate)
 *          map.insert(obj);  // Insert objects into the map.
 *      map.rebalance();  // Optimize the map structure for subsequent search operations.
 * 
 * In the update step of the simulation (from the current state to the next one)
 * you use the map to search for objects in desired space. For example, a search
 * for objects in a 3d space denoted by an axis aligned bounding box will look
 * like this:
 *
 *      vector3 query_bbox_min_corner, query_bbox_max_corner; // Defines 3d space where to search for objects
 *      ...  // Initialise the corner points to desired values.
 *      std::unordered_set<my_object3d_type*>  collected_objects;  // Will be filled in by found objects.
 *      map.find_by_bbox(
 *              query_bbox_min_corner,
 *              query_bbox_max_corner,
 *              [&collected_objects](my_object3d_type* obj) {
 *                  collected_objects.insert(obj);
 *                  return true; // Tell the map to continue the search for remaining objects (if any).
 *                               // The return value 'false' would instruct the map to terminate the search.
 *               });
 *
 * NOTE: In order to search for objects colliding with a line, use the function
 *       'find_by_line' instead of 'find_by_bbox'.
 *
 * NOTE: The proximity also provides a method 'enumerate' providing an enumeration of all
 *       objects in the proximity map PER PROXIMITY CLUSTER. Objects are grouped into cluster
 *       according to their spatial proximity. Technically speaking, each leaf node in the
 *       proximity map represents one such cluster. You can control maximal size of the clusters
 *       via the parameter 'max_num_objects_in_leaf_before_split' of the constructor of the
 *       map. The method 'enumerate' first enumerates all objects in the first leaf node (cluster),
 *       then all nodes in the second leaf, and so on. Each enumerated object is passed to the
 *       output callback function together with an index of the currently enumerated leaf node.
 *       So, the sequence of objects (with leaf indices) after enumeration of all K leaf nodes in
 *       the proximity map will look like this:
 *          (obj[0], 0), (obj[1], 0), ... ,(obj[n[0]-1], 0),                    // all n[0] objects in the leaf 0
 *          (obj[n[0]], 1), (obj[n[0]+1], 1), ..., (obj[n[0]+n[1]-1], 1),       // all n[1] objects in the leaf 1
 *          .........................................................................................................
 *          (obj[n[0]+...+n[K-2]], K-1), ..., (obj[n[0]+...+n[K-1]-1], K-1),    // all n[K-1] objects in the leaf K-1
 *
 * NOTE: All methods 'find_by_bbox', 'find_by_line', and 'enumerate' may send the same
 *       object several times to the collector callback. So, the collector is responsible
 *       for filtering out those duplicities.
 *
 * In the end of the update step of the simulation you typically want to move objects
 * from current positions to the new (computed) ones. You need to erase an object from
 * the map before you change its position and insert it back after the new position is set.
 * So, the process may looks like this:
 *
 *      for (my_object3d_type* obj :  my_objects_to_simulate)
 *      {
 *          map.erase(obj);   // Erase the object from the map using its old position.
 *
 *          // Update object's position according to computed data in this time step
 *          // of the simulation. Let us say, it is just object's velocity vector.
 *          vector3 const  object_translation = get_computed_veocity(obj) * simulation_time_step;
 *          obj->lo += object_translation;
 *          obj->hi += object_translation;
 *
 *          map.insert(obj);  // Insert object back into the map for the updated position.
 *      }
 *      // Once we updated the map for all relocated objects, we can optimize the map structure again
 *      // for search operations in the next update step of the simulation.
 *      map.rebalance();
 *
 * NOTE: The proximity map is partially thread-safe. It means that:
 *          - You can call insert and/or erase methods from different threads.
 *          - You can call find_by_bbox and/or find_by_line methods from different threads.
 *          - NO OTHER CONCURRENT EXECUTION OF METHODS IS ALLOWED.
 *
 */
template<typename  object_type__>
struct proximity_map
{
    using  object_type = object_type__;

    proximity_map(
            std::function<vector3(object_type)> const&  bbox_min_corner_of_object_getter,
            std::function<vector3(object_type)> const&  bbox_max_corner_of_object_getter,
            natural_32_bit const  max_num_objects_in_leaf_before_split = 25U,
            float_32_bit const  treashold_for_applying_node_balancing_rotations = 0.5f // 0.0 - min rotate, 1.0 - max rotate
            );

    bool  insert(object_type const object);
    bool  erase(object_type const object);

    void  clear();

    void  rebalance(natural_32_bit const  num_threads_available = 0U);

    void  find_by_bbox(
            vector3 const& query_bbox_min_corner,
            vector3 const& query_bbox_max_corner,
            std::function<bool(object_type)> const&  output_collector
            );

    void  find_by_line(
            vector3 const&  line_begin,
            vector3 const&  line_end,
            std::function<bool(object_type)> const&  output_collector
            );

    void  enumerate(std::function<bool(object_type, natural_32_bit)> const&  output_collector);

private:

    struct split_node
    {
        enum struct SPLIT_PLANE_NORMAL_DIRECTION : int
        {
            X_AXIS = 0,
            Y_AXIS = 1,
            Z_AXIS = 2,
            NOT_SET = 3,
        };

        split_node();

        vector3  m_spit_plane_origin;
        SPLIT_PLANE_NORMAL_DIRECTION  m_split_plane_normal_direction;

        natural_32_bit  m_num_objects;

        std::unique_ptr<split_node>  m_front_child_node;
        std::unique_ptr<split_node>  m_back_child_node;

        std::unique_ptr<std::unordered_set<object_type> >  m_objects;

        std::mutex  m_mutex;
    };

    static bool  insert(
            split_node* const  node_ptr,
            object_type const object,
            vector3 const&  object_bbox_min_corner,
            vector3 const&  object_bbox_max_corner
            );

    static bool  erase(
            split_node* const  node_ptr,
            object_type const object,
            vector3 const&  object_bbox_min_corner,
            vector3 const&  object_bbox_max_corner
            );

    void  rebalance(
            split_node* const  node_ptr,
            split_node* const  parent_node_ptr,
            natural_32_bit const  num_threads_available
            );

    bool  find_by_bbox(
            split_node*  node_ptr,
            vector3 const& query_bbox_min_corner,
            vector3 const& query_bbox_max_corner,
            std::function<bool(object_type)> const&  output_collector
            );

    bool  find_by_line(
            split_node* const  node_ptr,
            vector3 const&  line_begin,
            vector3 const&  line_end,
            std::function<bool(object_type)> const&  output_collector
            );

    bool  enumerate(
            split_node* const  node_ptr,
            natural_32_bit&  output_leaf_node_index,
            std::function<bool(object_type, natural_32_bit)> const&  output_collector
            );

    void  apply_node_split(split_node* const  node_ptr);
    static void  apply_node_merge(split_node* const  node_ptr);

    void  apply_node_rotation_from_back_to_front(split_node* const  node_ptr, split_node* const  parent_node_ptr);
    void  apply_node_rotation_from_front_to_back(split_node* const  node_ptr, split_node* const  parent_node_ptr);
    void  apply_objects_move(split_node* const  from_node_ptr, split_node* const  to_node_ptr);

    std::function<vector3(object_type)>  m_get_bbox_min_corner;
    std::function<vector3(object_type)>  m_get_bbox_max_corner;
    natural_32_bit  m_max_num_objects_in_leaf_before_split;
    float_32_bit  m_min_ratio_for_applycation_balancing_rotation;

    std::unique_ptr<split_node>  m_root;
};


template<typename  object_type__>
proximity_map<object_type__>::proximity_map(
        std::function<vector3(object_type)> const&  bbox_min_corner_of_object_getter,
        std::function<vector3(object_type)> const&  bbox_max_corner_of_object_getter,
        natural_32_bit const  max_num_objects_in_leaf_before_split,
        float_32_bit const  treashold_for_applying_node_balancing_rotations
        )
    : m_get_bbox_min_corner(bbox_min_corner_of_object_getter)
    , m_get_bbox_max_corner(bbox_max_corner_of_object_getter)
    , m_max_num_objects_in_leaf_before_split(max_num_objects_in_leaf_before_split)
    , m_min_ratio_for_applycation_balancing_rotation(
            0.5f + (1.0f - treashold_for_applying_node_balancing_rotations) * (1.0f - 0.5f)
            )
    , m_root(new split_node)
{
    ASSUMPTION(m_max_num_objects_in_leaf_before_split > 0U);
    ASSUMPTION(m_min_ratio_for_applycation_balancing_rotation > 0.5f);
}


template<typename  object_type__>
proximity_map<object_type__>::split_node::split_node()
    : m_spit_plane_origin(vector3_zero())
    , m_split_plane_normal_direction(SPLIT_PLANE_NORMAL_DIRECTION::NOT_SET)
    , m_num_objects(0U)
    , m_front_child_node(nullptr)
    , m_back_child_node(nullptr)
    , m_objects(new std::unordered_set<object_type>)
    , m_mutex()
{
}


template<typename  object_type__>
bool  proximity_map<object_type__>::insert(object_type const object)
{
    TMPROF_BLOCK();

    vector3 const  min_corner = m_get_bbox_min_corner(object);
    vector3 const  max_corner = m_get_bbox_max_corner(object);
    return insert(m_root.get(), object, min_corner, max_corner);
}


template<typename  object_type__>
bool  proximity_map<object_type__>::insert(
        split_node* const  node_ptr,
        object_type const object,
        vector3 const&  object_bbox_min_corner,
        vector3 const&  object_bbox_max_corner
        )
{
    if (node_ptr->m_split_plane_normal_direction == split_node::SPLIT_PLANE_NORMAL_DIRECTION::NOT_SET)
    {
        std::lock_guard<std::mutex> lock(node_ptr->m_mutex);
        if (node_ptr->m_objects->insert(object).second)
        {
            ++node_ptr->m_num_objects;
            return true;
        }
        return false;
    }

    int const  coord_idx = (int)node_ptr->m_split_plane_normal_direction;

    if (object_bbox_min_corner(coord_idx) > node_ptr->m_spit_plane_origin(coord_idx))
    {
        if (insert(node_ptr->m_front_child_node.get(), object, object_bbox_min_corner, object_bbox_max_corner))
        {
            std::lock_guard<std::mutex> lock(node_ptr->m_mutex);
            ++node_ptr->m_num_objects;
            return true;
        }
        return false;
    }
    
    if (object_bbox_max_corner(coord_idx) < node_ptr->m_spit_plane_origin(coord_idx))
    {
        if (insert(node_ptr->m_back_child_node.get(), object, object_bbox_min_corner, object_bbox_max_corner))
        {
            std::lock_guard<std::mutex> lock(node_ptr->m_mutex);
            ++node_ptr->m_num_objects;
            return true;
        }
        return false;
    }

    bool const  front_inserted =
            insert(node_ptr->m_front_child_node.get(), object, object_bbox_min_corner, object_bbox_max_corner);
    bool const  back_inserted =
            insert(node_ptr->m_back_child_node.get(), object, object_bbox_min_corner, object_bbox_max_corner);
    if (front_inserted || back_inserted)
    {
        std::lock_guard<std::mutex> lock(node_ptr->m_mutex);
        ++node_ptr->m_num_objects;
        return true;
    }
    return false;
}


template<typename  object_type__>
bool  proximity_map<object_type__>::erase(object_type const object)
{
    TMPROF_BLOCK();

    vector3 const  min_corner = m_get_bbox_min_corner(object);
    vector3 const  max_corner = m_get_bbox_max_corner(object);
    return erase(m_root.get(), object, min_corner, max_corner);
}


template<typename  object_type__>
bool  proximity_map<object_type__>::erase(
        split_node* const  node_ptr,
        object_type const  object,
        vector3 const&  object_bbox_min_corner,
        vector3 const&  object_bbox_max_corner
        )
{
    if (node_ptr->m_split_plane_normal_direction == split_node::SPLIT_PLANE_NORMAL_DIRECTION::NOT_SET)
    {
        std::lock_guard<std::mutex> lock(node_ptr->m_mutex);
        if (node_ptr->m_objects->erase(object) != 0U)
        {
            --node_ptr->m_num_objects;
            return true;
        }
        return false;
    }

    int const  coord_idx = (int)node_ptr->m_split_plane_normal_direction;

    if (object_bbox_min_corner(coord_idx) > node_ptr->m_spit_plane_origin(coord_idx))
    {
        if (erase(node_ptr->m_front_child_node.get(), object, object_bbox_min_corner, object_bbox_max_corner))
        {
            std::lock_guard<std::mutex> lock(node_ptr->m_mutex);
            --node_ptr->m_num_objects;
            return true;
        }
        return false;
    }
    
    if (object_bbox_max_corner(coord_idx) < node_ptr->m_spit_plane_origin(coord_idx))
    {
        if (erase(node_ptr->m_back_child_node.get(), object, object_bbox_min_corner, object_bbox_max_corner))
        {
            std::lock_guard<std::mutex> lock(node_ptr->m_mutex);
            --node_ptr->m_num_objects;
            return true;
        }
        return false;
    }

    bool const  front_erased =
        erase(node_ptr->m_front_child_node.get(), object, object_bbox_min_corner, object_bbox_max_corner);
    bool const  back_erased =
        erase(node_ptr->m_back_child_node.get(), object, object_bbox_min_corner, object_bbox_max_corner);
    if (front_erased || back_erased)
    {
        std::lock_guard<std::mutex> lock(node_ptr->m_mutex);
        --node_ptr->m_num_objects;
        return true;
    }
    return false;
}


template<typename  object_type__>
void  proximity_map<object_type__>::clear()
{
    m_root.reset(new split_node);
}


template<typename  object_type__>
void  proximity_map<object_type__>::rebalance(natural_32_bit const  num_threads_available)
{
    TMPROF_BLOCK();

    rebalance(m_root.get(), nullptr, num_threads_available);
}


template<typename  object_type__>
void  proximity_map<object_type__>::rebalance(
        split_node* const  node_ptr,
        split_node* const  parent_node_ptr,
        natural_32_bit const  num_threads_available
        )
{
    if (node_ptr->m_split_plane_normal_direction == split_node::SPLIT_PLANE_NORMAL_DIRECTION::NOT_SET)
    {
        if (node_ptr->m_num_objects <= m_max_num_objects_in_leaf_before_split ||
            (parent_node_ptr != nullptr && parent_node_ptr->m_num_objects == node_ptr->m_num_objects))
        {
            return;
        }
        apply_node_split(node_ptr);
    }

    // TODO: use 'num_threads_available' to spawn a thread for back child, if a tread is available.
    rebalance(node_ptr->m_front_child_node.get(), node_ptr, num_threads_available);
    rebalance(node_ptr->m_back_child_node.get(), node_ptr, num_threads_available);

    if (node_ptr->m_num_objects < m_max_num_objects_in_leaf_before_split)
    {
        apply_node_merge(node_ptr);
        return;
    }

    natural_32_bit const  diff_front_back_objects =
            node_ptr->m_front_child_node->m_num_objects - node_ptr->m_back_child_node->m_num_objects;
    float_32_bit const  ratio = (float_32_bit)diff_front_back_objects / (float_32_bit)node_ptr->m_num_objects;

    if (ratio >= m_min_ratio_for_applycation_balancing_rotation)
    {
        apply_node_rotation_from_back_to_front(node_ptr, parent_node_ptr);
        return;
    }
    if (ratio <= -m_min_ratio_for_applycation_balancing_rotation)
    {
        apply_node_rotation_from_front_to_back(node_ptr, parent_node_ptr);
        return;
    }
}


template<typename  object_type__>
void  proximity_map<object_type__>::find_by_bbox(
        vector3 const& query_bbox_min_corner,
        vector3 const& query_bbox_max_corner,
        std::function<bool(object_type)> const&  output_collector
        )
{
    TMPROF_BLOCK();

    find_by_bbox(m_root.get(), query_bbox_min_corner, query_bbox_max_corner, output_collector);
}


template<typename  object_type__>
bool  proximity_map<object_type__>::find_by_bbox(
        split_node* const  node_ptr,
        vector3 const& query_bbox_min_corner,
        vector3 const& query_bbox_max_corner,
        std::function<bool(object_type)> const&  output_collector
        )
{
    if (node_ptr->m_split_plane_normal_direction == split_node::SPLIT_PLANE_NORMAL_DIRECTION::NOT_SET)
    {
        for (object_type  object : *node_ptr->m_objects)
        {
            if (collision_bbox_bbox(
                    query_bbox_min_corner,
                    query_bbox_max_corner,
                    m_get_bbox_min_corner(object),
                    m_get_bbox_max_corner(object)
                    ))
            {
                if (output_collector(object) == false)
                    return false;
            }
        }
        return true;
    }

    int const  coord_idx = (int)node_ptr->m_split_plane_normal_direction;

    if (query_bbox_min_corner(coord_idx) > node_ptr->m_spit_plane_origin(coord_idx))
        return find_by_bbox(node_ptr->m_front_child_node.get(), query_bbox_min_corner, query_bbox_max_corner, output_collector);
    else if (query_bbox_max_corner(coord_idx) < node_ptr->m_spit_plane_origin(coord_idx))
        return find_by_bbox(node_ptr->m_back_child_node.get(), query_bbox_min_corner, query_bbox_max_corner, output_collector);
    else
    {
        // We do not have to cut the query AABB into two sub-AABBs, because bothsub-AABBs
        // would have the same lenghts along the remaining two axes as the original AABB.
        // So, the cutting will not lead to exploration of less nodes.

        if (find_by_bbox(node_ptr->m_front_child_node.get(), query_bbox_min_corner, query_bbox_max_corner, output_collector) == false)
            return false;
        return find_by_bbox(node_ptr->m_back_child_node.get(), query_bbox_min_corner, query_bbox_max_corner, output_collector);
    }
}


template<typename  object_type__>
void  proximity_map<object_type__>::find_by_line(
        vector3 const&  line_begin,
        vector3 const&  line_end,
        std::function<bool(object_type)> const&  output_collector
        )
{
    TMPROF_BLOCK();

    find_by_line(m_root.get(), line_begin, line_end, output_collector);
}


template<typename  object_type__>
bool  proximity_map<object_type__>::find_by_line(
        split_node* const  node_ptr,
        vector3 const&  line_begin,
        vector3 const&  line_end,
        std::function<bool(object_type)> const&  output_collector
        )
{
    if (node_ptr->m_split_plane_normal_direction == split_node::SPLIT_PLANE_NORMAL_DIRECTION::NOT_SET)
    {
        for (object_type  object : *node_ptr->m_objects)
        {
            if (clip_line_into_bbox(
                    line_begin,
                    line_end,
                    m_get_bbox_min_corner(object),
                    m_get_bbox_max_corner(object),
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr
                    ))
            {
                if (output_collector(object) == false)
                    return false;
            }
        }
        return true;
    }

    int const  coord_idx = (int)node_ptr->m_split_plane_normal_direction;

    if (line_begin(coord_idx) > node_ptr->m_spit_plane_origin(coord_idx) &&
        line_end(coord_idx) > node_ptr->m_spit_plane_origin(coord_idx))
    {
        return find_by_line(node_ptr->m_front_child_node.get(), line_begin, line_end, output_collector);
    }
    else if (line_begin(coord_idx) < node_ptr->m_spit_plane_origin(coord_idx) &&
             line_end(coord_idx) < node_ptr->m_spit_plane_origin(coord_idx))
    {
        return find_by_line(node_ptr->m_back_child_node.get(), line_begin, line_end, output_collector);
    }
    else
    {
        // We have to cut the query line by the split plane into two sub-lines, because that often
        // makes both sub-lines shorter also along the remaining two axes (which in turn leads to
        // more efficient query - more nodes NOT explored).

        vector3  plane_point;
        split_node* first_node;
        split_node* second_node;
        {
            float_32_bit const  dist = line_end(coord_idx) - line_begin(coord_idx);
            if (std::fabs(dist) > 0.0001f)
            {
                float_32_bit const  param = (node_ptr->m_spit_plane_origin(coord_idx) - line_begin(coord_idx)) / dist;
                plane_point = line_begin + param * (line_end - line_begin);
            }
            else
                plane_point = line_begin;
            if (dist >= 0.0f)
            {
                first_node = node_ptr->m_back_child_node.get();
                second_node = node_ptr->m_front_child_node.get();
            }
            else
            {
                first_node = node_ptr->m_front_child_node.get();
                second_node = node_ptr->m_back_child_node.get();
            }
        }

        if (find_by_line(first_node, line_begin, plane_point, output_collector) == false)
            return false;
        return find_by_line(second_node, plane_point, line_end, output_collector);
    }
}


template<typename  object_type__>
void  proximity_map<object_type__>::enumerate(std::function<bool(object_type, natural_32_bit)> const&  output_collector)
{
    TMPROF_BLOCK();

    natural_32_bit  leaf_node_index = 0U;
    enumerate(m_root.get(), leaf_node_index, output_collector);
}


template<typename  object_type__>
bool  proximity_map<object_type__>::enumerate(
        split_node* const  node_ptr,
        natural_32_bit&  output_leaf_node_index,
        std::function<bool(object_type, natural_32_bit)> const&  output_collector
        )
{
    if (node_ptr->m_split_plane_normal_direction == split_node::SPLIT_PLANE_NORMAL_DIRECTION::NOT_SET)
    {
        for (object_type  object : *node_ptr->m_objects)
            if (output_collector(object, output_leaf_node_index) == false)
                return false;
        ++output_leaf_node_index;
        return true;
    }

    if (enumerate(node_ptr->m_front_child_node.get(), output_leaf_node_index, output_collector) == false)
        return false;
    return enumerate(node_ptr->m_back_child_node.get(), output_leaf_node_index, output_collector);
}


template<typename  object_type__>
void  proximity_map<object_type__>::apply_node_split(split_node* const  node_ptr)
{
    TMPROF_BLOCK();

    std::unique_ptr<std::unordered_set<object_type> > const  objects(node_ptr->m_objects.release());
    node_ptr->m_num_objects = 0U;
    node_ptr->m_front_child_node.reset(new split_node);
    node_ptr->m_back_child_node.reset(new split_node);

    std::vector<vector3>  object_centers(objects->size());
    for (object_type object : *objects)
    {
        object_centers.push_back(0.5f * (m_get_bbox_min_corner(object) + m_get_bbox_max_corner(object)));
        node_ptr->m_spit_plane_origin += object_centers.back();
    }
    node_ptr->m_spit_plane_origin /= (float_32_bit)objects->size();

    vector3  directions = vector3_zero();
    for (vector3 const&  object_centre : object_centers)
        for (int i = 0; i != 3; ++i)
            directions(i) += std::fabs(object_centre(i) - node_ptr->m_spit_plane_origin(i));
    if (directions(0) > directions(1))
        if (directions(0) > directions(2))
            node_ptr->m_split_plane_normal_direction = split_node::SPLIT_PLANE_NORMAL_DIRECTION::X_AXIS;
        else
            node_ptr->m_split_plane_normal_direction = split_node::SPLIT_PLANE_NORMAL_DIRECTION::Z_AXIS;
    else
        if (directions(1) > directions(2))
            node_ptr->m_split_plane_normal_direction = split_node::SPLIT_PLANE_NORMAL_DIRECTION::Y_AXIS;
        else
            node_ptr->m_split_plane_normal_direction = split_node::SPLIT_PLANE_NORMAL_DIRECTION::Z_AXIS;
    
    for (object_type object : *objects)
        insert(node_ptr, object, m_get_bbox_min_corner(object), m_get_bbox_max_corner(object));
}


template<typename  object_type__>
void  proximity_map<object_type__>::apply_node_merge(split_node* const  node_ptr)
{
    TMPROF_BLOCK();

    std::array<std::unique_ptr<std::unordered_set<object_type> >, 2U> const  objects
    {
        std::unique_ptr<std::unordered_set<object_type> >(node_ptr->m_front_child_node->m_objects.release()),
        std::unique_ptr<std::unordered_set<object_type> >(node_ptr->m_back_child_node->m_objects.release())
    };
    node_ptr->m_spit_plane_origin = vector3_zero();
    node_ptr->m_split_plane_normal_direction = split_node::SPLIT_PLANE_NORMAL_DIRECTION::NOT_SET;
    node_ptr->m_num_objects = 0U;
    node_ptr->m_objects.reset(new std::unordered_set<object_type>);
    for (int i = 0; i != 2; ++i)
        for (object_type object : *objects[i])
            node_ptr->m_objects->insert(object);
}


template<typename  object_type__>
void  proximity_map<object_type__>::apply_node_rotation_from_back_to_front(
        split_node* const  node_ptr,
        split_node* const  parent_node_ptr
        )
{
    TMPROF_BLOCK();

    apply_objects_move(node_ptr->m_back_child_node.get(), node_ptr->m_front_child_node.get());

    if (parent_node_ptr == nullptr)
        m_root.reset(node_ptr->m_front_child_node.release());
    else if (parent_node_ptr->m_front_child_node.get() == node_ptr)
        parent_node_ptr->m_front_child_node.reset(node_ptr->m_front_child_node.release());
    else
        parent_node_ptr->m_back_child_node.reset(node_ptr->m_front_child_node.release());
}


template<typename  object_type__>
void  proximity_map<object_type__>::apply_node_rotation_from_front_to_back(
        split_node* const  node_ptr,
        split_node* const  parent_node_ptr
        )
{
    TMPROF_BLOCK();

    apply_objects_move(node_ptr->m_front_child_node.get(), node_ptr->m_back_child_node.get());

    if (parent_node_ptr == nullptr)
        m_root.reset(node_ptr->m_back_child_node.release());
    else if (parent_node_ptr->m_front_child_node.get() == node_ptr)
        parent_node_ptr->m_front_child_node.reset(node_ptr->m_back_child_node.release());
    else
        parent_node_ptr->m_back_child_node.reset(node_ptr->m_back_child_node.release());
}


template<typename  object_type__>
void  proximity_map<object_type__>::apply_objects_move(split_node* const  from_node_ptr, split_node* const  to_node_ptr)
{
    TMPROF_BLOCK();

    if (from_node_ptr->m_split_plane_normal_direction == split_node::SPLIT_PLANE_NORMAL_DIRECTION::NOT_SET)
    {
        for (object_type object : *from_node_ptr->m_objects)
            insert(to_node_ptr, object, m_get_bbox_min_corner(object), m_get_bbox_max_corner(object));
        return;
    }

    apply_objects_move(from_node_ptr->m_front_child_node.get(), to_node_ptr);
    apply_objects_move(from_node_ptr->m_back_child_node.get(), to_node_ptr);
}


}
/*
namespace __nouse__ {


struct X
{
    vector3 lo, hi;
};

void foo()
{
    angeo::proximity_map<X*> map([](X* x) { return x->lo; }, [](X* x) { return x->hi; });
    
    X x;
    map.insert(&x);

    map.rebalance();

    vector3 lo, hi;
    map.find_by_bbox(lo, hi, [](X*) { return true; });
    map.find_by_line(lo, hi, [](X*) { return true; });

    map.erase(&x);
    map.clear();
}


}
*/

#endif
