#include <gfxtuner/window_tabs/tab_scene_record_batch.hpp>
#include <gfxtuner/window_tabs/tab_scene.hpp>
#include <gfxtuner/window_tabs/tab_scene_utils.hpp>
#include <gfxtuner/program_window.hpp>
#include <gfxtuner/program_options.hpp>
#include <scene/scene.hpp>
#include <scene/scene_history.hpp>
#include <scene/scene_node_record_id.hpp>
#include <scene/scene_utils_specialised.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/canonical_path.hpp>
#include <utility/msgstream.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace window_tabs { namespace tab_scene { namespace record_collider { namespace detail {


struct  collider_props_dialog : public QDialog
{
    collider_props_dialog(
            program_window* const  wnd,
            std::string const&  shape_type
            );

    bool  ok() const { return m_ok; }

public slots:

    void  accept();
    void  reject();

private:
    program_window*  m_wnd;
    std::string  m_shape_type;
    bool  m_ok;
};


collider_props_dialog::collider_props_dialog(
        program_window* const  wnd,
        std::string const&  shape_type
        )
    : QDialog(wnd)
    , m_wnd(wnd)
    , m_shape_type(shape_type)
    , m_ok(false)
{
    QVBoxLayout* const dlg_layout = new QVBoxLayout;
    {
        QHBoxLayout* const buttons_layout = new QHBoxLayout;
        {
            buttons_layout->addWidget(
                [](collider_props_dialog* wnd) {
                    struct OK : public QPushButton {
                        OK(collider_props_dialog* wnd) : QPushButton("OK")
                        {
                            QObject::connect(this, SIGNAL(released()), wnd, SLOT(accept()));
                        }
                    };
                    return new OK(wnd);
                }(this)
                );
            buttons_layout->addWidget(
                [](collider_props_dialog* wnd) {
                    struct Close : public QPushButton {
                        Close(collider_props_dialog* wnd) : QPushButton("Cancel")
                        {
                            QObject::connect(this, SIGNAL(released()), wnd, SLOT(reject()));
                        }
                    };
                    return new Close(wnd);
                }(this)
                );
            buttons_layout->addStretch(1);
        }
        dlg_layout->addLayout(buttons_layout);
    }
    this->setLayout(dlg_layout);
    this->setWindowTitle((msgstream() << "Collider '" << m_shape_type << "' properties.").get().c_str());
    //this->resize(300,100);
}

void  collider_props_dialog::accept()
{
    m_ok = true;
    QDialog::accept();
}

void  collider_props_dialog::reject()
{
    QDialog::reject();
}


}}}}

