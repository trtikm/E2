#ifndef COM_DETAIL_IMPORT_SCENE_HPP_INCLUDED
#   define COM_DETAIL_IMPORT_SCENE_HPP_INCLUDED

#   include <com/object_guid.hpp>
#   include <com/import_scene_props.hpp>
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
private:
    boost::property_tree::ptree  m_hierarchy;
};


struct  imported_scene : public async::resource_accessor<imported_scene_data>
{
    using  super = async::resource_accessor<imported_scene_data>;
    imported_scene() : super() {}
    imported_scene(std::filesystem::path const&  path) : super(key_from_path(path), 1U, nullptr) {}
    boost::property_tree::ptree const&  hierarchy() const { return resource().hierarchy(); }
    static async::key_type  key_from_path(std::filesystem::path const&  path);
};


void  import_scene(simulation_context&  ctx, imported_scene const  scene, import_scene_props const&  props);


}}


#endif
