#include <gfxtuner/window_tabs/tab_scene_record_batch.hpp>
#include <gfxtuner/window_tabs/tab_scene.hpp>
#include <gfxtuner/window_tabs/tab_scene_utils.hpp>
#include <gfxtuner/dialog_windows/effects_config_dialog.hpp>
#include <gfxtuner/dialog_windows/sketch_batch_dialogs.hpp>
#include <gfxtuner/program_window.hpp>
#include <gfxtuner/program_options.hpp>
#include <scene/scene.hpp>
#include <scene/scene_history.hpp>
#include <scene/scene_node_record_id.hpp>
#include <scene/scene_utils_specialised.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/canonical_path.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string.hpp>

namespace window_tabs { namespace tab_scene { namespace record_batch { namespace detail {


std::string  update_sketch_batch_props(widgets* const  w, dialog_windows::sketch_batch_props_dialog::sketch_batch_props&  props)
{
    std::string  sketch_id;
    dialog_windows::sketch_batch_props_dialog  props_dlg(w->wnd(), &props);
    props_dlg.exec();
    if (!props_dlg.ok())
        return sketch_id;
    if (props.m_kind == qtgl::sketch_kind_box())
        sketch_id = qtgl::make_box_id_without_prefix(
                props.m_box_half_sizes_along_axes,
                props.m_colour, props.m_fog_type, props.m_wireframe);
    else if (props.m_kind == qtgl::sketch_kind_capsule())
        sketch_id = qtgl::make_capsule_id_without_prefix(
                props.m_capsule_half_distance_between_end_points,
                props.m_capsule_thickness_from_central_line,
                props.m_num_lines_per_quarter_of_circle,
                props.m_colour, props.m_fog_type, props.m_wireframe);
    else if (props.m_kind == qtgl::sketch_kind_sphere())
        sketch_id = qtgl::make_sphere_id_without_prefix(
                props.m_sphere_radius,
                props.m_num_lines_per_quarter_of_circle,
                props.m_colour, props.m_fog_type, props.m_wireframe);
    else if (props.m_kind == qtgl::sketch_kind_mesh())
        return { "", {} }; // TODO!
    else if (props.m_kind == qtgl::sketch_kind_convex_hull())
        return { "", {} }; // TODO!
    else
        UNREACHABLE();
    return qtgl::get_sketch_id_prefix() + sketch_id;
}


}}}}

