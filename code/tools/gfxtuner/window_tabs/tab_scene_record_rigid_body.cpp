#include <gfxtuner/window_tabs/tab_scene_record_batch.hpp>
#include <gfxtuner/window_tabs/tab_scene.hpp>
#include <gfxtuner/window_tabs/tab_scene_utils.hpp>
#include <gfxtuner/program_window.hpp>
#include <gfxtuner/program_options.hpp>
#include <scene/scene.hpp>
#include <scene/scene_history.hpp>
#include <scene/scene_node_record_id.hpp>
#include <scene/scene_utils_specialised.hpp>
#include <scene/records/rigid_body/rigid_body.hpp>
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


bool  is_valid(matrix33 const&  M)
{
    for (int i = 0; i != 3; ++i)
        for (int j = 0; j != 3; ++j)
            if (std::isnan(M(i, j)) || std::isinf(M(i, j)))
                return false;
    return true;
}


bool  is_zero(matrix33 const&  M)
{
    for (int i = 0; i != 3; ++i)
        for (int j = 0; j != 3; ++j)
            if (std::fabsf(M(i,j)) > 0.0001f)
                return false;
    return true;
}


struct  rigid_body_props_dialog : public QDialog
{
    rigid_body_props_dialog(
            program_window* const  wnd,
            bool* const  auto_compute_mass_and_inertia_tensor,
            struct  scn::rigid_body_props* const  props
            );

    bool  ok() const { return m_ok; }

    void on_auto_compute_mass_and_inertia_tensor_changed(int const  state);

public slots:

    void  accept();
    void  reject();

private:
    program_window*  m_wnd;
    bool*  m_auto_compute_mass_and_inertia_tensor;
    scn::rigid_body_props*  m_props;
    bool  m_ok;

    void  set_enable_state_of_mass_and_inertia_tensor(bool const set_enabled);

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

    QCheckBox*  m_widget_auto_compute_mass_and_inertia_tensor;

    QLineEdit*  m_widget_inverted_mass;

    std::array<std::array<QLineEdit*, 3>, 3>  m_widget_inverted_inertia_tensor;
};


