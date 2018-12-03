#ifndef E2_SCENE_RECORDS_BATCH_UTILS_HPP_INCLUDED
#   define E2_SCENE_RECORDS_BATCH_UTILS_HPP_INCLUDED

#   include <scene/scene.hpp>
#   include <scene/scene_utils.hpp>
#   include <qtgl/batch.hpp>

namespace scn { namespace detail {


inline scene_node::record_bbox_getter  make_batch_bbox_getter(qtgl::batch const  batch)
{
    return [batch](angeo::axis_aligned_bounding_box& out_bbox) -> void {
                if (batch.loaded_successfully())
                {
                    qtgl::spatial_boundary const&  boundary = batch.get_buffers_binding().get_boundary();
                    out_bbox.min_corner = boundary.lo_corner();
                    out_bbox.max_corner = boundary.hi_corner();
                }
                else
                {
                    out_bbox.min_corner = vector3(1e20f, 1e20f, 1e20f);
                    out_bbox.max_corner = vector3(1e20f, 1e20f, 1e20f);
                }
            };
}


}}

namespace scn {


inline scene_node::folder_name  get_batches_folder_name() { return "batches"; }

inline scene_record_id  make_batch_record_id(
        scene_node_id const&  node_id,
        scene_node::record_name const&  batch_name
        )
{
    return { node_id, get_batches_folder_name(), batch_name };
}

inline scene_node_record_id  make_batch_node_record_id(scene_node::record_name const&  batch_name)
{
    return { get_batches_folder_name(), batch_name };
}

inline scene_node::folder_content::records_map const&  get_batch_holders(scene_node const&  node)
{
    return get_folder_records_map(node, get_batches_folder_name());
}

inline qtgl::batch  as_batch(scene_node::record_holder const&  holder)
{
    return record_cast<qtgl::batch>(holder);
}

inline qtgl::batch  get_batch(scene_node const&  n, scene_node::record_name const&  batch_name)
{
    return get_record<qtgl::batch>(n, make_batch_node_record_id(batch_name));
}

inline bool  has_batch(scene_node const&  n, scene_node::record_name const&  batch_name)
{
    return has_record(n, make_batch_node_record_id(batch_name));
}

inline qtgl::batch  get_batch(scene const&  s, scene_record_id const&  id)
{
    ASSUMPTION(id.get_folder_name() == get_batches_folder_name());
    return get_record<qtgl::batch>(s, id);
}

inline bool  has_batch(scene const&  s, scene_record_id const&  id)
{
    ASSUMPTION(id.get_folder_name() == get_batches_folder_name());
    return has_record(s, id);
}

inline void  insert_batch(scene_node&  n, scene_node::record_name const&  batch_name, qtgl::batch const  batch)
{
    insert_record<qtgl::batch>(
            n,
            make_batch_node_record_id(batch_name),
            batch,
            detail::make_batch_bbox_getter(batch)
            );
}

inline void  insert_batch(scene&  s, scene_record_id const&  id, qtgl::batch const  batch)
{
    ASSUMPTION(id.get_folder_name() == get_batches_folder_name());
    insert_record<qtgl::batch>(
            s,
            id,
            batch,
            detail::make_batch_bbox_getter(batch)
            );
}

inline void  erase_batch(scene_node&  n, scene_node::record_name const&  batch_name)
{
    erase_record(n, make_batch_node_record_id(batch_name));
}

inline void  erase_batch(scene&  s, scene_record_id const&  id)
{
    ASSUMPTION(id.get_folder_name() == get_batches_folder_name());
    erase_record(s, id);
}


/// The AABB is writen to 'output_bbox' only if the return value of this function is 'true'.
inline bool  get_bbox_of_batch(qtgl::batch  batch, angeo::axis_aligned_bounding_box&  output_bbox)
{
    if (batch.loaded_successfully())
    {
        output_bbox.min_corner = batch.get_buffers_binding().get_boundary().lo_corner();
        output_bbox.max_corner = batch.get_buffers_binding().get_boundary().hi_corner();
        return true;
    }
    return false;
}


}

#endif
