#ifndef ANGEO_PROXIMITY_MAP_HPP_INCLUDED
#   define ANGEO_PROXIMITY_MAP_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <angeo/collide.hpp>
#   include <utility/assumptions.hpp>
#   include <utility/invariants.hpp>
#   include <unordered_set>
#   include <functional>
#   include <memory>
#   include <vector>
#   include <array>

namespace angeo {


template<typename  object_type__>
struct proximity_map
{
    using  object_type = object_type__;

    proximity_map(
            std::function<vector3(object_type)> const&  AABB_min_corner_of_object_getter,
            std::function<vector3(object_type)> const&  AABB_max_corner_of_object_getter,
            natural_32_bit const  max_num_objects_in_leaf_before_split = 25U,
            float_32_bit const  treashold_for_applying_node_balancing_rotations = 0.5f // 0.0 - min rotate, 1.0 - max rotate
            );

    bool  insert(object_type const object);
    bool  erase(object_type const object);
    void  clear();

    void  find_by_AABB(
            vector3 const& query_AABB_min_corner,
            vector3 const& query_AABB_max_corner,
            std::function<bool(object_type)> const&  output_collector
            );

    void  find_by_line(
            vector3 const&  line_begin,
            vector3 const&  line_end,
            std::function<bool(object_type)> const&  output_collector
            );

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
    };

    static bool  insert(
            split_node*  node_ptr,
            object_type const object,
            vector3 const&  object_AABB_min_corner,
            vector3 const&  object_AABB_max_corner
            );

    static bool  erase(
            split_node*  node_ptr,
            object_type const object,
            vector3 const&  object_AABB_min_corner,
            vector3 const&  object_AABB_max_corner
            );

    void  find_by_AABB(
            split_node*  node_ptr,
            vector3 const& query_AABB_min_corner,
            vector3 const& query_AABB_max_corner,
            split_node*  parent_node_ptr,
            std::function<bool(object_type)> const&  output_collector
            );

    void  find_by_line(
            split_node* const  node_ptr,
            vector3 const&  line_begin,
            vector3 const&  line_end,
            split_node*  parent_node_ptr,
            std::function<bool(object_type)> const&  output_collector
            );

    void  apply_node_split(split_node* const  node_ptr);
    static void  apply_node_merge(split_node* const  node_ptr);

    void  apply_node_rotation_from_back_to_front(split_node* const  node_ptr, split_node* const  parent_node_ptr);
    void  apply_node_rotation_from_front_to_back(split_node* const  node_ptr, split_node* const  parent_node_ptr);
    void  apply_objects_move(split_node* const  from_node_ptr, split_node* const  to_node_ptr);

    std::function<vector3(object_type)>  m_get_AABB_min_corner_of_object;
    std::function<vector3(object_type)>  m_get_AABB_max_corner_of_object;
    natural_32_bit  m_max_num_objects_in_leaf_before_split;
    float_32_bit  m_min_ratio_for_applycation_balancing_rotation;

    std::unique_ptr<split_node>  m_root;
};


template<typename  object_type__>
proximity_map<object_type__>::proximity_map(
        std::function<vector3(object_type)> const&  AABB_min_corner_of_object_getter,
        std::function<vector3(object_type)> const&  AABB_max_corner_of_object_getter,
        natural_32_bit const  max_num_objects_in_leaf_before_split,
        float_32_bit const  treashold_for_applying_node_balancing_rotations
        )
    : m_get_AABB_min_corner_of_object(AABB_min_corner_of_object_getter)
    , m_get_AABB_max_corner_of_object(AABB_max_corner_of_object_getter)
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
{
}


template<typename  object_type__>
bool  proximity_map<object_type__>::insert(object_type const object)
{
    vector3 const  min_corner = m_get_AABB_min_corner_of_object(object);
    vector3 const  max_corner = m_get_AABB_max_corner_of_object(object);
    return insert(m_root.get(), object, min_corner, max_corner);
}


