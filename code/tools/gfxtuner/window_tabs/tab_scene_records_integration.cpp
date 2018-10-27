#include <gfxtuner/window_tabs/tab_scene_records_integration.hpp>
#include <gfxtuner/window_tabs/tab_scene_record_batch.hpp>
#include <gfxtuner/window_tabs/tab_scene_record_collider.hpp>
#include <gfxtuner/window_tabs/tab_scene_record_rigid_body.hpp>

namespace window_tabs { namespace tab_scene {


void  register_record_icons(std::unordered_map<std::string, QIcon>& icons_of_records)
{
    record_batch::register_record_icons(icons_of_records);
    record_collider::register_record_icons(icons_of_records);
    record_rigid_body::register_record_icons(icons_of_records);
}

void  register_record_undo_redo_processors(widgets* const  w)
{
    record_batch::register_record_undo_redo_processors(w);
    record_collider::register_record_undo_redo_processors(w);
    record_rigid_body::register_record_undo_redo_processors(w);
}

void  register_record_handler_for_insert_scene_record(
        std::unordered_map<std::string,
                           std::function<std::pair<std::string, std::function<void(scn::scene_record_id const&)>>
                                         (widgets*, std::string const&, std::unordered_set<std::string> const&)> >&
                insert_record_handlers
        )
{
    record_batch::register_record_handler_for_insert_scene_record(insert_record_handlers);
    record_collider::register_record_handler_for_insert_scene_record(insert_record_handlers);
    record_rigid_body::register_record_handler_for_insert_scene_record(insert_record_handlers);
}

void  register_record_handler_for_erase_scene_record(
        std::unordered_map<std::string, std::function<void(widgets*, scn::scene_record_id const&)>>&
                erase_record_handlers
        )
{
    record_batch::register_record_handler_for_erase_scene_record(erase_record_handlers);
    record_collider::register_record_handler_for_erase_scene_record(erase_record_handlers);
    record_rigid_body::register_record_handler_for_erase_scene_record(erase_record_handlers);
}

void  register_record_handler_for_load_scene_record(
        std::unordered_map<std::string, std::function<void(widgets*,
                                                           scn::scene_record_id const&,
                                                           boost::property_tree::ptree const&)>>&
                load_record_handlers
        )
{
    record_batch::register_record_handler_for_load_scene_record(load_record_handlers);
    record_collider::register_record_handler_for_load_scene_record(load_record_handlers);
    record_rigid_body::register_record_handler_for_load_scene_record(load_record_handlers);
}


}}