namespace window_tabs { namespace tab_scene { namespace record_collider {


void  register_record_icons(std::unordered_map<std::string, QIcon>& icons_of_records)
{
    icons_of_records.insert({
        scn::get_collider_folder_name(),
        QIcon((boost::filesystem::path{get_program_options()->dataRoot()} / "shared/gfx/icons/collider.png").string().c_str())
        });
}


void  register_record_undo_redo_processors(widgets* const  w)
{
    //scn::scene_history_batch_insert::set_undo_processor(
    //    [w](scn::scene_history_batch_insert const&  history_node) {
    //        INVARIANT(history_node.get_id().get_folder_name() == scn::get_collider_folder_name());
    //        w->wnd()->glwindow().call_now(
    //                &simulator::erase_batch_from_scene_node,
    //                history_node.get_id().get_record_name(),
    //                history_node.get_id().get_node_name()
    //                );
    //        remove_record_from_tree_widget(w->scene_tree(), history_node.get_id());
    //    });
    //scn::scene_history_batch_insert::set_redo_processor(
    //    [w](scn::scene_history_batch_insert const&  history_node) {
    //        INVARIANT(history_node.get_id().get_folder_name() == scn::get_collider_folder_name());
    //        w->wnd()->glwindow().call_now(
    //                &simulator::insert_batch_to_scene_node,
    //                history_node.get_id().get_record_name(),
    //                history_node.get_batch_pathname(),
    //                history_node.get_id().get_node_name()
    //                );
    //        insert_record_to_tree_widget(
    //                w->scene_tree(),
    //                history_node.get_id(),
    //                w->get_record_icon(scn::get_collider_folder_name()),
    //                w->get_folder_icon());
    //    });
}



void  register_record_handler_for_insert_scene_record(
        std::unordered_map<std::string,
                           std::function<std::pair<std::string, std::function<void(scn::scene_record_id const&)>>
                                         (widgets*, std::string const&, std::unordered_set<std::string> const&)> >&
                insert_record_handlers
        )
{
    insert_record_handlers.insert({
            scn::get_collider_folder_name(),
            [](widgets* const  w, std::string const&  shape_type, std::unordered_set<std::string> const&  used_names)
                -> std::pair<std::string, std::function<void(scn::scene_record_id const&)>> {
                    if (used_names.size() != 0UL)
                    {
                        w->wnd()->print_status_message("ERROR: A coordinate system node may contain at most one collider.", 10000);
                        return {"", {}};
                    }
                    detail::collider_props_dialog  dlg(w->wnd(), shape_type);
                    dlg.exec();
                    if (!dlg.ok())
                        return{ "",{} };
                    w->wnd()->print_status_message("ERROR: Insertion of a collider is not implemented.", 10000);
                    return{ "",{} };
                    //return {
                    //    scn::get_collider_record_name(),
                    //    [w](scn::scene_record_id const&  record_id) -> void {
                    //            w->wnd()->glwindow().call_now(
                    //                    &simulator::insert_collision_sphere_to_scene_node,
                    //                    // TODO!
                    //                    record_id.get_node_name()
                    //                    );
                    //            w->get_scene_history()->insert<scn::scene_history_batch_insert>(record_id, false);
                    //        }
                    //    };
                }
            });
}


void  register_record_handler_for_erase_scene_record(
        std::unordered_map<std::string, std::function<void(widgets*, scn::scene_record_id const&)>>&
                erase_record_handlers
        )
{
    erase_record_handlers.insert({
            scn::get_collider_folder_name(),
            [](widgets* const  w, scn::scene_record_id const&  id) -> void {
                    scn::scene_node_ptr const  node_ptr =
                            w->wnd()->glwindow().call_now(&simulator::get_scene_node, id.get_node_name());
                    INVARIANT(node_ptr != nullptr);
                    //w->get_scene_history()->insert<scn::scene_history_batch_insert>(
                    //        id,
                    //        scn::get_batch(*node_ptr, id.get_record_name()).path_component_of_uid(),
                    //        true
                    //        );
                    w->wnd()->glwindow().call_now(
                            &simulator::erase_collision_object_from_scene_node,
                            id.get_node_name()
                            );
                }
            });
}


void  register_record_handler_for_load_scene_record(
        std::unordered_map<std::string, std::function<void(widgets*,
                                                           scn::scene_record_id const&,
                                                           boost::property_tree::ptree const&)>>&
                load_record_handlers
        )
{
    //load_record_handlers.insert({
    //        scn::get_collider_folder_name(),
    //        [](widgets* const  w, scn::scene_record_id const&  id, boost::property_tree::ptree const&  data) -> void {
    //                boost::filesystem::path const  pathname = data.get<std::string>("pathname");
    //                w->wnd()->glwindow().call_now(
    //                        &simulator::insert_batch_to_scene_node,
    //                        id.get_record_name(),
    //                        pathname,
    //                        id.get_node_name()
    //                        );
    //                insert_record_to_tree_widget(
    //                        w->scene_tree(),
    //                        id,
    //                        w->get_record_icon(scn::get_collider_folder_name()),
    //                        w->get_folder_icon()
    //                        );
    //            }
    //        });
}


void  register_record_handler_for_save_scene_record(
        std::unordered_map<std::string, std::function<void(widgets*,
                                                           scn::scene_node_ptr,
                                                           scn::scene_node_record_id const&,
                                                           boost::property_tree::ptree&)>>&
                save_record_handlers
        )
{
}


}}}