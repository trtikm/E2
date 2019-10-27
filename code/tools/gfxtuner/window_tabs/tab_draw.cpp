#include <gfxtuner/window_tabs/tab_draw.hpp>
#include <gfxtuner/program_window.hpp>
#include <gfxtuner/simulation/simulator_notifications.hpp>
#include <qtgl/camera_utils.hpp>
#include <boost/property_tree/ptree.hpp>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QColorDialog>
#include <QString>
#include <QIntValidator>
#include <QDoubleValidator>

namespace window_tabs { namespace tab_draw {


widgets::widgets(program_window* const  wnd)
    : m_wnd(wnd)

    , m_camera_pos_x(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setText(QString::number(wnd->ptree().get("camera.pos.x", 10.0f)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_camera_pos_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_camera_pos_y(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setText(QString::number(wnd->ptree().get("camera.pos.y", 10.0f)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_camera_pos_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_camera_pos_z(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setText(QString::number(wnd->ptree().get("camera.pos.z", 4.0f)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_camera_pos_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )

    , m_camera_rot_w(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setText(QString::number(wnd->ptree().get("camera.rot.w", 0.293152988f)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_camera_rot_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_camera_rot_x(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setText(QString::number(wnd->ptree().get("camera.rot.x", 0.245984003f)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_camera_rot_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_camera_rot_y(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setText(QString::number(wnd->ptree().get("camera.rot.y", 0.593858004f)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_camera_rot_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_camera_rot_z(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setText(QString::number(wnd->ptree().get("camera.rot.z", 0.707732975f)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_camera_rot_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )

    , m_camera_yaw(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_camera_rot_tait_bryan_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_camera_pitch(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_camera_rot_tait_bryan_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_camera_roll(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_camera_rot_tait_bryan_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )

    , m_camera_save_pos_rot(
        [](program_window* wnd) {
            struct s : public QCheckBox {
                s(program_window* wnd) : QCheckBox("Save position and rotation")
                {
                    setChecked(wnd->ptree().get("camera.save_pos_rot", true));
                    QObject::connect(this, SIGNAL(stateChanged(int)), wnd, SLOT(on_draw_save_pos_rot_changed(int)));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )

    , m_camera_far_plane(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    //setValidator(new QDoubleValidator(1.0, 5000.0, 1));
                    setText(QString::number(wnd->ptree().get("camera.far_plane", 200.0)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_camera_far_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )

    , m_camera_speed(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    //setValidator(new QDoubleValidator(0.1, 500.0, 1));
                    setText(QString::number(wnd->ptree().get("camera.speed", 15.0)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_camera_speed_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )

    , m_clear_colour_component_red(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setValidator(new QIntValidator(0, 255));
                    setText(QString::number(wnd->ptree().get("draw.clear_colour.red", 64)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_clear_colour_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_clear_colour_component_green(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setValidator(new QIntValidator(0, 255));
                    setText(QString::number(wnd->ptree().get("draw.clear_colour.green", 64)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_clear_colour_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_clear_colour_component_blue(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setValidator(new QIntValidator(0, 255));
                    setText(QString::number(wnd->ptree().get("draw.clear_colour.blue", 64)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_clear_colour_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )

    , m_show_grid(
        [](program_window* wnd) {
            struct s : public QCheckBox {
                s(program_window* wnd) : QCheckBox("Grid")
                {
                    setChecked(wnd->ptree().get("draw.show.grid", true));
                    QObject::connect(this, SIGNAL(stateChanged(int)), wnd, SLOT(on_draw_show_grid_changed(int)));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_show_batches(
        [](program_window* wnd) {
            struct s : public QCheckBox {
                s(program_window* wnd) : QCheckBox("Batches")
                {
                    setChecked(wnd->ptree().get("draw.show.batches", true));
                    QObject::connect(this, SIGNAL(stateChanged(int)), wnd, SLOT(on_draw_show_batches_changed(int)));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_show_colliders(
        [](program_window* wnd) {
            struct s : public QCheckBox {
                s(program_window* wnd) : QCheckBox("Colliders")
                {
                    setChecked(wnd->ptree().get("draw.show.colliders", true));
                    QObject::connect(this, SIGNAL(stateChanged(int)), wnd, SLOT(on_draw_show_colliders_changed(int)));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_show_contact_normals(
        [](program_window* wnd) {
            struct s : public QCheckBox {
                s(program_window* wnd) : QCheckBox("Contact normals")
                {
                    setChecked(wnd->ptree().get("draw.show.contact_normals", false));
                    QObject::connect(this, SIGNAL(stateChanged(int)), wnd, SLOT(on_draw_show_contact_normals_changed(int)));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_show_ai_action_control(
        [](program_window* wnd) {
            struct s : public QCheckBox {
                s(program_window* wnd) : QCheckBox("AI action control")
                {
                    setChecked(wnd->ptree().get("draw.show.ai_action_control", false));
                    QObject::connect(this, SIGNAL(stateChanged(int)), wnd, SLOT(on_draw_show_ai_action_control_changed(int)));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_colliders_colour_component_red(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setValidator(new QIntValidator(0, 255));
                    setText(QString::number(wnd->ptree().get("draw.colliders_colour.red", 191)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_colliders_colour_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_colliders_colour_component_green(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setValidator(new QIntValidator(0, 255));
                    setText(QString::number(wnd->ptree().get("draw.colliders_colour.green", 191)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_colliders_colour_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_colliders_colour_component_blue(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setValidator(new QIntValidator(0, 255));
                    setText(QString::number(wnd->ptree().get("draw.colliders_colour.blue", 255)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_draw_colliders_colour_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )

    , m_render_in_wireframe(
        [](program_window* wnd) {
            struct s : public QCheckBox {
                s(program_window* wnd) : QCheckBox("Wireframe")
                {
                    setChecked(wnd->ptree().get("draw.render_in_wireframe", false));
                    QObject::connect(this, SIGNAL(stateChanged(int)), wnd, SLOT(on_draw_render_in_wireframe_changed(int)));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
{
    m_camera_rot_w->setEnabled(false);
    m_camera_rot_x->setEnabled(false);
    m_camera_rot_y->setEnabled(false);
    m_camera_rot_z->setEnabled(false);
    m_camera_pitch->setEnabled(false);

    CAMERA_CONTROLLER_TYPE  camera_controller_type;
    {
        std::string const  camera_controller_type_name = wnd->ptree().get("camera.controller_type_in_simulation_mode", "FREE FLY");
        if (camera_controller_type_name == "FREE FLY") camera_controller_type = CAMERA_CONTROLLER_FREE_FLY;
        else if (camera_controller_type_name == "ORBIT") camera_controller_type = CAMERA_CONTROLLER_ORBIT;
        else if (camera_controller_type_name == "FOLLOW") camera_controller_type = CAMERA_CONTROLLER_FOLLOW;
        else if (camera_controller_type_name == "LOOK_AT") camera_controller_type = CAMERA_CONTROLLER_LOOK_AT;
        else if (camera_controller_type_name == "FOLLOW_AND_LOOK_AT") camera_controller_type = CAMERA_CONTROLLER_FOLLOW_AND_LOOK_AT;
        else UNREACHABLE();
    }
    wnd->glwindow().call_later(&simulator::set_camera_controller_type_in_simulation_mode, camera_controller_type);

    scn::scene_node_id::path_type  camera_target_node_path;
    if (wnd->ptree().get_child_optional("camera.camera_target_node_id"))
        for (auto const&  tree_node : wnd->ptree().get_child("camera.camera_target_node_id"))
            camera_target_node_path.push_back(tree_node.second.data());
    wnd->glwindow().call_later(&simulator::set_camera_target_node_id, scn::scene_node_id{camera_target_node_path});
}


void  widgets::on_camera_pos_changed()
{
    vector3 const  pos(m_camera_pos_x->text().toFloat(),
        m_camera_pos_y->text().toFloat(),
        m_camera_pos_z->text().toFloat());
    wnd()->glwindow().call_later(&simulator::set_camera_position, pos);
}

void  widgets::camera_position_listener()
{
    vector3 const pos = wnd()->glwindow().call_now(&simulator::get_camera_position);
    update_camera_pos_widgets(pos);
}

void  widgets::update_camera_pos_widgets(vector3 const&  pos)
{
    m_camera_pos_x->setText(QString::number(pos(0)));
    m_camera_pos_y->setText(QString::number(pos(1)));
    m_camera_pos_z->setText(QString::number(pos(2)));
}


void  widgets::on_camera_rot_changed()
{
    quaternion  q(m_camera_rot_w->text().toFloat(),
        m_camera_rot_x->text().toFloat(),
        m_camera_rot_y->text().toFloat(),
        m_camera_rot_z->text().toFloat());
    if (length_squared(q) < 1e-5f)
        q.z() = 1.0f;
    normalise(q);

    update_camera_rot_widgets(q);
    wnd()->glwindow().call_later(&simulator::set_camera_orientation, q);
}

void  widgets::on_camera_rot_tait_bryan_changed()
{
    quaternion  q = rotation_matrix_to_quaternion(yaw_pitch_roll_to_rotation(
        m_camera_yaw->text().toFloat() * PI() / 180.0f,
        m_camera_pitch->text().toFloat() * PI() / 180.0f,
        m_camera_roll->text().toFloat() * PI() / 180.0f
        ));
    normalise(q);
    update_camera_rot_widgets(q);
    wnd()->glwindow().call_later(&simulator::set_camera_orientation, q);
}

void  widgets::camera_rotation_listener()
{
    update_camera_rot_widgets(wnd()->glwindow().call_now(&simulator::get_camera_orientation));
}

void  widgets::update_camera_rot_widgets(quaternion const&  q)
{
    m_camera_rot_w->setText(QString::number(q.w()));
    m_camera_rot_x->setText(QString::number(q.x()));
    m_camera_rot_y->setText(QString::number(q.y()));
    m_camera_rot_z->setText(QString::number(q.z()));

    scalar  yaw, pitch, roll;
    rotation_to_yaw_pitch_roll(quaternion_to_rotation_matrix(q), yaw, pitch, roll);
    m_camera_yaw->setText(QString::number(yaw * 180.0f / PI()));
    m_camera_pitch->setText(QString::number(pitch * 180.0f / PI()));
    m_camera_roll->setText(QString::number(roll * 180.0f / PI()));
}

void  widgets::on_camera_far_changed()
{
    float_32_bit  far_plane = m_camera_far_plane->text().toFloat();
    if (far_plane < 1.0f)
    {
        far_plane = 1.0f;
        m_camera_far_plane->setText(QString("1"));
    }
    else if (far_plane > 5000.0f)
    {
        far_plane = 5000.0f;
        m_camera_far_plane->setText(QString("5000"));
    }
    wnd()->glwindow().call_later(&simulator::set_camera_far_plane, far_plane);
}

void  widgets::on_camera_speed_changed()
{
    set_camera_speed(m_camera_speed->text().toFloat());
}

void widgets::on_clear_colour_changed()
{
    vector3 const  colour(
        (float_32_bit)m_clear_colour_component_red->text().toInt() / 255.0f,
        (float_32_bit)m_clear_colour_component_green->text().toInt() / 255.0f,
        (float_32_bit)m_clear_colour_component_blue->text().toInt() / 255.0f
        );
    wnd()->glwindow().call_later(&simulator::set_clear_color, colour);
}

void widgets::on_clear_colour_set(QColor const&  colour)
{
    m_clear_colour_component_red->setText(QString::number(colour.red()));
    m_clear_colour_component_green->setText(QString::number(colour.green()));
    m_clear_colour_component_blue->setText(QString::number(colour.blue()));
    on_clear_colour_changed();
}

void widgets::on_clear_colour_choose()
{
    QColor const  init_colour(m_clear_colour_component_red->text().toInt(),
        m_clear_colour_component_green->text().toInt(),
        m_clear_colour_component_blue->text().toInt());
    QColor const  colour = QColorDialog::getColor(init_colour, wnd(), "Choose clear colour");
    if (!colour.isValid())
        return;
    m_clear_colour_component_red->setText(QString::number(colour.red()));
    m_clear_colour_component_green->setText(QString::number(colour.green()));
    m_clear_colour_component_blue->setText(QString::number(colour.blue()));
    vector3 const  normalised_colour(
        (float_32_bit)m_clear_colour_component_red->text().toInt() / 255.0f,
        (float_32_bit)m_clear_colour_component_green->text().toInt() / 255.0f,
        (float_32_bit)m_clear_colour_component_blue->text().toInt() / 255.0f
        );
    wnd()->glwindow().call_later(&simulator::set_clear_color, normalised_colour);
}

void widgets::on_clear_colour_reset()
{
    on_clear_colour_set(QColor(64, 64, 64));
}

void widgets::on_show_grid_changed(int const  value)
{
    wnd()->glwindow().call_later(&simulator::set_show_grid_state, m_show_grid->isChecked());
}

void widgets::on_show_batches_changed(int const  value)
{
    wnd()->glwindow().call_later(&simulator::set_show_batches, m_show_batches->isChecked());
}

void widgets::on_show_colliders_changed(int const  value)
{
    wnd()->glwindow().call_later(&simulator::set_show_colliders, m_show_colliders->isChecked());
}

void widgets::on_show_contact_normals_changed(int const  value)
{
    wnd()->glwindow().call_later(&simulator::set_show_contact_normals, m_show_contact_normals->isChecked());
}

void widgets::on_show_ai_action_control_changed(int const  value)
{
    wnd()->glwindow().call_later(&simulator::set_show_ai_action_controller_props, m_show_ai_action_control->isChecked());
}

void widgets::on_colliders_colour_changed()
{
    vector3 const  colour(
        (float_32_bit)m_colliders_colour_component_red->text().toInt() / 255.0f,
        (float_32_bit)m_colliders_colour_component_green->text().toInt() / 255.0f,
        (float_32_bit)m_colliders_colour_component_blue->text().toInt() / 255.0f
        );
    wnd()->glwindow().call_later(&simulator::set_colliders_color, colour);
}

void widgets::on_colliders_colour_set(QColor const&  colour)
{
    m_colliders_colour_component_red->setText(QString::number(colour.red()));
    m_colliders_colour_component_green->setText(QString::number(colour.green()));
    m_colliders_colour_component_blue->setText(QString::number(colour.blue()));
    on_colliders_colour_changed();
}

void widgets::on_colliders_colour_choose()
{
    QColor const  init_colour(m_colliders_colour_component_red->text().toInt(),
        m_colliders_colour_component_green->text().toInt(),
        m_colliders_colour_component_blue->text().toInt());
    QColor const  colour = QColorDialog::getColor(init_colour, wnd(), "Choose colliders colour");
    if (!colour.isValid())
        return;
    m_colliders_colour_component_red->setText(QString::number(colour.red()));
    m_colliders_colour_component_green->setText(QString::number(colour.green()));
    m_colliders_colour_component_blue->setText(QString::number(colour.blue()));
    vector3 const  normalised_colour(
        (float_32_bit)m_colliders_colour_component_red->text().toInt() / 255.0f,
        (float_32_bit)m_colliders_colour_component_green->text().toInt() / 255.0f,
        (float_32_bit)m_colliders_colour_component_blue->text().toInt() / 255.0f
        );
    wnd()->glwindow().call_later(&simulator::set_colliders_color, normalised_colour);
}

void widgets::on_colliders_colour_reset()
{
    on_colliders_colour_set(QColor(191, 191, 255));
}

void widgets::on_render_in_wireframe_changed(int const  value)
{
    wnd()->glwindow().call_later(&simulator::set_render_in_wireframe, m_render_in_wireframe->isChecked());
}

void  widgets::on_save_pos_rot_changed(int const  value)
{
}

void  widgets::on_double_camera_speed()
{
    set_camera_speed(m_camera_speed->text().toFloat() * 2.0f);
}

void  widgets::on_half_camera_speed()
{
    set_camera_speed(m_camera_speed->text().toFloat() / 2.0f);
}

void  widgets::on_look_at(vector3 const&  target, float_32_bit const* const  distance_ptr)
{
    angeo::coordinate_system  coord_system{
        wnd()->glwindow().call_now(&simulator::get_camera_position),
        wnd()->glwindow().call_now(&simulator::get_camera_orientation)
    };

    qtgl::look_at(coord_system, target, distance_ptr == nullptr ? length(target - coord_system.origin()) : *distance_ptr);

    wnd()->glwindow().call_later(&simulator::set_camera_position, coord_system.origin());
    wnd()->glwindow().call_later(&simulator::set_camera_orientation, coord_system.orientation());

    update_camera_pos_widgets(coord_system.origin());
    update_camera_rot_widgets(coord_system.orientation());
}

void  widgets::set_camera_speed(float_32_bit  speed)
{
    if (speed < 0.1f)
        speed = 0.1f;
    else if (speed > 500.0f)
        speed = 500.0f;

    //if (std::fabs(speed - m_camera_speed->text().toFloat()) > 1e-4f)
    {
        std::stringstream  sstr;
        sstr << speed;
        m_camera_speed->setText(QString(sstr.str().c_str()));
    }

    wnd()->glwindow().call_later(&simulator::set_camera_speed, speed);
}


void  widgets::save()
{
    if (m_camera_save_pos_rot->isChecked())
    {
        wnd()->ptree().put("camera.pos.x", m_camera_pos_x->text().toFloat());
        wnd()->ptree().put("camera.pos.y", m_camera_pos_y->text().toFloat());
        wnd()->ptree().put("camera.pos.z", m_camera_pos_z->text().toFloat());
        wnd()->ptree().put("camera.rot.w", m_camera_rot_w->text().toFloat());
        wnd()->ptree().put("camera.rot.x", m_camera_rot_x->text().toFloat());
        wnd()->ptree().put("camera.rot.y", m_camera_rot_y->text().toFloat());
        wnd()->ptree().put("camera.rot.z", m_camera_rot_z->text().toFloat());
    }
    wnd()->ptree().put("camera.save_pos_rot", m_camera_save_pos_rot->isChecked());
    wnd()->ptree().put("camera.far_plane", m_camera_far_plane->text().toFloat());
    wnd()->ptree().put("camera.speed", m_camera_speed->text().toFloat());

    std::string  camera_controller_type_name;
    switch (wnd()->glwindow().call_now(&simulator::get_camera_controller_type_in_simulation_mode))
    {
    case CAMERA_CONTROLLER_FREE_FLY: camera_controller_type_name = "FREE FLY"; break;
    case CAMERA_CONTROLLER_ORBIT: camera_controller_type_name = "ORBIT"; break;
    case CAMERA_CONTROLLER_FOLLOW: camera_controller_type_name = "FOLLOW"; break;
    case CAMERA_CONTROLLER_LOOK_AT: camera_controller_type_name = "LOOK_AT"; break;
    case CAMERA_CONTROLLER_FOLLOW_AND_LOOK_AT: camera_controller_type_name = "FOLLOW_AND_LOOK_AT"; break;
    default: UNREACHABLE(); break;
    }
    wnd()->ptree().put("camera.controller_type_in_simulation_mode", camera_controller_type_name);

    scn::scene_node_id const  camera_target_node_id = wnd()->glwindow().call_now(&simulator::get_camera_target_node_id);
    wnd()->ptree().get_child("camera").erase("camera_target_node_id");
    for (auto const&  elem : camera_target_node_id.path())
        wnd()->ptree().add("camera.camera_target_node_id.path_elem", elem);

    wnd()->ptree().put("draw.clear_colour.red", m_clear_colour_component_red->text().toInt());
    wnd()->ptree().put("draw.clear_colour.green", m_clear_colour_component_green->text().toInt());
    wnd()->ptree().put("draw.clear_colour.blue", m_clear_colour_component_blue->text().toInt());

    wnd()->ptree().put("draw.show.grid", m_show_grid->isChecked());
    wnd()->ptree().put("draw.show.batches", m_show_batches->isChecked());
    wnd()->ptree().put("draw.show.colliders", m_show_colliders->isChecked());
    wnd()->ptree().put("draw.show.contact_normals", m_show_contact_normals->isChecked());
    wnd()->ptree().put("draw.show.ai_action_control", m_show_ai_action_control->isChecked());

    wnd()->ptree().put("draw.colliders_colour.red", m_colliders_colour_component_red->text().toInt());
    wnd()->ptree().put("draw.colliders_colour.green", m_colliders_colour_component_green->text().toInt());
    wnd()->ptree().put("draw.colliders_colour.blue", m_colliders_colour_component_blue->text().toInt());

    wnd()->ptree().put("draw.render_in_wireframe", m_render_in_wireframe->isChecked());
}

QWidget*  make_draw_tab_content(widgets const&  w)
{
    QWidget* const  draw_tab = new QWidget;
    {
        QVBoxLayout* const draw_tab_layout = new QVBoxLayout;
        {
            QWidget* const camera_group = new QGroupBox("Camera");
            {
                camera_group->setToolTip(
                    "In 'edit mode', i.e. when the simulation is paused, there are\n"
                    "two camera controllers available: Free-fly, and Orbit.\n"
                    "    In Free-fly use keys 'W', 'S', 'A', 'D', 'E', and 'Q' for moving\n"
                    "camera forward,backward, left, right, up, and down, respectively.\n"
                    "Use menu 'View' to control movement speed. To rotate camera around\n"
                    "its center press and hold 'MIDDLE_MOUSE_BUTTON' and move the mouse.\n"
                    "    To switch to Orbit camera controller press and hold the key\n"
                    "'CTRL' (left or right). Now to rotate the camera around the center\n"
                    "of selected scene nodes, or '@pivot' node when nothing is selected,\n"
                    "press and hold 'MIDDLE_MOUSE_BUTTON' and move the mouse. To switch\n"
                    "back to the Free-fly controller release the key 'CTRL'.\n"
                    "In 'simulation mode', i.e. when the simulation is resumed, there is\n"
                    "currently five camera controllers available: Free-fly, Orbit, Follow,\n"
                    "Look at, and Follow & look at. You can rotate the camera controllers\n"
                    "by pressing 'MIDDLE_MOUSE_BUTTON' while holding down the key 'CTRL'\n"
                    "(left or right). When 'SHIFT' key (left or right) is also down, then\n"
                    "actibvation of the controllers rotate in the oposit direction.\n"
                    "    The Free-fly controller is used exactly the same way as in the\n"
                    "edit mode.\n"
                    "    All remaining controllers require exactly one target scene node\n"
                    "with rigid body record under the node. Use the 'edit mode' to select\n"
                    "a desired node. We emphasise that exactly one scene node (namely\n"
                    "coordinate system) must be selected. Muli-selection is not supported.\n"
                    "When no target node is selected, then only Free-fly controller is\n"
                    "available in the 'simulation mode'.\n"
                    "   The controllers Orbit and Follow are used exactly the same way.\n"
                    "Press and hold 'MIDDLE_MOUSE_BUTTON' and move the mouse.\n"
                    "   The controllers Look at and Follow & look at work without your\n"
                    "input. The camera is locked to the target scene node.\n"
                    "In both edit and simulation modes the current camera controller is\n"
                    "always shown in the status bar."
                    );
                QVBoxLayout* const camera_layout = new QVBoxLayout;
                {
                    QWidget* const position_group = new QGroupBox("Position in meters [xyz]");
                    {
                        QHBoxLayout* const position_layout = new QHBoxLayout;
                        {
                            position_layout->addWidget(w.camera_pos_x());
                            position_layout->addWidget(w.camera_pos_y());
                            position_layout->addWidget(w.camera_pos_z());
                            w.wnd()->on_draw_camera_pos_changed();
                            w.wnd()->glwindow().register_listener(
                                        simulator_notifications::camera_position_updated(),
                                        { &program_window::draw_camera_position_listener,w.wnd() }
                                        );
                        }
                        position_group->setLayout(position_layout);
                    }
                    camera_layout->addWidget(position_group);

                    QWidget* const rotation_group = new QGroupBox("Rotation");
                    {
                        QVBoxLayout* const rotation_layout = new QVBoxLayout;
                        {
                            rotation_layout->addWidget(new QLabel("Quaternion [wxyz]"));
                            QHBoxLayout* const quaternion_layout = new QHBoxLayout;
                            {
                                quaternion_layout->addWidget(w.camera_rot_w());
                                quaternion_layout->addWidget(w.camera_rot_x());
                                quaternion_layout->addWidget(w.camera_rot_y());
                                quaternion_layout->addWidget(w.camera_rot_z());
                            }
                            rotation_layout->addLayout(quaternion_layout);

                            rotation_layout->addWidget(new QLabel("Tait-Bryan angles in degrees [yaw(z)-pitch(y')-roll(x'')]"));
                            QHBoxLayout* const tait_bryan_layout = new QHBoxLayout;
                            {
                                tait_bryan_layout->addWidget(w.camera_yaw());
                                tait_bryan_layout->addWidget(w.camera_pitch());
                                tait_bryan_layout->addWidget(w.camera_roll());
                            }
                            rotation_layout->addLayout(tait_bryan_layout);
                        }
                        rotation_group->setLayout(rotation_layout);

                        w.wnd()->on_draw_camera_rot_changed();
                        w.wnd()->glwindow().register_listener(
                                    simulator_notifications::camera_orientation_updated(),
                                    { &program_window::draw_camera_rotation_listener,w.wnd() }
                                    );
                    }
                    camera_layout->addWidget(rotation_group);

                    w.camera_save_pos_rot()->setCheckState(
                        w.wnd()->ptree().get("camera.save_pos_rot", false) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked
                        );
                    camera_layout->addWidget(w.camera_save_pos_rot());

                    QHBoxLayout* const far_plane_layout = new QHBoxLayout;
                    {
                        far_plane_layout->addWidget(w.camera_far_plane());
                        far_plane_layout->addWidget(new QLabel("Far clip plane [m]"));
                        w.wnd()->on_draw_camera_far_changed();
                    }
                    camera_layout->addLayout(far_plane_layout);

                    QHBoxLayout* const speed_layout = new QHBoxLayout;
                    {
                        speed_layout->addWidget(w.camera_speed());
                        speed_layout->addWidget(new QLabel("Speed [m/s]"));
                        w.wnd()->on_draw_camera_speed_changed();
                    }
                    camera_layout->addLayout(speed_layout);
                }
                camera_group->setLayout(camera_layout);
            }
            draw_tab_layout->addWidget(camera_group);

            QWidget* const clear_colour_group = new QGroupBox("Clear colour [rgb]");
            {
                QVBoxLayout* const clear_colour_layout = new QVBoxLayout;
                {
                    QHBoxLayout* const edit_boxes_layout = new QHBoxLayout;
                    {
                        edit_boxes_layout->addWidget(w.clear_colour_component_red());
                        edit_boxes_layout->addWidget(w.clear_colour_component_green());
                        edit_boxes_layout->addWidget(w.clear_colour_component_blue());
                        w.wnd()->on_draw_clear_colour_changed();
                    }
                    clear_colour_layout->addLayout(edit_boxes_layout);

                    QHBoxLayout* const buttons_layout = new QHBoxLayout;
                    {
                        buttons_layout->addWidget(
                            [](program_window* wnd) {
                                struct choose : public QPushButton {
                                    choose(program_window* wnd) : QPushButton("Choose")
                                    {
                                        QObject::connect(this, SIGNAL(released()), wnd, SLOT(on_draw_clear_colour_choose()));
                                    }
                                };
                                return new choose(wnd);
                            }(w.wnd())
                            );
                        buttons_layout->addWidget(
                            [](program_window* wnd) {
                                struct choose : public QPushButton {
                                    choose(program_window* wnd) : QPushButton("Default")
                                    {
                                        QObject::connect(this, SIGNAL(released()), wnd, SLOT(on_draw_clear_colour_reset()));
                                    }
                                };
                                return new choose(wnd);
                            }(w.wnd())
                            );
                    }
                    clear_colour_layout->addLayout(buttons_layout);

                }
                clear_colour_group->setLayout(clear_colour_layout);
            }
            draw_tab_layout->addWidget(clear_colour_group);

            QWidget* const colliders_colour_group = new QGroupBox("Collider and contact normal colour [rgb]");
            {
                QVBoxLayout* const colliders_colour_layout = new QVBoxLayout;
                {
                    QHBoxLayout* const edit_boxes_layout = new QHBoxLayout;
                    {
                        edit_boxes_layout->addWidget(w.colliders_colour_component_red());
                        edit_boxes_layout->addWidget(w.colliders_colour_component_green());
                        edit_boxes_layout->addWidget(w.colliders_colour_component_blue());
                        w.wnd()->on_draw_colliders_colour_changed();
                    }
                    colliders_colour_layout->addLayout(edit_boxes_layout);

                    QHBoxLayout* const buttons_layout = new QHBoxLayout;
                    {
                        buttons_layout->addWidget(
                            [](program_window* wnd) {
                                struct choose : public QPushButton {
                                    choose(program_window* wnd) : QPushButton("Choose")
                                    {
                                        QObject::connect(this, SIGNAL(released()), wnd, SLOT(on_draw_colliders_colour_choose()));
                                    }
                                };
                                return new choose(wnd);
                            }(w.wnd())
                            );
                        buttons_layout->addWidget(
                            [](program_window* wnd) {
                                struct choose : public QPushButton {
                                    choose(program_window* wnd) : QPushButton("Default")
                                    {
                                        QObject::connect(this, SIGNAL(released()), wnd, SLOT(on_draw_colliders_colour_reset()));
                                    }
                                };
                                return new choose(wnd);
                            }(w.wnd())
                            );
                    }
                    colliders_colour_layout->addLayout(buttons_layout);

                }
                colliders_colour_group->setLayout(colliders_colour_layout);
            }
            draw_tab_layout->addWidget(colliders_colour_group);

            QVBoxLayout* const render_options_layout = new QVBoxLayout;
            {
                QWidget* const show_group = new QGroupBox("Show");
                {
                    QHBoxLayout* const show_layout = new QHBoxLayout;
                    {
                        show_layout->addWidget(w.show_grid());
                        w.wnd()->on_draw_show_grid_changed(0);

                        show_layout->addWidget(w.show_batches());
                        w.show_batches()->setToolTip(
                            "A 'batch' is a render unit for a graphic card. It consists\n"
                            "of shader programs and render buffers. It may also comprise\n"
                            "textures."
                            );
                        w.wnd()->on_draw_show_batches_changed(0);

                        show_layout->addWidget(w.show_colliders());
                        w.show_colliders()->setToolTip(
                            "A 'collider' is a convex collision primitive, like a sphere or capsule,\n"
                            "appearing in the collision system (used for contact detection between colliders)."
                            );
                        w.wnd()->on_draw_show_colliders_changed(0);

                        show_layout->addWidget(w.show_contact_normals());
                        w.show_contact_normals()->setToolTip(
                            "A 'contact normal' is a normal vector of a separation plane between\n"
                            "colliding colliders at a certain contact point.\n"
                            "NOTE: Contact normals are shown only when at least one simulation step\n"
                            "      is performed, after this check-box is set on. E.g. either press\n"
                            "      'SPACE' or 'PAUSE' key, if some colliders are in collision, but\n"
                            "      contact normals are not shown."
                            );
                        w.wnd()->on_draw_show_contact_normals_changed(0);

                        show_layout->addWidget(w.show_ai_action_control());
                        w.show_ai_action_control()->setToolTip(
                            "For each AI agent in the scene show its desire motion vectors, forward\n"
                            "and up direction vectors, and the current motion in the space (linear\n"
                            "and angular velocities). The vectors are drawn with these colours:\n"
                            "   forward-dir: AQUA\n"
                            "   up-dir: AZURE\n"
                            "   linear-velocity: YELLOW\n"
                            "   angular-velocity: ORANGE\n"
                            "   desired-forward-dir: WHITE\n"
                            "   desired-linear-velocity-dir: PINK\n"
                            "   desired-linear-velocity: PURPLE"
                            );
                        w.wnd()->on_draw_show_ai_action_control_changed(0);
                    }
                    show_group->setLayout(show_layout);
                }
                render_options_layout->addWidget(show_group);

                render_options_layout->addWidget(w.render_in_wireframe());
                w.wnd()->on_draw_render_in_wireframe_changed(0);
            }
            draw_tab_layout->addLayout(render_options_layout);

            draw_tab_layout->addStretch(1);
        }
        draw_tab->setLayout(draw_tab_layout);
    }
    return draw_tab;
}


}}
