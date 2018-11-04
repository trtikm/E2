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
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <memory>
#include <QFileDialog>
#include <QDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QGroupBox>

namespace window_tabs { namespace tab_scene { namespace record_rigid_body { namespace detail {


struct  rigid_body_props_dialog : public QDialog
{
    struct  rigid_body_props
    {
        vector3  m_linear_velocity;
        vector3  m_angular_velocity;
        vector3  m_external_linear_acceleration;
        vector3  m_external_angular_acceleration;
    };

    rigid_body_props_dialog(program_window* const  wnd, struct  rigid_body_props* const  props);

    bool  ok() const { return m_ok; }

public slots:

    void  accept();
    void  reject();

private:
    program_window*  m_wnd;
    rigid_body_props*  m_props;
    bool  m_ok;

    QLineEdit*  m_widget_linear_velocity_x;
    QLineEdit*  m_widget_linear_velocity_y;
    QLineEdit*  m_widget_linear_velocity_z;

    QLineEdit*  m_widget_angular_velocity_x;
    QLineEdit*  m_widget_angular_velocity_y;
    QLineEdit*  m_widget_angular_velocity_z;

    QLineEdit*  m_widget_external_linear_acceleration_x;
    QLineEdit*  m_widget_external_linear_acceleration_y;
    QLineEdit*  m_widget_external_linear_acceleration_z;

    QLineEdit*  m_widget_external_angular_acceleration_x;
    QLineEdit*  m_widget_external_angular_acceleration_y;
    QLineEdit*  m_widget_external_angular_acceleration_z;
};


rigid_body_props_dialog::rigid_body_props_dialog(program_window* const  wnd, rigid_body_props* const  props)
    : QDialog(wnd)
    , m_wnd(wnd)
    , m_props(props)
    , m_ok(false)

    , m_widget_linear_velocity_x(new QLineEdit)
    , m_widget_linear_velocity_y(new QLineEdit)
    , m_widget_linear_velocity_z(new QLineEdit)

    , m_widget_angular_velocity_x(new QLineEdit)
    , m_widget_angular_velocity_y(new QLineEdit)
    , m_widget_angular_velocity_z(new QLineEdit)

    , m_widget_external_linear_acceleration_x(new QLineEdit)
    , m_widget_external_linear_acceleration_y(new QLineEdit)
    , m_widget_external_linear_acceleration_z(new QLineEdit)

