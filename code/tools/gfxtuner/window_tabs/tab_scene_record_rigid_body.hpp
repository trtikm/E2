#ifndef ANGEO_COLLISION_SCENE_RECORD_RIGID_BODY_HPP_INCLUDED
#   define ANGEO_COLLISION_SCENE_RECORD_RIGID_BODY_HPP_INCLUDED

#   include <scene/scene_record_id.hpp>
#   include <scene/scene_node_record_id.hpp>
#   include <unordered_map>
#   include <unordered_set>
#   include <functional>
#   include <tuple>
#   include <string>
#   include <QIcon>
#   include <boost/property_tree/info_parser.hpp>

namespace window_tabs { namespace tab_scene {

struct  widgets;

}}

namespace window_tabs { namespace tab_scene { namespace record_rigid_body {


void  register_record_icons(std::unordered_map<std::string, QIcon>& icons_of_records);

void  register_record_undo_redo_processors(widgets* const  w);

void  register_record_handler_for_insert_scene_record(
        std::unordered_map<std::string,
                           std::function<std::pair<std::string, std::function<void(scn::scene_record_id const&)>>
                                         (widgets*, std::string const&, std::unordered_set<std::string> const&)> >&
                insert_record_handlers
        );

void  register_record_handler_for_erase_scene_record(
        std::unordered_map<std::string, std::function<void(widgets*, scn::scene_record_id const&)>>&
                erase_record_handlers
        );

void  register_record_handler_for_load_scene_record(
        std::unordered_map<std::string, std::function<void(widgets*,
                                                           scn::scene_record_id const&,
                                                           boost::property_tree::ptree const&)>>&
                load_record_handlers
        );

void  register_record_handler_for_save_scene_record(
        std::unordered_map<std::string, std::function<void(widgets*,
                                                           scn::scene_node_ptr,
                                                           scn::scene_node_record_id const&,
                                                           boost::property_tree::ptree&)>>&
                save_record_handlers
        );


}}}

#endif