template<typename  object_type__>
bool  proximity_map<object_type__>::insert(
        split_node*  node_ptr,
        object_type const object,
        vector3 const&  object_AABB_min_corner,
        vector3 const&  object_AABB_max_corner
        )
{
    if (node_ptr->m_split_plane_normal_direction == split_node::SPLIT_PLANE_NORMAL_DIRECTION::NOT_SET)
    {
        if (node_ptr->m_objects->insert(object).second)
        {
            ++node_ptr->m_num_objects;
            return true;
        }
        return false;
    }

    int const  coord_idx = (int)node_ptr->m_split_plane_normal_direction;

    if (object_AABB_min_corner(coord_idx) > node_ptr->m_spit_plane_origin(coord_idx))
    {
        if (insert(node_ptr->m_front_child_node.get(), object, object_AABB_min_corner, object_AABB_max_corner))
        {
            ++node_ptr->m_num_objects;
            return true;
        }
        return false;
    }
    
    if (object_AABB_max_corner(coord_idx) < node_ptr->m_spit_plane_origin(coord_idx))
    {
        if (insert(node_ptr->m_back_child_node.get(), object, object_AABB_min_corner, object_AABB_max_corner))
        {
            ++node_ptr->m_num_objects;
            return true;
        }
        return false;
    }

    bool const  front_inserted =
            insert(node_ptr->m_front_child_node.get(), object, object_AABB_min_corner, object_AABB_max_corner);
    bool const  back_inserted =
            insert(node_ptr->m_back_child_node.get(), object, object_AABB_min_corner, object_AABB_max_corner);
    if (front_inserted || back_inserted)
    {
        ++node_ptr->m_num_objects;
        return true;
    }
    return false;
}


template<typename  object_type__>
bool  proximity_map<object_type__>::erase(object_type const object)
{
    vector3 const  min_corner = m_get_AABB_min_corner_of_object(object);
    vector3 const  max_corner = m_get_AABB_max_corner_of_object(object);
    return erase(m_root.get(), object, min_corner, max_corner);
}


