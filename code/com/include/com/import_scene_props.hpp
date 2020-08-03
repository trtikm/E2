#ifndef COM_IMPORT_SCENE_PROPS_HPP_INCLUDED
#   define COM_IMPORT_SCENE_PROPS_HPP_INCLUDED

#   include <com/object_guid.hpp>
#   include <angeo/tensor_math.hpp>
#   include <string>

namespace com {


struct  import_scene_props
{
    import_scene_props();
    import_scene_props(std::string const&  import_dir_, object_guid const  folder_guid_);
    std::string  import_dir;    // absolute disk path to the dir of the imported scene.
    object_guid  folder_guid;   // under which folder to import the data
    object_guid  relocation_frame_guid; // can be invalid_object_guid()
    bool  store_in_cache;
    bool  apply_linear_velocity;
    bool  apply_angular_velocity;
    vector3  linear_velocity;   // Must be valid only if apply_linear_velocity is true
    vector3  angular_velocity;  // Must be valid only if apply_angular_velocity is true
    object_guid  motion_frame_guid; // can be invalid_object_guid(), even if any apply_*_velocity is true
};


}

#endif