    , m_widget_external_angular_acceleration_x(new QLineEdit)
    , m_widget_external_angular_acceleration_y(new QLineEdit)
    , m_widget_external_angular_acceleration_z(new QLineEdit)
{
    ASSUMPTION(m_props != nullptr);

    m_widget_linear_velocity_x->setText(QString::number(m_props->m_linear_velocity(0)));
    m_widget_linear_velocity_y->setText(QString::number(m_props->m_linear_velocity(1)));
    m_widget_linear_velocity_z->setText(QString::number(m_props->m_linear_velocity(2)));

    m_widget_angular_velocity_x->setText(QString::number(m_props->m_angular_velocity(0)));
    m_widget_angular_velocity_y->setText(QString::number(m_props->m_angular_velocity(1)));
    m_widget_angular_velocity_z->setText(QString::number(m_props->m_angular_velocity(2)));

    m_widget_external_linear_acceleration_x->setText(QString::number(m_props->m_external_linear_acceleration(0)));
    m_widget_external_linear_acceleration_y->setText(QString::number(m_props->m_external_linear_acceleration(1)));
    m_widget_external_linear_acceleration_z->setText(QString::number(m_props->m_external_linear_acceleration(2)));

    m_widget_external_angular_acceleration_x->setText(QString::number(m_props->m_external_angular_acceleration(0)));
    m_widget_external_angular_acceleration_y->setText(QString::number(m_props->m_external_angular_acceleration(1)));
    m_widget_external_angular_acceleration_z->setText(QString::number(m_props->m_external_angular_acceleration(2)));

    m_widget_linear_velocity_x->setToolTip("x coordinate in world space.");
    m_widget_linear_velocity_y->setToolTip("y coordinate in world space.");
    m_widget_linear_velocity_z->setToolTip("z coordinate in world space.");

    m_widget_angular_velocity_x->setToolTip("x coordinate in world space.");
    m_widget_angular_velocity_y->setToolTip("y coordinate in world space.");
    m_widget_angular_velocity_z->setToolTip("z coordinate in world space.");

    m_widget_external_linear_acceleration_x->setToolTip("x coordinate in world space.");
    m_widget_external_linear_acceleration_y->setToolTip("y coordinate in world space.");
    m_widget_external_linear_acceleration_z->setToolTip("z coordinate in world space.");

    m_widget_external_angular_acceleration_x->setToolTip("x coordinate in world space.");
    m_widget_external_angular_acceleration_y->setToolTip("y coordinate in world space.");
    m_widget_external_angular_acceleration_z->setToolTip("z coordinate in world space.");

    QVBoxLayout* const dlg_layout = new QVBoxLayout;
    {
        auto const  insert_vector_group =
            [](std::string const&  group_name, QLineEdit* const  x_edit, QLineEdit* const  y_edit, QLineEdit* const  z_edit)
                -> QWidget*
            {
                QWidget* const group = new QGroupBox(group_name.c_str());
                {
                    QHBoxLayout* const coords_layout = new QHBoxLayout;
                    {
                        coords_layout->addWidget(x_edit);
                        coords_layout->addWidget(y_edit);
                        coords_layout->addWidget(z_edit);
                    }
                    group->setLayout(coords_layout);
                }
                return group;
            };

        dlg_layout->addWidget(insert_vector_group(
                "Linear velocity [m/s]",
                m_widget_linear_velocity_x,
                m_widget_linear_velocity_y,
                m_widget_linear_velocity_z
                ));

        dlg_layout->addWidget(insert_vector_group(
                "Angular velocity [m/s]",
                m_widget_angular_velocity_x,
                m_widget_angular_velocity_y,
                m_widget_angular_velocity_z
                ));

        dlg_layout->addWidget(insert_vector_group(
                "External linear acceleration [m/(s*s)]",
                m_widget_external_linear_acceleration_x,
                m_widget_external_linear_acceleration_y,
                m_widget_external_linear_acceleration_z
                ));

        dlg_layout->addWidget(insert_vector_group(
                "External angular acceleration [m/(s*s)]",
                m_widget_external_angular_acceleration_x,
                m_widget_external_angular_acceleration_y,
                m_widget_external_angular_acceleration_z
                ));

        QHBoxLayout* const buttons_layout = new QHBoxLayout;
        {
            buttons_layout->addWidget(
                [](rigid_body_props_dialog* wnd) {
                struct OK : public QPushButton {
                    OK(rigid_body_props_dialog* wnd) : QPushButton("OK")
                    {
                        QObject::connect(this, SIGNAL(released()), wnd, SLOT(accept()));
                    }
                };
                return new OK(wnd);
            }(this)
                );
            buttons_layout->addWidget(
                [](rigid_body_props_dialog* wnd) {
                struct Close : public QPushButton {
                    Close(rigid_body_props_dialog* wnd) : QPushButton("Cancel")
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
        //dlg_layout->setAlignment(buttons_layout, Qt::Alignment(Qt::AlignmentFlag::AlignRight));
    }
    this->setLayout(dlg_layout);
    this->setWindowTitle("Rigid body");
    //this->resize(300,100);
}


void  rigid_body_props_dialog::accept()
{
    m_props->m_linear_velocity = {
            (float_32_bit)std::atof(qtgl::to_string(m_widget_linear_velocity_x->text()).c_str()),
            (float_32_bit)std::atof(qtgl::to_string(m_widget_linear_velocity_y->text()).c_str()),
            (float_32_bit)std::atof(qtgl::to_string(m_widget_linear_velocity_z->text()).c_str())
            };

    m_props->m_angular_velocity = {
            (float_32_bit)std::atof(qtgl::to_string(m_widget_angular_velocity_x->text()).c_str()),
            (float_32_bit)std::atof(qtgl::to_string(m_widget_angular_velocity_y->text()).c_str()),
            (float_32_bit)std::atof(qtgl::to_string(m_widget_angular_velocity_z->text()).c_str())
            };

    m_props->m_external_linear_acceleration = {
            (float_32_bit)std::atof(qtgl::to_string(m_widget_external_linear_acceleration_x->text()).c_str()),
            (float_32_bit)std::atof(qtgl::to_string(m_widget_external_linear_acceleration_y->text()).c_str()),
            (float_32_bit)std::atof(qtgl::to_string(m_widget_external_linear_acceleration_z->text()).c_str())
            };

    m_props->m_external_angular_acceleration = {
            (float_32_bit)std::atof(qtgl::to_string(m_widget_external_angular_acceleration_x->text()).c_str()),
            (float_32_bit)std::atof(qtgl::to_string(m_widget_external_angular_acceleration_y->text()).c_str()),
            (float_32_bit)std::atof(qtgl::to_string(m_widget_external_angular_acceleration_z->text()).c_str())
            };

    m_ok = true;

    QDialog::accept();
}

void  rigid_body_props_dialog::reject()
{
    QDialog::reject();
}


}}}}

namespace window_tabs { namespace tab_scene { namespace record_rigid_body {


void  register_record_icons(std::unordered_map<std::string, QIcon>& icons_of_records)
{
    icons_of_records.insert({
        scn::get_rigid_body_folder_name(),
        QIcon((boost::filesystem::path{get_program_options()->dataRoot()} / "shared/gfx/icons/rigid_body.png").string().c_str())
        });
}


void  register_record_undo_redo_processors(widgets* const  w)
{
    //scn::scene_history_batch_insert::set_undo_processor(
    //    [w](scn::scene_history_batch_insert const&  history_node) {
    //        INVARIANT(history_node.get_id().get_folder_name() == scn::get_rigid_body_folder_name());
    //        w->wnd()->glwindow().call_now(
    //                &simulator::erase_batch_from_scene_node,
    //                history_node.get_id().get_record_name(),
    //                history_node.get_id().get_node_name()
    //                );
    //        remove_record_from_tree_widget(w->scene_tree(), history_node.get_id());
    //    });
    //scn::scene_history_batch_insert::set_redo_processor(
    //    [w](scn::scene_history_batch_insert const&  history_node) {
    //        INVARIANT(history_node.get_id().get_folder_name() == scn::get_rigid_body_folder_name());
    //        w->wnd()->glwindow().call_now(
    //                &simulator::insert_batch_to_scene_node,
    //                history_node.get_id().get_record_name(),
    //                history_node.get_batch_pathname(),
    //                history_node.get_id().get_node_name()
    //                );
    //        insert_record_to_tree_widget(
    //                w->scene_tree(),
    //                history_node.get_id(),
    //                w->get_record_icon(scn::get_rigid_body_folder_name()),
    //                w->get_folder_icon());
    //    });
}



void  register_record_handler_for_insert_scene_record(
        std::unordered_map<std::string, std::pair<bool,
                           std::function<std::pair<std::string, std::function<void(scn::scene_record_id const&)>>
                                         (widgets*, std::string const&, std::unordered_set<std::string> const&)>> >&
                insert_record_handlers
        )
{
    insert_record_handlers.insert({
            scn::get_rigid_body_folder_name(),
            {
                false, // There cannot be mutiple records in one folder.
                [](widgets* const  w, std::string const&, std::unordered_set<std::string> const&  used_names)
                    -> std::pair<std::string, std::function<void(scn::scene_record_id const&)>> {
                        if (used_names.size() != 0UL)
                        {
                            w->wnd()->print_status_message("ERROR: A coordinate system node may contain at most one rigid body.", 10000);
                            return{ "",{} };
                        }
                        std::shared_ptr<detail::rigid_body_props_dialog::rigid_body_props>  rb_props =
                                std::make_shared<detail::rigid_body_props_dialog::rigid_body_props>();
                        rb_props->m_linear_velocity = { 0.0f, 0.0f, 0.0f };
                        rb_props->m_angular_velocity ={ 0.0f, 0.0f, 0.0f };
                        rb_props->m_external_linear_acceleration = { 0.0f, 0.0f, -9.81f };
                        rb_props->m_external_angular_acceleration ={ 0.0f, 0.0f, 0.0f };
                        detail::rigid_body_props_dialog  dlg(w->wnd(), rb_props.get());
                        dlg.exec();
                        if (!dlg.ok())
                            return{ "",{} };
                        return {
                            scn::get_rigid_body_record_name(),
                            [w, rb_props](scn::scene_record_id const&  record_id) -> void {
                                    w->wnd()->glwindow().call_now(
                                            &simulator::insert_rigid_body_to_scene_node,
                                            rb_props->m_linear_velocity,
                                            rb_props->m_angular_velocity,
                                            rb_props->m_external_linear_acceleration,
                                            rb_props->m_external_angular_acceleration,
                                            record_id.get_node_name()
                                            );
                                    //w->get_scene_history()->insert<scn::scene_history_batch_insert>(record_id, pathname, false);
                                }
                            };
                    }
                }
            });
}


void  register_record_handler_for_erase_scene_record(
        std::unordered_map<std::string, std::function<void(widgets*, scn::scene_record_id const&)>>&
                erase_record_handlers
        )
{
    erase_record_handlers.insert({
            scn::get_rigid_body_folder_name(),
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
                            &simulator::erase_rigid_body_from_scene_node,
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
    load_record_handlers.insert({
            scn::get_rigid_body_folder_name(),
            [](widgets* const  w, scn::scene_record_id const&  id, boost::property_tree::ptree const&  data) -> void {
                    w->wnd()->glwindow().call_now(
                            &simulator::load_rigid_body,
                            std::cref(data),
                            id.get_node_name()
                            );
                    insert_record_to_tree_widget(
                            w->scene_tree(),
                            id,
                            w->get_record_icon(scn::get_rigid_body_folder_name()),
                            w->get_folder_icon()
                            );
                }
            });
}


void  register_record_handler_for_save_scene_record(
        std::unordered_map<std::string, std::function<void(widgets*,
                                                           scn::scene_node_ptr,
                                                           scn::scene_node_record_id const&,
                                                           boost::property_tree::ptree&)>>&
                save_record_handlers
        )
{
    save_record_handlers.insert({
            scn::get_rigid_body_folder_name(),
            []( widgets* const  w,
                scn::scene_node_ptr const  node_ptr,
                scn::scene_node_record_id const&  id,
                boost::property_tree::ptree&  data) -> void {
                    w->wnd()->glwindow().call_now(
                            &simulator::save_rigid_body,
                            scn::get_rigid_body(*node_ptr)->id(),
                            std::ref(data)
                            );
                }
            });
}


}}}
