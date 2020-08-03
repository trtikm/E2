#include <com/import_scene_props.hpp>

namespace com {


import_scene_props::import_scene_props()
    : import_scene_props(".", invalid_object_guid())
{}


import_scene_props::import_scene_props(std::string const&  import_dir_, object_guid const  folder_guid_)
    : import_dir(import_dir_)
    , folder_guid(folder_guid_)
    , relocation_frame_guid(invalid_object_guid())
    , store_in_cache(true)
    , apply_linear_velocity(false)
    , apply_angular_velocity(false)
    , linear_velocity(vector3_zero())
    , angular_velocity(vector3_zero())
    , motion_frame_guid(invalid_object_guid())
{}


}