namespace window_tabs { namespace tab_scene { namespace record_batch {


void  register_record_icons(std::unordered_map<std::string, QIcon>& icons_of_records)
{
    icons_of_records.insert({
        scn::get_batches_folder_name(),
        QIcon((boost::filesystem::path{get_program_options()->dataRoot()} / "shared/gfx/icons/batch.png").string().c_str())
        });
}


void  register_record_undo_redo_processors(widgets* const  w)
{
    scn::scene_history_batch_insert::set_undo_processor(
        [w](scn::scene_history_batch_insert const&  history_node) {
            INVARIANT(history_node.get_id().get_folder_name() == scn::get_batches_folder_name());
            w->wnd()->glwindow().call_now(
                    &simulator::erase_batch_from_scene_node,
                    std::cref(history_node.get_id().get_record_name()),
                    std::cref(history_node.get_id().get_node_id())
                    );
            remove_record_from_tree_widget(w->scene_tree(), history_node.get_id());
        });
    scn::scene_history_batch_insert::set_redo_processor(
        [w](scn::scene_history_batch_insert const&  history_node) {
            INVARIANT(history_node.get_id().get_folder_name() == scn::get_batches_folder_name());
            w->wnd()->glwindow().call_now(
                    &simulator::insert_batch_to_scene_node,
                    std::cref(history_node.get_id().get_record_name()),
                    std::cref(history_node.get_batch_pathname_or_sketch_id()),
                    std::cref(history_node.get_skin_name()),
                    history_node.get_effects_config(),
                    std::cref(history_node.get_id().get_node_id())
                    );
            insert_record_to_tree_widget(
                    w->scene_tree(),
                    history_node.get_id(),
                    w->get_record_icon(scn::get_batches_folder_name()),
                    w->get_folder_icon()
                    );
        });

    scn::scene_history_batch_update_props::set_undo_processor(
        [w](scn::scene_history_batch_update_props const&  history_node) {
            INVARIANT(history_node.get_id().get_folder_name() == scn::get_batches_folder_name());
            w->wnd()->glwindow().call_now(
                    &simulator::erase_batch_from_scene_node,
                    std::cref(history_node.get_id().get_record_name()),
                    std::cref(history_node.get_id().get_node_id())
                    );
            w->wnd()->glwindow().call_now(
                    &simulator::insert_batch_to_scene_node,
                    std::cref(history_node.get_id().get_record_name()),
                    std::cref(history_node.get_old_batch_pathname_or_sketch_id()),
                    std::cref(history_node.get_old_skin_name()),
                    history_node.get_old_effects_config(),
                    std::cref(history_node.get_id().get_node_id())
                    );
        });
    scn::scene_history_batch_update_props::set_redo_processor(
        [w](scn::scene_history_batch_update_props const&  history_node) {
            INVARIANT(history_node.get_id().get_folder_name() == scn::get_batches_folder_name());
            w->wnd()->glwindow().call_now(
                    &simulator::erase_batch_from_scene_node,
                    std::cref(history_node.get_id().get_record_name()),
                    std::cref(history_node.get_id().get_node_id())
                    );
            w->wnd()->glwindow().call_now(
                    &simulator::insert_batch_to_scene_node,
                    std::cref(history_node.get_id().get_record_name()),
                    std::cref(history_node.get_new_batch_pathname_or_sketch_id()),
                    std::cref(history_node.get_new_skin_name()),
                    history_node.get_new_effects_config(),
                    std::cref(history_node.get_id().get_node_id())
                    );
        });
}



void  register_record_handler_for_insert_scene_record(
        std::unordered_map<std::string, std::pair<bool,
                           std::function<std::pair<std::string, std::function<bool(scn::scene_record_id const&)> >
                                         (widgets*, std::string const&, std::unordered_set<std::string> const&)>> >&
                insert_record_handlers
        )
{
    insert_record_handlers.insert({
            scn::get_batches_folder_name(),
            {
                true, // Allows multiple records in the folder (i.e. to open the name-selection dialog).
                [](widgets* const  w, std::string const&  mode, std::unordered_set<std::string> const&  used_names)
                    -> std::pair<std::string, std::function<bool(scn::scene_record_id const&)>> {
                    std::string  batch_pathname_or_sketch_id;
                    std::string  batch_name;
                    if (mode == "sketch")
                    {
                        dialog_windows::sketch_batch_kind_selection_dialog  kind_dlg(w->wnd(), qtgl::sketch_kind_box());
                        kind_dlg.exec();
                        if (!kind_dlg.ok())
                            return { "", {} };
                        dialog_windows::sketch_batch_props_dialog::sketch_batch_props  props;
                        props.m_kind = kind_dlg.get_kind();
                        batch_pathname_or_sketch_id = detail::update_sketch_batch_props(w, props);
                        if (batch_pathname_or_sketch_id.empty())
                            return { "", {} };
                        batch_name = "sketch_" + props.m_kind;
                    }
                    else
                    {
                        ASSUMPTION(mode == "default");
                        boost::filesystem::path const&  root_dir =
                                canonical_path(boost::filesystem::absolute(
                                    boost::filesystem::path(get_program_options()->dataRoot()) / "shared" / "gfx" / "batches"
                                    ));
                        QFileDialog  dialog(w->wnd());
                        dialog.setDirectory(root_dir.string().c_str());
                        dialog.setFileMode(QFileDialog::ExistingFile);
                        if (!dialog.exec())
                            return {"", {}};
                        QStringList const  selected = dialog.selectedFiles();
                        if (selected.size() != 1)
                        {
                            w->wnd()->print_status_message("ERROR: Exactly one file must be provided.", 10000);
                            return {"", {}};
                        }
                        boost::filesystem::path const  pathname = qtgl::to_string(selected.front());
                        std::string const  batch_dir = pathname.parent_path().filename().string();
                        if (batch_dir != "batches")
                        {
                            batch_name.append(batch_dir);
                            batch_name.push_back('.');
                        }
                        boost::filesystem::path  batch_file_name = pathname.filename();
                        if (batch_file_name.has_extension())
                            batch_file_name.replace_extension("");
                        batch_name.append(batch_file_name.string());
                        batch_pathname_or_sketch_id = pathname.string();
                    }
                    return {
                        batch_name,
                        [batch_pathname_or_sketch_id, w](scn::scene_record_id const&  record_id) -> bool {
                                w->wnd()->glwindow().call_now(
                                        &simulator::insert_batch_to_scene_node,
                                        std::cref(record_id.get_record_name()),
                                        std::cref(batch_pathname_or_sketch_id),
                                        std::cref("default"),
                                        w->wnd()->glwindow().call_now(&simulator::get_effects_config),
                                        std::cref(record_id.get_node_id())
                                        );
                                w->get_scene_history()->insert<scn::scene_history_batch_insert>(
                                        record_id,
                                        batch_pathname_or_sketch_id,
                                        "default",
                                        w->wnd()->glwindow().call_now(&simulator::get_effects_config),
                                        false
                                        );
                                return true;
                            }
                        };
                    }
                }
            });
}


void  register_record_handler_for_update_scene_record(
        std::unordered_map<std::string, std::function<void(widgets*, scn::scene_record_id const&)> >&  update_record_handlers
        )
{
    update_record_handlers.insert({
            scn::get_batches_folder_name(),
            [](widgets* const  w, scn::scene_record_id const&  record_id) -> void {
                    scn::scene_node_ptr const  src_node_ptr =
                            w->wnd()->glwindow().call_now(&simulator::get_scene_node, record_id.get_node_id());

                    std::string  old_batch_pathname_or_sketch_id;
                    qtgl::effects_config old_effects_config;
                    std::string  old_skin_name;
                    std::vector<std::string>  skin_names;
                    {
                        qtgl::batch  old_batch = scn::get_batch(*src_node_ptr, record_id.get_record_name());
                        old_batch_pathname_or_sketch_id = old_batch.get_id();
                        old_effects_config = old_batch.get_effects_config();
                        old_skin_name = old_batch.get_skin_name();
                        for (auto const& elem : old_batch.get_available_resources().skins())
                            skin_names.push_back(elem.first);
                    }

                    std::string  new_batch_pathname_or_sketch_id;
                    qtgl::effects_config  new_effects_config;
                    std::string  new_skin_name;
                    if (boost::starts_with(old_batch_pathname_or_sketch_id, qtgl::get_sketch_id_prefix()))
                    {
                        dialog_windows::sketch_batch_props_dialog::sketch_batch_props  props(old_batch_pathname_or_sketch_id);
                        new_batch_pathname_or_sketch_id = detail::update_sketch_batch_props(w, props);
                        if (new_batch_pathname_or_sketch_id.empty() ||
                                new_batch_pathname_or_sketch_id == old_batch_pathname_or_sketch_id)
                            return;
                        new_effects_config = old_effects_config;
                        new_skin_name = old_skin_name;
                    }
                    else
                    {
                        dialog_windows::effects_config_dialog  dlg(w->wnd(), old_effects_config.resource_const(), old_skin_name, skin_names);
                        dlg.exec();
                        if (!dlg.ok())
                            return;
                        new_effects_config = qtgl::effects_config::make(dlg.get_new_effects_data());
                        new_skin_name = dlg.get_new_skin_name();
                    }

                    w->get_scene_history()->insert<scn::scene_history_batch_update_props>(
                            std::cref(record_id),
                            std::cref(old_batch_pathname_or_sketch_id),
                            std::cref(old_skin_name),
                            old_effects_config,
                            std::cref(new_batch_pathname_or_sketch_id),
                            std::cref(new_skin_name),
                            new_effects_config,
                            false);
                    w->wnd()->glwindow().call_now(
                            &simulator::replace_batch_in_scene_node,
                            std::cref(record_id.get_record_name()),
                            std::cref(new_batch_pathname_or_sketch_id),
                            std::cref(new_skin_name),
                            new_effects_config,
                            std::cref(record_id.get_node_id())
                            );
                }
            });
}


void  register_record_handler_for_duplicate_scene_record(
        std::unordered_map<std::string, std::function<void(widgets*, scn::scene_record_id const&, scn::scene_record_id const&)> >&
                duplicate_record_handlers
        )
{
    duplicate_record_handlers.insert({
            scn::get_batches_folder_name(),
            [](widgets* const  w, scn::scene_record_id const&  src_record_id, scn::scene_record_id const&  dst_record_id) -> void {
                    scn::scene_node_ptr const  src_node_ptr =
                            w->wnd()->glwindow().call_now(&simulator::get_scene_node, src_record_id.get_node_id());
                    qtgl::batch const  b = scn::get_batch(*src_node_ptr, src_record_id.get_record_name());
                    w->wnd()->glwindow().call_now(
                            &simulator::insert_batch_to_scene_node,
                            std::cref(dst_record_id.get_record_name()),
                            std::cref(b.get_id()),
                            std::cref(b.get_skin_name()),
                            b.get_effects_config(),
                            std::cref(dst_record_id.get_node_id())
                            );
                    w->get_scene_history()->insert<scn::scene_history_batch_insert>(
                            dst_record_id,
                            b.get_id(),
                            b.get_skin_name(),
                            b.get_effects_config(),
                            false
                            );
                    }
            });
}


void  register_record_handler_for_erase_scene_record(
        std::unordered_map<std::string, std::function<void(widgets*, scn::scene_record_id const&)> >&
                erase_record_handlers
        )
{
    erase_record_handlers.insert({
            scn::get_batches_folder_name(),
            [](widgets* const  w, scn::scene_record_id const&  id) -> void {
                    scn::scene_node_ptr const  parent_node_ptr =
                            w->wnd()->glwindow().call_now(&simulator::get_scene_node, id.get_node_id());
                    INVARIANT(parent_node_ptr != nullptr);
                    {
                        qtgl::batch const  b = scn::get_batch(*parent_node_ptr, id.get_record_name());
                        w->get_scene_history()->insert<scn::scene_history_batch_insert>(
                                id,
                                b.get_id(),
                                b.get_skin_name(),
                                b.get_effects_config(),
                                true
                                );
                    }
                    w->wnd()->glwindow().call_now(
                            &simulator::erase_batch_from_scene_node,
                            std::cref(id.get_record_name()),
                            std::cref(id.get_node_id())
                            );
                }
            });
}


void  register_record_handler_for_load_scene_record(
        std::unordered_map<std::string, std::function<void(widgets*,
                                                           scn::scene_record_id const&,
                                                           boost::property_tree::ptree const&,
                                                           std::unordered_map<std::string, boost::property_tree::ptree> const&,
                                                           bool)> >&
                load_record_handlers
        )
{
    load_record_handlers.insert({
            scn::get_batches_folder_name(),
            []( widgets* const  w,
                scn::scene_record_id const&  id,
                boost::property_tree::ptree const&  data,
                std::unordered_map<std::string, boost::property_tree::ptree> const&  infos,
                bool const  do_update_history) -> void {
                    boost::property_tree::ptree const&  effects = infos.at("effects").get_child(data.get<std::string>("effects"));
                    qtgl::effects_config::light_types  light_types;
                    for (auto const& lt_and_tree : effects.get_child("light_types"))
                        light_types.insert((qtgl::LIGHT_TYPE)lt_and_tree.second.get_value<int>());
                    qtgl::effects_config::lighting_data_types  lighting_data_types;
                    for (auto const& ldt_and_tree : effects.get_child("lighting_data_types"))
                        lighting_data_types.insert({
                            (qtgl::LIGHTING_DATA_TYPE)std::atoi(ldt_and_tree.first.c_str()),
                            (qtgl::SHADER_DATA_INPUT_TYPE)ldt_and_tree.second.get_value<int>()
                        });
                    qtgl::effects_config::shader_output_types  shader_output_types;
                    for (auto const& sot_and_tree : effects.get_child("shader_output_types"))
                        shader_output_types.insert((qtgl::SHADER_DATA_OUTPUT_TYPE)sot_and_tree.second.get_value<int>());
                    std::string const  batch_pathname_or_sketch_id = data.get<std::string>("id");
                    std::string const  skin = data.get<std::string>("skin");
                    qtgl::effects_config const  effects_config(
                            nullptr,
                            light_types,
                            lighting_data_types,
                            (qtgl::SHADER_PROGRAM_TYPE)effects.get<int>("lighting_algo_location"),
                            shader_output_types,
                            (qtgl::FOG_TYPE)effects.get<int>("fog_type"),
                            (qtgl::SHADER_PROGRAM_TYPE)effects.get<int>("fog_algo_location")
                            );
                    w->wnd()->glwindow().call_now(
                            &simulator::insert_batch_to_scene_node,
                            std::cref(id.get_record_name()),
                            std::cref(batch_pathname_or_sketch_id),
                            std::cref(skin),
                            std::cref(effects_config),
                            std::cref(id.get_node_id())
                            );
                    insert_record_to_tree_widget(
                            w->scene_tree(),
                            id,
                            w->get_record_icon(scn::get_batches_folder_name()),
                            w->get_folder_icon()
                            );
                    if (do_update_history)
                    {
                        w->get_scene_history()->insert<scn::scene_history_batch_insert>(
                                id,
                                batch_pathname_or_sketch_id,
                                skin,
                                effects_config,
                                false
                                );
                    }
                }
            });
}


void  register_record_handler_for_save_scene_record(
        std::unordered_map<std::string, std::function<void(widgets*,
                                                           scn::scene_node_ptr,
                                                           scn::scene_node_record_id const&,
                                                           boost::property_tree::ptree&,
                                                           std::unordered_map<std::string, boost::property_tree::ptree>&)> >&
                save_record_handlers
        )
{
    save_record_handlers.insert({
            scn::get_batches_folder_name(),
            []( widgets*,
                scn::scene_node_ptr const  node_ptr,
                scn::scene_node_record_id const&  id,
                boost::property_tree::ptree&  data,
                std::unordered_map<std::string, boost::property_tree::ptree>&  infos) -> void {
                    qtgl::batch const  b = scn::get_batch(*node_ptr, id.get_record_name());
                    data.put("id", b.get_id());
                    data.put("skin", b.get_skin_name());
                    data.put("effects", b.get_effects_config().key().get_unique_id());

                    if (infos.at("effects").count(b.get_effects_config().key().get_unique_id()) == 0UL)
                    {
                        boost::property_tree::ptree  effect;
                        {
                            boost::property_tree::ptree  light_types;
                            for (auto lt : b.get_effects_config().get_light_types())
                            {
                                boost::property_tree::ptree p;
                                p.put_value((int)lt);
                                light_types.push_back(boost::property_tree::ptree::value_type("", p));
                            }
                            effect.add_child("light_types", light_types);

                            boost::property_tree::ptree  lighting_data_types;
                            for (auto const& ldt : b.get_effects_config().get_lighting_data_types())
                            {
                                boost::property_tree::ptree p;
                                lighting_data_types.put(std::to_string((int)ldt.first), (int)ldt.second);
                            }
                            effect.add_child("lighting_data_types", lighting_data_types);

                            effect.put("lighting_algo_location", (int)b.get_effects_config().get_lighting_algo_location());
                            
                            boost::property_tree::ptree  shader_output_types;
                            for (auto sot : b.get_effects_config().get_shader_output_types())
                            {
                                boost::property_tree::ptree p;
                                p.put_value((int)sot);
                                shader_output_types.push_back(boost::property_tree::ptree::value_type("", p));
                            }
                            effect.add_child("shader_output_types", shader_output_types);

                            effect.put("fog_type", (int)b.get_effects_config().get_fog_type());
                            effect.put("fog_algo_location", (int)b.get_effects_config().get_fog_algo_location());
                        }
                        infos.at("effects").add_child(b.get_effects_config().key().get_unique_id(), effect);
                    }
                }
            });
}


}}}
