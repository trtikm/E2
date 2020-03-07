#include <gfxtuner/dialog_windows/sketch_batch_dialogs.hpp>
#include <qtgl/gui_utils.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/canonical_path.hpp>
#include <utility/msgstream.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>

namespace dialog_windows {


extern std::string  check_triangle_mesh_buffers_directory(boost::filesystem::path const&  buffers_dir);


sketch_batch_kind_selection_dialog::sketch_batch_kind_selection_dialog(program_window* const  wnd, std::string const&  kind_)
    : QDialog(wnd)
    , m_wnd(wnd)
    , m_kind(kind_)
    , m_ok(false)
    , m_widget_ok(
            [](sketch_batch_kind_selection_dialog*  wnd) {
                    struct OK : public QPushButton {
                        OK(sketch_batch_kind_selection_dialog*  wnd) : QPushButton("OK")
                        {
                            QObject::connect(this, SIGNAL(released()), wnd, SLOT(accept()));
                        }
                    };
                    return new OK(wnd);
                }(this)
            )
    , m_kind_combo_box(new QComboBox)
{
    ASSUMPTION(
            m_kind == qtgl::sketch_kind_box()
            || m_kind == qtgl::sketch_kind_capsule()
            || m_kind == qtgl::sketch_kind_sphere()
            //|| m_kind == qtgl::sketch_kind_mesh()
            //|| m_kind == qtgl::sketch_kind_convex_hull()
            );
    m_kind_combo_box->addItem(qtgl::sketch_kind_box().c_str());
    m_kind_combo_box->addItem(qtgl::sketch_kind_capsule().c_str());
    m_kind_combo_box->addItem(qtgl::sketch_kind_sphere().c_str());
    //m_kind_combo_box->addItem(qtgl::sketch_kind_mesh().c_str());
    //m_kind_combo_box->addItem(qtgl::sketch_kind_convex_hull().c_str());
    m_kind_combo_box->setCurrentText(m_kind.c_str());

    QVBoxLayout* const dlg_layout = new QVBoxLayout;
    {
        dlg_layout->addWidget(new QLabel("Kind"));
        dlg_layout->addWidget(m_kind_combo_box);

        QHBoxLayout* const buttons_layout = new QHBoxLayout;
        {
            buttons_layout->addWidget(m_widget_ok);
            buttons_layout->addWidget(
                [](sketch_batch_kind_selection_dialog*  wnd) {
                    struct Close : public QPushButton {
                        Close(sketch_batch_kind_selection_dialog*  wnd) : QPushButton("Cancel")
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
    this->setWindowTitle("Sketch batch kind");
}


void  sketch_batch_kind_selection_dialog::accept()
{
    m_kind = qtgl::to_string(m_kind_combo_box->currentText());

    m_ok = true;

    QDialog::accept();
}

void  sketch_batch_kind_selection_dialog::reject()
{
    QDialog::reject();
}



sketch_batch_props_dialog::sketch_batch_props::sketch_batch_props()
    : m_kind(qtgl::sketch_kind_box())

    , m_colour(0.85f, 0.85f, 0.85f, 1.0f)
    , m_fog_type(qtgl::FOG_TYPE::NONE)
    , m_wireframe(false)

    // DATA OF CAPSULE & SPHERE

    , m_num_lines_per_quarter_of_circle(4)

    // DATA OF BOX

    , m_box_half_sizes_along_axes(0.5f, 0.5f, 0.5f)

    // DATA OF CAPSULE

    , m_capsule_half_distance_between_end_points(0.5f)
    , m_capsule_thickness_from_central_line(0.5f)

    // DATA OF SPHERE 

    , m_sphere_radius(0.5f)

    // DATA OF MESH & CONVEX HULL

    , m_triangle_mesh_buffers_directory()
{}

sketch_batch_props_dialog::sketch_batch_props::sketch_batch_props(std::string const&  sketch_id)
{
    boost::property_tree::ptree  props;
    qtgl::read_sketch_info_from_id(sketch_id, props);
    if (qtgl::parse_box_info_from_id(props, m_box_half_sizes_along_axes, m_colour, m_fog_type, m_wireframe))
        m_kind = qtgl::sketch_kind_box();
    else if (qtgl::parse_capsule_info_from_id(props, m_capsule_half_distance_between_end_points, m_capsule_thickness_from_central_line, m_num_lines_per_quarter_of_circle, m_colour, m_fog_type, m_wireframe))
        m_kind = qtgl::sketch_kind_capsule();
    else if (qtgl::parse_sphere_info_from_id(props, m_sphere_radius, m_num_lines_per_quarter_of_circle, m_colour, m_fog_type, m_wireframe))
        m_kind = qtgl::sketch_kind_sphere();
    else { UNREACHABLE(); return; }
}


sketch_batch_props_dialog::sketch_batch_props_dialog(program_window* const  wnd, sketch_batch_props* const  props)
    : QDialog(wnd)
    , m_wnd(wnd)
    , m_props(props)
    , m_ok(false)
    , m_widget_ok(
            [](sketch_batch_props_dialog*  wnd) {
                    struct OK : public QPushButton {
                        OK(sketch_batch_props_dialog*  wnd) : QPushButton("OK")
                        {
                            QObject::connect(this, SIGNAL(released()), wnd, SLOT(accept()));
                        }
                    };
                    return new OK(wnd);
                }(this)
            )

    // WIDGETS COMMON TO ALL COLLIDERS

    , m_widget_colour_red(new QLineEdit)
    , m_widget_colour_green(new QLineEdit)
    , m_widget_colour_blue(new QLineEdit)
    , m_widget_colour_alpha(new QLineEdit)

    , m_widget_fog_type(new QComboBox)

    , m_widget_wireframe(new QCheckBox("Wireframe"))

    // WIDGETS FOR BOX

    , m_widget_box_half_size_along_x_axis(nullptr)
    , m_widget_box_half_size_along_y_axis(nullptr)
    , m_widget_box_half_size_along_z_axis(nullptr)

    // WIDGETS FOR CAPSULE & SPHERE

    , m_widget_num_lines_per_quarter_of_circle(nullptr)

    // WIDGETS FOR CAPSULE

    , m_widget_capsule_half_distance_between_end_points(nullptr)
    , m_widget_capsule_thickness_from_central_line(nullptr)

    // WIDGETS FOR SPHERE

    , m_widget_sphere_radius(nullptr)

    // WIDGETS FOR TRIANGLE MESH

    , m_widget_triangle_mesh_buffers_directory(nullptr)
{
    ASSUMPTION(m_props != nullptr);

    QVBoxLayout* const dlg_layout = new QVBoxLayout;
    {
        QWidget* const collider_shape_group = new QGroupBox("Shape properties");
        {
            QVBoxLayout* const shape_layout = new QVBoxLayout;
            {
                if (m_props->m_kind == qtgl::sketch_kind_box())
                {
                    QHBoxLayout* const half_sizes_along_axis_layout = new QHBoxLayout;
                    {
                        half_sizes_along_axis_layout->addWidget(new QLabel("Half sizes [xyz]"));
                        m_widget_box_half_size_along_x_axis = new QLineEdit();
                        m_widget_box_half_size_along_x_axis->setText(
                            QString::number(m_props->m_box_half_sizes_along_axes(0))
                        );
                        m_widget_box_half_size_along_x_axis->setToolTip(
                            "A halved length of the box, i.e. half size along the axis x."
                        );
                        half_sizes_along_axis_layout->addWidget(m_widget_box_half_size_along_x_axis);

                        m_widget_box_half_size_along_y_axis = new QLineEdit();
                        m_widget_box_half_size_along_y_axis->setText(
                            QString::number(m_props->m_box_half_sizes_along_axes(1))
                        );
                        m_widget_box_half_size_along_y_axis->setToolTip(
                            "A halved width of the box, i.e. half size along the axis y."
                        );
                        half_sizes_along_axis_layout->addWidget(m_widget_box_half_size_along_y_axis);

                        m_widget_box_half_size_along_z_axis = new QLineEdit();
                        m_widget_box_half_size_along_z_axis->setText(
                            QString::number(m_props->m_box_half_sizes_along_axes(2))
                        );
                        m_widget_box_half_size_along_z_axis->setToolTip(
                            "A halved height of the box, i.e. half size along the axis z."
                        );
                        half_sizes_along_axis_layout->addWidget(m_widget_box_half_size_along_z_axis);

                        half_sizes_along_axis_layout->addStretch(1);
                    }
                    shape_layout->addLayout(half_sizes_along_axis_layout);
                }
                else if (m_props->m_kind == qtgl::sketch_kind_capsule())
                {
                    QHBoxLayout* const half_distance_between_end_points_layout = new QHBoxLayout;
                    {
                        half_distance_between_end_points_layout->addWidget(new QLabel("Excentricity: "));
                        m_widget_capsule_half_distance_between_end_points = new QLineEdit();
                        m_widget_capsule_half_distance_between_end_points->setText(
                                QString::number(m_props->m_capsule_half_distance_between_end_points)
                                );
                        m_widget_capsule_half_distance_between_end_points->setToolTip(
                                "A distance in meters from the genometrical center of the capsule\n"
                                "to the center of any of the two hemispheres."
                                );
                        half_distance_between_end_points_layout->addWidget(m_widget_capsule_half_distance_between_end_points);

                        half_distance_between_end_points_layout->addStretch(1);
                    }
                    shape_layout->addLayout(half_distance_between_end_points_layout);

                    QHBoxLayout* const thickness_from_central_line_layout = new QHBoxLayout;
                    {
                        thickness_from_central_line_layout->addWidget(new QLabel("Radius: "));
                        m_widget_capsule_thickness_from_central_line = new QLineEdit();
                        m_widget_capsule_thickness_from_central_line->setText(
                                QString::number(m_props->m_capsule_thickness_from_central_line)
                                );
                        m_widget_capsule_thickness_from_central_line->setToolTip(
                                "Defines a radius in meters of both hemispheres. You can also think\n"
                                "of this number as the thickness of the capsule."
                                );
                        thickness_from_central_line_layout->addWidget(m_widget_capsule_thickness_from_central_line);

                        thickness_from_central_line_layout->addStretch(1);
                    }
                    shape_layout->addLayout(thickness_from_central_line_layout);

                    QHBoxLayout* const num_lines_layout = new QHBoxLayout;
                    {
                        num_lines_layout->addWidget(new QLabel("Num lines per qurter circle"));
                        m_widget_num_lines_per_quarter_of_circle = new QLineEdit();
                        m_widget_num_lines_per_quarter_of_circle->setText(
                            QString::number(m_props->m_num_lines_per_quarter_of_circle)
                        );
                        m_widget_num_lines_per_quarter_of_circle->setToolTip(
                            "Defines a level of details. Namely, the intersection of the object with the XY-coord. plane\n"
                            "is a circle. Then specificy in the edit box, how many lines you want the 1/4 of that circle\n"
                            "to be approximated by. The lowest valid value is 1."
                        );
                        num_lines_layout->addWidget(m_widget_num_lines_per_quarter_of_circle);
                    }
                    shape_layout->addLayout(num_lines_layout);

                }
                else if (m_props->m_kind == qtgl::sketch_kind_sphere())
                {
                    QHBoxLayout* const radius_layout = new QHBoxLayout;
                    {
                        radius_layout->addWidget(new QLabel("Radius: "));
                        m_widget_sphere_radius = new QLineEdit();
                        m_widget_sphere_radius->setText(QString::number(m_props->m_sphere_radius));
                        m_widget_sphere_radius->setToolTip("The radius of the sphere in meters.");
                        radius_layout->addWidget(m_widget_sphere_radius);
                        radius_layout->addStretch(1);
                    }
                    shape_layout->addLayout(radius_layout);

                    QHBoxLayout* const num_lines_layout = new QHBoxLayout;
                    {
                        num_lines_layout->addWidget(new QLabel("Num lines per qurter circle"));
                        m_widget_num_lines_per_quarter_of_circle = new QLineEdit();
                        m_widget_num_lines_per_quarter_of_circle->setText(
                            QString::number(m_props->m_num_lines_per_quarter_of_circle)
                        );
                        m_widget_num_lines_per_quarter_of_circle->setToolTip(
                            "Defines a level of details. Namely, the intersection of the object with the XY-coord. plane\n"
                            "is a circle. Then specificy in the edit box, how many lines you want the 1/4 of that circle\n"
                            "to be approximated by. The lowest valid value is 1."
                        );
                        num_lines_layout->addWidget(m_widget_num_lines_per_quarter_of_circle);
                    }
                    shape_layout->addLayout(num_lines_layout);
                }
                else if (m_props->m_kind == qtgl::sketch_kind_mesh() || m_props->m_kind == qtgl::sketch_kind_convex_hull())
                {
                    QHBoxLayout* const triangle_mesh_layout = new QHBoxLayout;
                    {
                        triangle_mesh_layout->addWidget(new QLabel("Buffers' directory: "));
                        m_widget_triangle_mesh_buffers_directory = new QLineEdit();
                        m_widget_triangle_mesh_buffers_directory->setText(
                            QString(m_props->m_triangle_mesh_buffers_directory.string().c_str())
                            );
                        m_widget_triangle_mesh_buffers_directory->setToolTip(
                            "A directory containing files 'vertices.txt' and 'indices.txt', representing\n"
                            "vertex and index buffer respectively, for the constructed triangle mesh."
                            );
                        triangle_mesh_layout->addWidget(m_widget_triangle_mesh_buffers_directory);

                        triangle_mesh_layout->addWidget(
                            [](sketch_batch_props_dialog*  wnd) {
                                struct choose_dir : public QPushButton {
                                    choose_dir(sketch_batch_props_dialog*  wnd) : QPushButton("Choose")
                                    {
                                        QObject::connect(this, SIGNAL(released()), wnd, SLOT(on_triangle_mesh_choose_buffers_directory()));
                                    }
                                };
                                return new choose_dir(wnd);
                            }(this)
                            );
                    }
                    shape_layout->addLayout(triangle_mesh_layout);

                    m_widget_ok->setEnabled(
                            check_triangle_mesh_buffers_directory(
                                    qtgl::to_string(m_widget_triangle_mesh_buffers_directory->text())
                                    ).empty()
                            );
                }
                else
                {
                    UNREACHABLE();
                }

                shape_layout->addStretch(1);
            }
            collider_shape_group->setLayout(shape_layout);
        }
        dlg_layout->addWidget(collider_shape_group);

        QVBoxLayout* const common_props_layout = new QVBoxLayout;
        {
            QWidget* const colour_group = new QGroupBox("Colour [rgba] (in range [0..255] each)");
            {
                QHBoxLayout* const colour_layout = new QHBoxLayout;
                {
                    colour_layout->addWidget(m_widget_colour_red);
                    m_widget_colour_red->setText(std::to_string((int)(m_props->m_colour(0) * 255.0f + 0.5f)).c_str());
                    m_widget_colour_red->setToolTip("Red component of the colour of the sketch batch. In range [0..255].");
                    colour_layout->addWidget(m_widget_colour_green);
                    m_widget_colour_green->setText(std::to_string((int)(m_props->m_colour(1) * 255.0f + 0.5f)).c_str());
                    m_widget_colour_green->setToolTip("Green component of the colour of the sketch batch. In range [0..255].");
                    colour_layout->addWidget(m_widget_colour_blue);
                    m_widget_colour_blue->setText(std::to_string((int)(m_props->m_colour(2) * 255.0f + 0.5f)).c_str());
                    m_widget_colour_blue->setToolTip("Blue component of the colour of the sketch batch. In range [0..255].");
                    colour_layout->addWidget(m_widget_colour_alpha);
                    m_widget_colour_alpha->setText(std::to_string((int)(m_props->m_colour(3) * 255.0f + 0.5f)).c_str());
                    m_widget_colour_alpha->setToolTip("Alpha component of the colour of the sketch batch. In range [0..255].");
                }
                colour_group->setLayout(colour_layout);
            }
            common_props_layout->addWidget(colour_group);

            QHBoxLayout* const fog_and_wireframe_layout = new QHBoxLayout;
            {
                fog_and_wireframe_layout->addWidget(new QLabel("Fog type"));

                fog_and_wireframe_layout->addWidget(m_widget_fog_type);
                m_widget_fog_type->setEditable(false);
                m_widget_fog_type->insertItem(0, qtgl::name(qtgl::FOG_TYPE::NONE).c_str());
                m_widget_fog_type->insertItem(1, qtgl::name(qtgl::FOG_TYPE::INTERPOLATED).c_str());
                m_widget_fog_type->insertItem(2, qtgl::name(qtgl::FOG_TYPE::DETAILED).c_str());
                m_widget_fog_type->setCurrentText(qtgl::name(m_props->m_fog_type).c_str());
                m_widget_fog_type->setToolTip(
                        "Adds this collider to the selected collision class. Two colliders may collide only if\n"
                        "their collision classes allow for collision detection. Here is the table (only the lower\n"
                        "triangle as the table is symetrix) of collision classes:\n"
                        "   TODO: print here the table of collision classes."
                        );

                fog_and_wireframe_layout->addWidget(m_widget_wireframe);
                m_widget_wireframe->setChecked(m_props->m_wireframe);
                fog_and_wireframe_layout->addStretch(1);
            }
            common_props_layout->addLayout(fog_and_wireframe_layout);

            common_props_layout->addStretch(1);
        }
        dlg_layout->addLayout(common_props_layout);

        QHBoxLayout* const buttons_layout = new QHBoxLayout;
        {
            buttons_layout->addWidget(m_widget_ok);
            buttons_layout->addWidget(
                [](sketch_batch_props_dialog*  wnd) {
                    struct Close : public QPushButton {
                        Close(sketch_batch_props_dialog*  wnd) : QPushButton("Cancel")
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
    this->setWindowTitle((msgstream() << "Sketch " << m_props->m_kind).get().c_str());
    //this->resize(300,100);
}


void  sketch_batch_props_dialog::on_triangle_mesh_choose_buffers_directory()
{
    QFileDialog  dialog(this);
    dialog.setDirectory(m_widget_triangle_mesh_buffers_directory->text());
    dialog.setFileMode(QFileDialog::DirectoryOnly);
    if (!dialog.exec())
        return;
    QStringList const  selected = dialog.selectedFiles();
    if (selected.size() != 1)
    {
        m_wnd->print_status_message("ERROR: Exactly one directory must be selected/provided.", 10000);
        return;
    }
    m_widget_triangle_mesh_buffers_directory->setText(selected.front());
    boost::filesystem::path const  buffers_dir = canonical_path(boost::filesystem::absolute(qtgl::to_string(selected.front())));
    std::string const  error_message = check_triangle_mesh_buffers_directory(
            qtgl::to_string(m_widget_triangle_mesh_buffers_directory->text())
            );
    if (!error_message.empty())
    {
        m_widget_ok->setEnabled(false);
        m_wnd->print_status_message("ERROR: " + error_message, 10000);
        return;
    }
    m_widget_ok->setEnabled(true);
}


void  sketch_batch_props_dialog::accept()
{
    if (m_props->m_kind == qtgl::sketch_kind_box())
    {
        m_props->m_box_half_sizes_along_axes(0) =
            std::atof(qtgl::to_string(m_widget_box_half_size_along_x_axis->text()).c_str());
        m_props->m_box_half_sizes_along_axes(1) =
            std::atof(qtgl::to_string(m_widget_box_half_size_along_y_axis->text()).c_str());
        m_props->m_box_half_sizes_along_axes(2) =
            std::atof(qtgl::to_string(m_widget_box_half_size_along_z_axis->text()).c_str());
    }
    else if (m_props->m_kind == qtgl::sketch_kind_capsule())
    {
        m_props->m_capsule_half_distance_between_end_points =
                std::atof(qtgl::to_string(m_widget_capsule_half_distance_between_end_points->text()).c_str());
        m_props->m_capsule_thickness_from_central_line =
                std::atof(qtgl::to_string(m_widget_capsule_thickness_from_central_line->text()).c_str());
        m_props->m_num_lines_per_quarter_of_circle = std::atoi(qtgl::to_string(m_widget_num_lines_per_quarter_of_circle->text()).c_str());
    }
    else if (m_props->m_kind == qtgl::sketch_kind_sphere())
    {
        m_props->m_sphere_radius =
            std::atof(qtgl::to_string(m_widget_sphere_radius->text()).c_str());
        m_props->m_num_lines_per_quarter_of_circle = std::atoi(qtgl::to_string(m_widget_num_lines_per_quarter_of_circle->text()).c_str());
    }
    else if (m_props->m_kind == qtgl::sketch_kind_mesh() || m_props->m_kind == qtgl::sketch_kind_sphere())
    {
        m_props->m_triangle_mesh_buffers_directory = qtgl::to_string(m_widget_triangle_mesh_buffers_directory->text());
    }
    else
    {
        UNREACHABLE();
    }

    m_props->m_colour(0) = std::atof(qtgl::to_string(m_widget_colour_red->text()).c_str()) / 255.0f;
    m_props->m_colour(1) = std::atof(qtgl::to_string(m_widget_colour_green->text()).c_str()) / 255.0f;
    m_props->m_colour(2) = std::atof(qtgl::to_string(m_widget_colour_blue->text()).c_str()) / 255.0f;
    m_props->m_colour(3) = std::atof(qtgl::to_string(m_widget_colour_alpha->text()).c_str()) / 255.0f;

    m_props->m_fog_type = qtgl::fog_type_from_name(qtgl::to_string(m_widget_fog_type->currentText()));

    m_props->m_wireframe = m_widget_wireframe->isChecked();

    m_ok = true;

    QDialog::accept();
}

void  sketch_batch_props_dialog::reject()
{
    QDialog::reject();
}


}