template<typename  object_type__>
bool  proximity_map<object_type__>::erase(
        split_node*  node_ptr,
        object_type const object,
        vector3 const&  object_AABB_min_corner,
        vector3 const&  object_AABB_max_corner
        )
{
    if (node_ptr->m_split_plane_normal_direction == split_node::SPLIT_PLANE_NORMAL_DIRECTION::NOT_SET)
    {
        if (node_ptr->m_objects->erase(object) != 0U)
        {
            --node_ptr->m_num_objects;
            return true;
        }
        return false;
    }

    int const  coord_idx = (int)node_ptr->m_split_plane_normal_direction;

    if (object_AABB_min_corner(coord_idx) > node_ptr->m_spit_plane_origin(coord_idx))
    {
        if (erase(node_ptr->m_front_child_node.get(), object, object_AABB_min_corner, object_AABB_max_corner))
        {
            --node_ptr->m_num_objects;
            return true;
        }
        return false;
    }
    
    if (object_AABB_max_corner(coord_idx) < node_ptr->m_spit_plane_origin(coord_idx))
    {
        if (erase(node_ptr->m_back_child_node.get(), object, object_AABB_min_corner, object_AABB_max_corner))
        {
            --node_ptr->m_num_objects;
            return true;
        }
        return false;
    }

    bool const  front_erased =
        erase(node_ptr->m_front_child_node.get(), object, object_AABB_min_corner, object_AABB_max_corner);
    bool const  back_erased =
        erase(node_ptr->m_back_child_node.get(), object, object_AABB_min_corner, object_AABB_max_corner);
    if (front_erased || back_erased)
    {
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
void  proximity_map<object_type__>::find_by_AABB(
        vector3 const& query_AABB_min_corner,
        vector3 const& query_AABB_max_corner,
        std::function<bool(object_type)> const&  output_collector
        )
{
    find_by_AABB(m_root.get(), query_AABB_min_corner, query_AABB_max_corner, nullptr, output_collector);
}


template<typename  object_type__>
void  proximity_map<object_type__>::find_by_AABB(
        split_node*  node_ptr,
        vector3 const& query_AABB_min_corner,
        vector3 const& query_AABB_max_corner,
        split_node*  parent_node_ptr,
        std::function<bool(object_type)> const&  output_collector
        )
{
    if (node_ptr->m_split_plane_normal_direction == split_node::SPLIT_PLANE_NORMAL_DIRECTION::NOT_SET)
    {
        if (node_ptr->m_num_objects <= m_max_num_objects_in_leaf_before_split ||
            (parent_node_ptr != nullptr && parent_node_ptr->m_num_objects == node_ptr->m_num_objects))
        {
            for (object_type  object : *node_ptr->m_objects)
            {
                if (collision_bbox_bbox(
                        query_AABB_min_corner,
                        query_AABB_max_corner,
                        m_get_AABB_min_corner_of_object(object),
                        m_get_AABB_max_corner_of_object(object)
                        ))
                {
                    if (output_collector(object) == false)
                        return;
                }
            }
            return;
        }
        apply_node_split(node_ptr);
    }

    int const  coord_idx = (int)node_ptr->m_split_plane_normal_direction;

    if (query_AABB_max_corner(coord_idx) > node_ptr->m_spit_plane_origin(coord_idx))
        find_by_AABB(
                node_ptr->m_front_child_node.get(),
                query_AABB_min_corner,
                query_AABB_max_corner,
                node_ptr,
                output_collector
                );
    else if (query_AABB_min_corner(coord_idx) < node_ptr->m_spit_plane_origin(coord_idx))
        find_by_AABB(
                node_ptr->m_back_child_node.get(),
                query_AABB_min_corner,
                query_AABB_max_corner,
                node_ptr,
                output_collector
                );

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
void  proximity_map<object_type__>::find_by_line(
        vector3 const&  line_begin,
        vector3 const&  line_end,
        std::function<bool(object_type)> const&  output_collector
        )
{
    find_by_ray(m_root.get(), ray_origin, ray_unit_direction, nullptr, output_collector);
}


template<typename  object_type__>
void  proximity_map<object_type__>::find_by_line(
        split_node* const  node_ptr,
        vector3 const&  line_begin,
        vector3 const&  line_end,
        split_node*  parent_node_ptr,
        std::function<bool(object_type)> const&  output_collector
        )
{
    if (node_ptr->m_split_plane_normal_direction == split_node::SPLIT_PLANE_NORMAL_DIRECTION::NOT_SET)
    {
        if (node_ptr->m_num_objects <= m_max_num_objects_in_leaf_before_split ||
            (parent_node_ptr != nullptr && parent_node_ptr->m_num_objects == node_ptr->m_num_objects))
        {
            for (object_type  object : *node_ptr->m_objects)
            {
                if (clip_line_into_bbox(
                        line_begin,
                        line_end,
                        m_get_AABB_min_corner_of_object(object),
                        m_get_AABB_max_corner_of_object(object),
                        nullptr,
                        nullptr,
                        nullptr,
                        nullptr
                        ))
                {
                    if (output_collector(object) == false)
                        return;
                }
            }
            return;
        }
        apply_node_split(node_ptr);
    }

    int const  coord_idx = (int)node_ptr->m_split_plane_normal_direction;

    if (line_begin(coord_idx) > node_ptr->m_spit_plane_origin(coord_idx) &&
        line_end(coord_idx) > node_ptr->m_spit_plane_origin(coord_idx))
    {
        find_by_line(node_ptr->m_front_child_node.get(), line_begin, line_end, node_ptr, output_collector);
    }
    else if (line_begin(coord_idx) < node_ptr->m_spit_plane_origin(coord_idx) &&
             line_end(coord_idx) < node_ptr->m_spit_plane_origin(coord_idx))
    {
        find_by_line(node_ptr->m_back_child_node.get(), line_begin, line_end, node_ptr, output_collector);
    }
    else
    {
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

        find_by_line(first_node, line_begin, plane_point, node_ptr, output_collector);
        find_by_line(second_node, plane_point, line_end, node_ptr, output_collector);
    }

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
void  proximity_map<object_type__>::apply_node_split(split_node* const  node_ptr)
{
    std::unique_ptr<std::unordered_set<object_type> > const  objects(node_ptr->m_objects.release());
    node_ptr->m_num_objects = 0U;
    node_ptr->m_front_child_node.reset(new split_node);
    node_ptr->m_back_child_node.reset(new split_node);

    std::vector<vector3>  object_centers(objects->size());
    for (object_type object : *objects)
    {
        object_centers.push_back(0.5f * (m_get_AABB_min_corner_of_object(object) + m_get_AABB_max_corner_of_object(object)));
        node_ptr->m_spit_plane_origin += object_centers.back();
    }
    node_ptr->m_spit_plane_origin /= objects->size();

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
        insert(node_ptr, object, m_get_AABB_min_corner_of_object(object), m_get_AABB_max_corner_of_object(object));
}


template<typename  object_type__>
void  proximity_map<object_type__>::apply_node_merge(split_node* const  node_ptr)
{
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
    if (from_node_ptr->m_split_plane_normal_direction == split_node::SPLIT_PLANE_NORMAL_DIRECTION::NOT_SET)
    {
        for (object_type object : *from_node_ptr->m_objects)
            insert(to_node_ptr, object, m_get_AABB_min_corner_of_object(object), m_get_AABB_max_corner_of_object(object));
        return;
    }

    apply_objects_move(from_node_ptr->m_front_child_node.get(), to_node_ptr);
    apply_objects_move(from_node_ptr->m_back_child_node.get(), to_node_ptr);
}


}

#endif
