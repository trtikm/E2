#include <scene/scene_utils.hpp>
#include <scene/scene_selection.hpp>
#include <angeo/collide.hpp>
#include <utility/timeprof.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>

namespace scn { namespace detail {


scene_node::record_holder const*  get_record_holder_ptr(scene_node const&  n, scene_node_record_id const&  id)
{
    scene_node::folder_content::records_map const&  folder_ref = get_folder_records_map(n, id.get_folder_name());
    auto const  holder_it = folder_ref.find(id.get_record_name());
    return (holder_it == folder_ref.cend()) ? nullptr : &holder_it->second;
}


scene_node::record_holder const*  get_record_holder_ptr(scene const&  s, scene_record_id const&  id)
{
    scene_node_ptr const  node_ptr = get_node(s, id.get_node_id());
    return (node_ptr == nullptr) ? nullptr : get_record_holder_ptr(*node_ptr, { id.get_folder_name(), id.get_record_name() });
}


scene_node::record_bbox_getter const*  get_record_bbox_getter_ptr(scene_node const&  n, scene_node_record_id const&  id)
{
    scene_node::folder_content::bbox_getters_map const&  folder_ref = get_folder_bbox_getters_map(n, id.get_folder_name());
    auto const  getter_it = folder_ref.find(id.get_record_name());
    return (getter_it == folder_ref.cend()) ? nullptr : &getter_it->second;
}


scene_node::record_bbox_getter const*  get_record_bbox_getter_ptr(scene const&  s, scene_record_id const&  id)
{
    scene_node_ptr const  node_ptr = get_node(s, id.get_node_id());
    return (node_ptr == nullptr) ? nullptr : get_record_bbox_getter_ptr(*node_ptr, { id.get_folder_name(), id.get_record_name() });
}


bool  get_bbox(scene_node::record_bbox_getter const* const  bbox_getter_ptr, angeo::axis_aligned_bounding_box&  out_bbox)
{
    if (bbox_getter_ptr == nullptr)
        return false;
    (*bbox_getter_ptr)(out_bbox);
    return true;
}


}}

