#ifndef COM_DETAIL_IMPORT_SCENE_HPP_INCLUDED
#   define COM_DETAIL_IMPORT_SCENE_HPP_INCLUDED

#   include <com/object_guid.hpp>
#   include <angeo/tensor_math.hpp>
#   include <utility/async_resource_load.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <unordered_map>
#   include <vector>
#   include <string>

namespace com {
    struct  simulation_context;
}

namespace com { namespace detail {


struct  imported_scene_data
{
    imported_scene_data(async::finalise_load_on_destroy_ptr const  finaliser);
    boost::property_tree::ptree const&  hierarchy() const { return m_hierarchy; }
    std::unordered_map<std::string, boost::property_tree::ptree> const&  effects() const { return m_effects; }
private:
    boost::property_tree::ptree  m_hierarchy;
    std::unordered_map<std::string, boost::property_tree::ptree>  m_effects;
};


struct  imported_scene : public async::resource_accessor<imported_scene_data>
{
    using  super = async::resource_accessor<imported_scene_data>;
    imported_scene() : super() {}
    imported_scene(boost::filesystem::path const&  path) : super(key_from_path(path), 1U, nullptr) {}
    boost::property_tree::ptree const&  hierarchy() const { return resource().hierarchy(); }
    std::unordered_map<std::string, boost::property_tree::ptree> const&  effects() const { return resource().effects(); }
    static async::key_type  key_from_path(boost::filesystem::path const&  path);
};


void  import_scene(
        simulation_context&  ctx,
        imported_scene const  scene,
        object_guid const  under_folder_guid,
        object_guid const  relocation_frame_guid,
        vector3 const&  linear_velocity,
        vector3 const&  angular_velocity,
        object_guid const  motion_frame_guid
        );


}}


#endif
