#include <com/detail/import_scene.hpp>
#include <com/simulation_context.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace com { namespace detail {


imported_scene_data::imported_scene_data(async::finalise_load_on_destroy_ptr const  finaliser)
    : m_hierarchy()
    , m_effects()
{
    boost::filesystem::path const  scene_dir = finaliser->get_key().get_unique_id();

    {
        boost::filesystem::path  pathname = scene_dir / "hierarchy.json";

        if (!boost::filesystem::is_regular_file(pathname))
            throw std::runtime_error(msgstream() << "Cannot access scene file '" << pathname << "'.");

        std::ifstream  istr(pathname.string(), std::ios_base::binary);
        if (!istr.good())
            throw std::runtime_error(msgstream() << "Cannot open the scene file '" << pathname << "'.");

        boost::property_tree::read_json(istr, m_hierarchy);
    }

    {
        boost::filesystem::path  pathname = scene_dir / "effects.json";

        if (!boost::filesystem::is_regular_file(pathname))
            throw std::runtime_error(msgstream() << "Cannot access effects file '" << pathname << "'.");

        std::ifstream  istr(pathname.string(), std::ios_base::binary);
        if (!istr.good())
            throw std::runtime_error(msgstream() << "Cannot open the effects file '" << pathname << "'.");

        boost::property_tree::ptree  ptree;
        boost::property_tree::read_json(istr, ptree);
        for (auto it = ptree.begin(); it != ptree.end(); ++it)
            m_effects.insert({it->first, it->second});
    }
}


async::key_type  imported_scene::key_from_path(boost::filesystem::path const&  path)
{
    return { "com::detail::imported_scene", boost::filesystem::absolute(path).string() };
}


gfx::effects_config  import_effects_config(boost::property_tree::ptree const&  ptree)
{
    gfx::effects_config::light_types  light_types;
    for (auto const& lt_and_tree : ptree.get_child("light_types"))
        light_types.insert((gfx::LIGHT_TYPE)lt_and_tree.second.get_value<int>());
    gfx::effects_config::lighting_data_types  lighting_data_types;
    for (auto const& ldt_and_tree : ptree.get_child("lighting_data_types"))
        lighting_data_types.insert({
            (gfx::LIGHTING_DATA_TYPE)std::atoi(ldt_and_tree.first.c_str()),
            (gfx::SHADER_DATA_INPUT_TYPE)ldt_and_tree.second.get_value<int>()
        });
    gfx::effects_config::shader_output_types  shader_output_types;
    for (auto const& sot_and_tree : ptree.get_child("shader_output_types"))
        shader_output_types.insert((gfx::SHADER_DATA_OUTPUT_TYPE)sot_and_tree.second.get_value<int>());

    return gfx::effects_config(
            nullptr,
            light_types,
            lighting_data_types,
            (gfx::SHADER_PROGRAM_TYPE)ptree.get<int>("lighting_algo_location"),
            shader_output_types,
            (gfx::FOG_TYPE)ptree.get<int>("fog_type"),
            (gfx::SHADER_PROGRAM_TYPE)ptree.get<int>("fog_algo_location")
            );
}


extern void  import_gfxtuner_scene(
        simulation_context&  ctx,
        imported_scene const  scene,
        object_guid const  under_folder_guid,
        object_guid const  relocation_frame_guid,
        vector3 const&  linear_velocity,
        vector3 const&  angular_velocity,
        object_guid const  motion_frame_guid
        );


void  import_scene(
        simulation_context&  ctx,
        imported_scene const  scene,
        object_guid const  under_folder_guid,
        object_guid const  relocation_frame_guid,
        vector3 const&  linear_velocity,
        vector3 const&  angular_velocity,
        object_guid const  motion_frame_guid
        )
{
    if (scene.hierarchy().count("@pivot") != 0UL)
    {
        import_gfxtuner_scene(ctx, scene, under_folder_guid, relocation_frame_guid,
                              linear_velocity, angular_velocity, motion_frame_guid);
        return;
    }

}


}}