namespace scn {


scene_node::folder_content::records_map const&  get_folder_records_map(
        scene_node const&  node,
        scene_node::folder_name const&  folder_name
        )
{
    static scene_node::folder_content::records_map  dummy;
    auto const  folder_it = node.find_folder(folder_name);
    return (folder_it == node.get_folders().cend()) ? dummy : folder_it->second.get_records();
}


scene_node::folder_content::bbox_getters_map const&  get_folder_bbox_getters_map(
        scene_node const&  node,
        scene_node::folder_name const&  folder_name
        )
{
    static scene_node::folder_content::bbox_getters_map  dummy;
    auto const  folder_it = node.find_folder(folder_name);
    return (folder_it == node.get_folders().cend()) ? dummy : folder_it->second.get_bbox_getters();
}


void  transform_origin_and_orientation(
        matrix44 const  transformation,
        vector3&  origin,
        quaternion&  orientation
        )
{
    origin = transform_point(origin, transformation);
    orientation = transform(orientation, transformation);
}


vector3  get_center_of_scene_nodes(std::unordered_set<scene_node_ptr> const&  nodes)
{
    vector3  center = vector3_zero();
    for (auto const& node : nodes)
        center += transform_point(vector3_zero(), node->get_world_matrix());
    return center / scalar(nodes.size());
}


bool  collision_scene_node_vs_line(
        scene_node const&  node,
        vector3 const&  line_start_point,
        vector3 const&  line_end_point,
        scalar* const  output_param_on_line
        )
{
    angeo::axis_aligned_bounding_box  bbox_in_local_space;
    get_bbox(node, bbox_in_local_space);
    angeo::axis_aligned_bounding_box  bbox_in_world_space;
    angeo::transform_bbox(bbox_in_local_space, node.get_world_matrix(), bbox_in_world_space);
    return angeo::clip_line_into_bbox(
                line_start_point,
                line_end_point,
                bbox_in_world_space.min_corner,
                bbox_in_world_space.max_corner,
                nullptr,
                nullptr,
                output_param_on_line,
                nullptr
                );
}


bool  collision_scene_record_vs_line(
        scene_node const&  node,
        scene_node_record_id const&  id,
        vector3 const&  line_start_point,
        vector3 const&  line_end_point,
        scalar* const  output_param_on_line
        )
{
    if (!has_bbox(node, id))
        return false;
    angeo::axis_aligned_bounding_box  bbox_in_local_space;
    get_bbox(node, id, bbox_in_local_space);
    angeo::axis_aligned_bounding_box  bbox_in_world_space;
    angeo::transform_bbox(bbox_in_local_space, node.get_world_matrix(), bbox_in_world_space);
    return angeo::clip_line_into_bbox(
                line_start_point,
                line_end_point,
                bbox_in_world_space.min_corner,
                bbox_in_world_space.max_corner,
                nullptr,
                nullptr,
                output_param_on_line,
                nullptr
                );

}


void  collision_scene_vs_line(
        scene const&  s,
        vector3 const&  line_start_point,
        vector3 const&  line_end_point,
        std::multimap<scalar, scn::scene_node_id>&  output_nodes
        )
{
    s.foreach_node(
        [&line_start_point, &line_end_point, &output_nodes](scene_node_ptr const  node_ptr) -> bool {
                float_32_bit  winner_param = 1.0f;
                {
                    float_32_bit  param;
                    for (auto const&  name_and_folder : node_ptr->get_folders())
                        for (auto const& name_and_holder : name_and_folder.second.get_records())
                            if (collision_scene_record_vs_line(
                                    *node_ptr,
                                    { name_and_folder.first, name_and_holder.first },
                                    line_start_point,
                                    line_end_point,
                                    &param
                                    ))
                            {
                                if (param < winner_param)
                                    winner_param = param;
                            }
                    if (winner_param == 1.0f && collision_scene_node_vs_line(*node_ptr, line_start_point, line_end_point, &param))
                        winner_param = param;
                }
                if (winner_param < 1.0f)
                    output_nodes.insert({winner_param, node_ptr->get_id()});
                return true;
            },
        false
        );
}


void  collision_scene_vs_line(
        scene const&  s,
        vector3 const&  line_start_point,
        vector3 const&  line_end_point,
        std::multimap<scalar, scn::scene_node_id>* const  output_nodes_ptr,
        std::multimap<scalar, scn::scene_record_id>* const  output_records_ptr
        )
{
    s.foreach_node(
        [&line_start_point, &line_end_point, output_nodes_ptr, output_records_ptr](scene_node_ptr const  node_ptr) -> bool {
                scalar  param;
                if (output_nodes_ptr != nullptr &&
                        collision_scene_node_vs_line(*node_ptr, line_start_point, line_end_point, &param))
                    output_nodes_ptr->insert({param, node_ptr->get_id()});
                if (output_records_ptr != nullptr)
                    for (auto const&  name_and_folder : node_ptr->get_folders())
                        for (auto const& name_and_holder : name_and_folder.second.get_records())
                            if (collision_scene_record_vs_line(
                                    *node_ptr,
                                    { name_and_folder.first, name_and_holder.first },
                                    line_start_point,
                                    line_end_point,
                                    &param
                                    ))
                                output_records_ptr->insert({
                                        param, { node_ptr->get_id(), name_and_folder.first, name_and_holder.first }
                                        });
                return true;
            },
        false
        );
}


void  collect_nearest_scene_objects_on_line_within_parameter_range(
        std::multimap<scalar, scn::scene_node_id> const* const  nodes_on_line_ptr,
        std::multimap<scalar, scn::scene_record_id> const* const  records_on_line_ptr,
        float_32_bit const  param_region_size,
        std::vector<scn::scene_record_id>&  output_nearnest_records_in_range,
        std::vector<scalar>*  output_params_of_records_in_range_ptr
        )
{
    static std::multimap<scalar, scn::scene_node_id>  dummy_nodes;
    static std::multimap<scalar, scn::scene_record_id>  dummy_records;

    std::multimap<scalar, scn::scene_node_id>::const_iterator  nodes_it =
        (nodes_on_line_ptr == nullptr) ? dummy_nodes.cbegin() : nodes_on_line_ptr->cbegin();
    std::multimap<scalar, scn::scene_node_id>::const_iterator  nodes_end =
        (nodes_on_line_ptr == nullptr) ? dummy_nodes.cend() : nodes_on_line_ptr->cend();

    std::multimap<scalar, scn::scene_record_id>::const_iterator  records_it =
        (records_on_line_ptr == nullptr) ? dummy_records.cbegin() : records_on_line_ptr->cbegin();
    std::multimap<scalar, scn::scene_record_id>::const_iterator  records_end =
        (records_on_line_ptr == nullptr) ? dummy_records.cend() : records_on_line_ptr->cend();

    std::vector<scalar>  dummy_params;
    if (output_params_of_records_in_range_ptr == nullptr)
        output_params_of_records_in_range_ptr = &dummy_params;

    auto const  insert_from_nodes =
        [&nodes_it, output_params_of_records_in_range_ptr, &output_nearnest_records_in_range, param_region_size]() -> bool {
            if (!output_params_of_records_in_range_ptr->empty() &&
                    nodes_it->first > output_params_of_records_in_range_ptr->front() + param_region_size)
                return false;
            output_params_of_records_in_range_ptr->push_back(nodes_it->first);
            output_nearnest_records_in_range.push_back(scn::scene_record_id(nodes_it->second));
            ++nodes_it;
            return true;
        };

    auto const  insert_from_records =
        [&records_it, output_params_of_records_in_range_ptr, &output_nearnest_records_in_range, param_region_size]() -> bool {
            if (!output_params_of_records_in_range_ptr->empty() &&
                    records_it->first > output_params_of_records_in_range_ptr->front() + param_region_size)
                return false;
            output_params_of_records_in_range_ptr->push_back(records_it->first);
            output_nearnest_records_in_range.push_back(records_it->second);
            ++records_it;
            return true;
        };

    bool  is_parameter_in_region = true;
    while (nodes_it != nodes_end || records_it != records_end)
    {
        if (nodes_it == nodes_end)
            is_parameter_in_region = insert_from_records();
        else if (records_it == records_end)
            is_parameter_in_region = insert_from_nodes();
        else if (nodes_it->first < records_it->first)
            is_parameter_in_region = insert_from_nodes();
        else
            is_parameter_in_region = insert_from_records();

        if (!is_parameter_in_region)
            break;
    }
}


}