rigid_body_props_dialog::rigid_body_props_dialog(
        program_window* const  wnd,
        bool* const  auto_compute_mass_and_inertia_tensor,
        scn::rigid_body_props* const  props
        )
    : QDialog(wnd)
    , m_wnd(wnd)
    , m_auto_compute_mass_and_inertia_tensor(auto_compute_mass_and_inertia_tensor)
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

    , m_widget_auto_compute_mass_and_inertia_tensor(
        [](rigid_body_props_dialog* dlg, bool const  auto_compute_mass_and_inertia_tensor) {
            struct s : public QCheckBox {
                s(rigid_body_props_dialog* dlg, bool const  auto_compute_mass_and_inertia_tensor)
                    : QCheckBox("Compute inverted mass and inverted inertia tensor from colliders.")
                {
                    setChecked(auto_compute_mass_and_inertia_tensor);
                    QObject::connect(this, &QCheckBox::stateChanged, dlg, &rigid_body_props_dialog::on_auto_compute_mass_and_inertia_tensor_changed);
                }
            };
            return new s(dlg, auto_compute_mass_and_inertia_tensor);
        }(this, auto_compute_mass_and_inertia_tensor)
        
        )

    , m_widget_inverted_mass(new QLineEdit)

    , m_widget_inverted_inertia_tensor()
{
    ASSUMPTION(m_auto_compute_mass_and_inertia_tensor != nullptr);
    ASSUMPTION(m_props != nullptr);

    for (int i = 0; i != 3; ++i)
        for (int j = 0; j != 3; ++j)
            m_widget_inverted_inertia_tensor[i][j] = new QLineEdit;

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

    m_widget_auto_compute_mass_and_inertia_tensor->setChecked(*m_auto_compute_mass_and_inertia_tensor);

    m_widget_inverted_mass->setText(QString::number(m_props->m_mass_inverted));

    for (int i = 0; i != 3; ++i)
        for (int j = 0; j != 3; ++j)
            m_widget_inverted_inertia_tensor[i][j]->setText(QString::number(m_props->m_inertia_tensor_inverted(i, j)));

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

    m_widget_auto_compute_mass_and_inertia_tensor->setToolTip(
            "Whether to automatically compute inverted mass and inverted innertia tensor\n"
            "from colliders under the scene node with this rigid body, or to manually\n"
            "specify the mass and the inertia tensor using the widgets below."
            );

    m_widget_inverted_mass->setToolTip("Inverted mass of the rigid body.");
    for (int i = 0; i != 3; ++i)
        for (int j = 0; j != 3; ++j)
        {
            std::string const  msg = msgstream() << "Element (" << i << "," << j << ") of the inverted inertia tensor.";
            m_widget_inverted_inertia_tensor[i][j]->setToolTip(QString(msg.c_str()));
        }

    set_enable_state_of_mass_and_inertia_tensor(!*m_auto_compute_mass_and_inertia_tensor);

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

        dlg_layout->addWidget(m_widget_auto_compute_mass_and_inertia_tensor);

        QWidget* const mass_and_inertia_tensor_group = new QGroupBox("Inverted mass and inverted inertia tensor");
        {
            mass_and_inertia_tensor_group->setToolTip(
                    "In these widgets you can manually specify distribution of mass in the local\n"
                    "coordinate system (i.e. frame) of the rigid body.\n"
                    "NOTE: Insertion of mass and inertia tensor in the inverted forms has advantage\n"
                    "that you can also define unreal rigid bodies, i.e. those for which the inverted\n"
                    "forms normally do no exist. For example, rigid body with zero inverted mass\n"
                    "and zero inverted inertia tensor cannot be moved by any force or torque;\n"
                    "when all elements of the inverted inertia tensor are zero except (2,2),\n"
                    "then the rigid body can rotate only along z-axis (of its frame)."
                    );
            QVBoxLayout* const group_layout = new QVBoxLayout;
            {
                QHBoxLayout* const mass_layout = new QHBoxLayout;
                {
                    mass_layout->addWidget(new QLabel("Inverted mass [1/kg]: "));
                    mass_layout->addWidget(m_widget_inverted_mass);
                }
                group_layout->addLayout(mass_layout);

                group_layout->addWidget(new QLabel("Inverted inertia tensor (in the local frame of the ridid body):"));
                QHBoxLayout* const inertia_tensor_row_1_layout = new QHBoxLayout;
                {
                    inertia_tensor_row_1_layout->addWidget(m_widget_inverted_inertia_tensor[0][0]);
                    inertia_tensor_row_1_layout->addWidget(m_widget_inverted_inertia_tensor[0][1]);
                    inertia_tensor_row_1_layout->addWidget(m_widget_inverted_inertia_tensor[0][2]);
                }
                group_layout->addLayout(inertia_tensor_row_1_layout);
                QHBoxLayout* const inertia_tensor_row_2_layout = new QHBoxLayout;
                {
                    inertia_tensor_row_2_layout->addWidget(m_widget_inverted_inertia_tensor[1][0]);
                    inertia_tensor_row_2_layout->addWidget(m_widget_inverted_inertia_tensor[1][1]);
                    inertia_tensor_row_2_layout->addWidget(m_widget_inverted_inertia_tensor[1][2]);
                }
                group_layout->addLayout(inertia_tensor_row_2_layout);
                QHBoxLayout* const inertia_tensor_row_3_layout = new QHBoxLayout;
                {
                    inertia_tensor_row_3_layout->addWidget(m_widget_inverted_inertia_tensor[2][0]);
                    inertia_tensor_row_3_layout->addWidget(m_widget_inverted_inertia_tensor[2][1]);
                    inertia_tensor_row_3_layout->addWidget(m_widget_inverted_inertia_tensor[2][2]);
                }
                group_layout->addLayout(inertia_tensor_row_3_layout);
            }
            mass_and_inertia_tensor_group->setLayout(group_layout);
        }
        dlg_layout->addWidget(mass_and_inertia_tensor_group);

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


void rigid_body_props_dialog::on_auto_compute_mass_and_inertia_tensor_changed(int const  state)
{
    *m_auto_compute_mass_and_inertia_tensor = (state != 0);
    set_enable_state_of_mass_and_inertia_tensor(!*m_auto_compute_mass_and_inertia_tensor);
}


void  rigid_body_props_dialog::set_enable_state_of_mass_and_inertia_tensor(bool const  set_enabled)
{
    m_widget_inverted_mass->setEnabled(set_enabled);
    for (int i = 0; i != 3; ++i)
        for (int j = 0; j != 3; ++j)
            m_widget_inverted_inertia_tensor[i][j]->setEnabled(set_enabled);
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

    *m_auto_compute_mass_and_inertia_tensor = m_widget_auto_compute_mass_and_inertia_tensor->isChecked();

    if (*m_auto_compute_mass_and_inertia_tensor == false)
    {
        m_props->m_mass_inverted = (float_32_bit)std::atof(qtgl::to_string(m_widget_inverted_mass->text()).c_str());

        for (int i = 0; i != 3; ++i)
            for (int j = 0; j != 3; ++j)
                m_props->m_inertia_tensor_inverted(i,j) =
                        (float_32_bit)std::atof(qtgl::to_string(m_widget_inverted_inertia_tensor[i][j]->text()).c_str());
    }        

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
    scn::scene_history_rigid_body_insert::set_undo_processor(
        [w](scn::scene_history_rigid_body_insert const&  history_node) {
            INVARIANT(history_node.get_id().get_folder_name() == scn::get_rigid_body_folder_name());
            w->wnd()->glwindow().call_now(
                    &simulator::erase_rigid_body_from_scene_node,
                    std::cref(history_node.get_id().get_node_id())
                    );
            remove_record_from_tree_widget(w->scene_tree(), history_node.get_id());
        });
    scn::scene_history_rigid_body_insert::set_redo_processor(
        [w](scn::scene_history_rigid_body_insert const&  history_node) {
            INVARIANT(history_node.get_id().get_folder_name() == scn::get_rigid_body_folder_name());
            if (history_node.auto_compute_mass_and_inertia_tensor())
                w->wnd()->glwindow().call_now(
                        &simulator::insert_rigid_body_to_scene_node,
                        std::cref(history_node.get_props().m_linear_velocity),
                        std::cref(history_node.get_props().m_angular_velocity),
                        std::cref(history_node.get_props().m_external_linear_acceleration),
                        std::cref(history_node.get_props().m_external_angular_acceleration),
                        std::cref(history_node.get_id().get_node_id())
                        );
            else
                w->wnd()->glwindow().call_now(
                        &simulator::insert_rigid_body_to_scene_node_ex,
                        std::cref(history_node.get_props().m_linear_velocity),
                        std::cref(history_node.get_props().m_angular_velocity),
                        std::cref(history_node.get_props().m_external_linear_acceleration),
                        std::cref(history_node.get_props().m_external_angular_acceleration),
                        history_node.get_props().m_mass_inverted,
                        std::cref(history_node.get_props().m_inertia_tensor_inverted),
                        std::cref(history_node.get_id().get_node_id())
                        );
            insert_record_to_tree_widget(
                    w->scene_tree(),
                    history_node.get_id(),
                    w->get_record_icon(scn::get_rigid_body_folder_name()),
                    w->get_folder_icon());
        });


    scn::scene_history_rigid_body_update_props::set_undo_processor(
        [w](scn::scene_history_rigid_body_update_props const&  history_node) {
            INVARIANT(history_node.get_id().get_folder_name() == scn::get_rigid_body_folder_name());
            w->wnd()->glwindow().call_now(
                    &simulator::erase_rigid_body_from_scene_node,
                    std::cref(history_node.get_id().get_node_id())
                    );
            if (history_node.get_old_auto_compute_mass_and_inertia_tensor())
                w->wnd()->glwindow().call_now(
                        &simulator::insert_rigid_body_to_scene_node,
                        std::cref(history_node.get_old_props().m_linear_velocity),
                        std::cref(history_node.get_old_props().m_angular_velocity),
                        std::cref(history_node.get_old_props().m_external_linear_acceleration),
                        std::cref(history_node.get_old_props().m_external_angular_acceleration),
                        std::cref(history_node.get_id().get_node_id())
                        );
            else
                w->wnd()->glwindow().call_now(
                        &simulator::insert_rigid_body_to_scene_node_ex,
                        std::cref(history_node.get_old_props().m_linear_velocity),
                        std::cref(history_node.get_old_props().m_angular_velocity),
                        std::cref(history_node.get_old_props().m_external_linear_acceleration),
                        std::cref(history_node.get_old_props().m_external_angular_acceleration),
                        history_node.get_old_props().m_mass_inverted,
                        std::cref(history_node.get_old_props().m_inertia_tensor_inverted),
                        std::cref(history_node.get_id().get_node_id())
                        );
        });
    scn::scene_history_rigid_body_update_props::set_redo_processor(
        [w](scn::scene_history_rigid_body_update_props const&  history_node) {
            INVARIANT(history_node.get_id().get_folder_name() == scn::get_rigid_body_folder_name());
            w->wnd()->glwindow().call_now(
                    &simulator::erase_rigid_body_from_scene_node,
                    std::cref(history_node.get_id().get_node_id())
                    );
            if (history_node.get_new_auto_compute_mass_and_inertia_tensor())
                w->wnd()->glwindow().call_now(
                        &simulator::insert_rigid_body_to_scene_node,
                        std::cref(history_node.get_new_props().m_linear_velocity),
                        std::cref(history_node.get_new_props().m_angular_velocity),
                        std::cref(history_node.get_new_props().m_external_linear_acceleration),
                        std::cref(history_node.get_new_props().m_external_angular_acceleration),
                        std::cref(history_node.get_id().get_node_id())
                        );
            else
                w->wnd()->glwindow().call_now(
                        &simulator::insert_rigid_body_to_scene_node_ex,
                        std::cref(history_node.get_new_props().m_linear_velocity),
                        std::cref(history_node.get_new_props().m_angular_velocity),
                        std::cref(history_node.get_new_props().m_external_linear_acceleration),
                        std::cref(history_node.get_new_props().m_external_angular_acceleration),
                        history_node.get_new_props().m_mass_inverted,
                        std::cref(history_node.get_new_props().m_inertia_tensor_inverted),
                        std::cref(history_node.get_id().get_node_id())
                        );
        });
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
                        std::shared_ptr<scn::rigid_body_props>  rb_props = std::make_shared<scn::rigid_body_props>();
                        rb_props->m_linear_velocity = { 0.0f, 0.0f, 0.0f };
                        rb_props->m_angular_velocity ={ 0.0f, 0.0f, 0.0f };
                        rb_props->m_external_linear_acceleration = { 0.0f, 0.0f, -9.81f };
                        rb_props->m_external_angular_acceleration ={ 0.0f, 0.0f, 0.0f };
                        rb_props->m_mass_inverted = 0.0f;
                        rb_props->m_inertia_tensor_inverted = matrix33_zero();
                        bool  auto_compute_mass_and_inertia_tensor = true;
                        detail::rigid_body_props_dialog  dlg(w->wnd(), &auto_compute_mass_and_inertia_tensor, rb_props.get());
                        dlg.exec();
                        if (!dlg.ok())
                            return{ "",{} };
                        return {
                            scn::get_rigid_body_record_name(),
                            [w, auto_compute_mass_and_inertia_tensor, rb_props](scn::scene_record_id const&  record_id) -> void {
                                    if (auto_compute_mass_and_inertia_tensor)
                                        w->wnd()->glwindow().call_now(
                                                &simulator::insert_rigid_body_to_scene_node,
                                                std::cref(rb_props->m_linear_velocity),
                                                std::cref(rb_props->m_angular_velocity),
                                                std::cref(rb_props->m_external_linear_acceleration),
                                                std::cref(rb_props->m_external_angular_acceleration),
                                                std::cref(record_id.get_node_id())
                                                );
                                    else
                                        w->wnd()->glwindow().call_now(
                                                &simulator::insert_rigid_body_to_scene_node_ex,
                                                std::cref(rb_props->m_linear_velocity),
                                                std::cref(rb_props->m_angular_velocity),
                                                std::cref(rb_props->m_external_linear_acceleration),
                                                std::cref(rb_props->m_external_angular_acceleration),
                                                rb_props->m_mass_inverted,
                                                std::cref(rb_props->m_inertia_tensor_inverted),
                                                std::cref(record_id.get_node_id())
                                                );
                                    w->get_scene_history()->insert<scn::scene_history_rigid_body_insert>(
                                            record_id,
                                            auto_compute_mass_and_inertia_tensor,
                                            *rb_props,
                                            false
                                            );
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
            scn::get_rigid_body_folder_name(),
            [](widgets* const  w, scn::scene_record_id const&  record_id) -> void {
                    bool  auto_compute_mass_and_inertia_tensor;
                    scn::rigid_body_props  rb_props;
                    w->wnd()->glwindow().call_now(
                            &simulator::get_rigid_body_info,
                            std::cref(record_id.get_node_id()),
                            std::ref(auto_compute_mass_and_inertia_tensor),
                            std::ref(rb_props)
                            );
                    bool const  old_auto_compute_mass_and_inertia_tensor = auto_compute_mass_and_inertia_tensor;
                    scn::rigid_body_props  rb_old_props = rb_props;
                    detail::rigid_body_props_dialog  dlg(w->wnd(), &auto_compute_mass_and_inertia_tensor, &rb_props);
                    dlg.exec();
                    if (!dlg.ok())
                        return;
                    w->get_scene_history()->insert<scn::scene_history_rigid_body_update_props>(
                            record_id,
                            old_auto_compute_mass_and_inertia_tensor,
                            rb_old_props,
                            auto_compute_mass_and_inertia_tensor,
                            rb_props,
                            false);
                    w->wnd()->glwindow().call_now(
                            &simulator::erase_rigid_body_from_scene_node,
                            std::cref(record_id.get_node_id())
                            );
                    if (auto_compute_mass_and_inertia_tensor)
                        w->wnd()->glwindow().call_now(
                                &simulator::insert_rigid_body_to_scene_node,
                                std::cref(rb_props.m_linear_velocity),
                                std::cref(rb_props.m_angular_velocity),
                                std::cref(rb_props.m_external_linear_acceleration),
                                std::cref(rb_props.m_external_angular_acceleration),
                                std::cref(record_id.get_node_id())
                                );
                    else
                        w->wnd()->glwindow().call_now(
                                &simulator::insert_rigid_body_to_scene_node_ex,
                                std::cref(rb_props.m_linear_velocity),
                                std::cref(rb_props.m_angular_velocity),
                                std::cref(rb_props.m_external_linear_acceleration),
                                std::cref(rb_props.m_external_angular_acceleration),
                                std::cref(rb_props.m_mass_inverted),
                                std::cref(rb_props.m_inertia_tensor_inverted),
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
            scn::get_rigid_body_folder_name(),
            [](widgets* const  w, scn::scene_record_id const&  src_record_id, scn::scene_record_id const&  dst_record_id) -> void {
                    bool  auto_compute_mass_and_inertia_tensor;
                    scn::rigid_body_props  rb_props;
                    w->wnd()->glwindow().call_now(
                            &simulator::get_rigid_body_info,
                            std::cref(src_record_id.get_node_id()),
                            std::ref(auto_compute_mass_and_inertia_tensor),
                            std::ref(rb_props)
                            );
                    if (auto_compute_mass_and_inertia_tensor)
                        w->wnd()->glwindow().call_now(
                                &simulator::insert_rigid_body_to_scene_node,
                                std::cref(rb_props.m_linear_velocity),
                                std::cref(rb_props.m_angular_velocity),
                                std::cref(rb_props.m_external_linear_acceleration),
                                std::cref(rb_props.m_external_angular_acceleration),
                                std::cref(dst_record_id.get_node_id())
                                );
                    else
                        w->wnd()->glwindow().call_now(
                                &simulator::insert_rigid_body_to_scene_node_ex,
                                std::cref(rb_props.m_linear_velocity),
                                std::cref(rb_props.m_angular_velocity),
                                std::cref(rb_props.m_external_linear_acceleration),
                                std::cref(rb_props.m_external_angular_acceleration),
                                std::cref(rb_props.m_mass_inverted),
                                std::cref(rb_props.m_inertia_tensor_inverted),
                                std::cref(dst_record_id.get_node_id())
                                );
                    w->get_scene_history()->insert<scn::scene_history_rigid_body_insert>(
                            dst_record_id,
                            auto_compute_mass_and_inertia_tensor,
                            rb_props,
                            false
                            );
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
                    bool  auto_compute_mass_and_inertia_tensor;
                    scn::rigid_body_props  rb_props;
                    w->wnd()->glwindow().call_now(
                            &simulator::get_rigid_body_info,
                            std::cref(id.get_node_id()),
                            std::ref(auto_compute_mass_and_inertia_tensor),
                            std::ref(rb_props)
                            );
                    w->get_scene_history()->insert<scn::scene_history_rigid_body_insert>(
                            id,
                            auto_compute_mass_and_inertia_tensor,
                            rb_props,
                            true
                            );
                    w->wnd()->glwindow().call_now(
                            &simulator::erase_rigid_body_from_scene_node,
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
                                                           bool)>>&
                load_record_handlers
        )
{
    load_record_handlers.insert({
            scn::get_rigid_body_folder_name(),
            []( widgets* const  w,
                scn::scene_record_id const&  id,
                boost::property_tree::ptree const&  data,
                std::unordered_map<std::string, boost::property_tree::ptree> const&  infos,
                bool const  do_update_history) -> void {
                    w->wnd()->glwindow().call_now(
                            &simulator::load_rigid_body,
                            std::cref(data),
                            std::cref(id.get_node_id())
                            );
                    insert_record_to_tree_widget(
                            w->scene_tree(),
                            id,
                            w->get_record_icon(scn::get_rigid_body_folder_name()),
                            w->get_folder_icon()
                            );
                    if (do_update_history)
                    {
                        bool  auto_compute_mass_and_inertia_tensor;
                        scn::rigid_body_props  rb_props;
                        w->wnd()->glwindow().call_now(
                                &simulator::get_rigid_body_info,
                                std::cref(id.get_node_id()),
                                std::ref(auto_compute_mass_and_inertia_tensor),
                                std::ref(rb_props)
                                );
                        w->get_scene_history()->insert<scn::scene_history_rigid_body_insert>(
                                id,
                                auto_compute_mass_and_inertia_tensor,
                                rb_props,
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
                                                           std::unordered_map<std::string, boost::property_tree::ptree>&)>>&
                save_record_handlers
        )
{
    save_record_handlers.insert({
            scn::get_rigid_body_folder_name(),
            []( widgets* const  w,
                scn::scene_node_ptr const  node_ptr,
                scn::scene_node_record_id const&  id,
                boost::property_tree::ptree&  data,
                std::unordered_map<std::string, boost::property_tree::ptree>&  infos) -> void {
                    w->wnd()->glwindow().call_now(
                            &simulator::save_rigid_body,
                            node_ptr->get_id(),
                            std::ref(data)
                            );
                }
            });
}


}}}
