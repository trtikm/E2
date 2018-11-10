#include <gfxtuner/window_tabs/tab_scene_record_collider_props_dialog.hpp>
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

namespace window_tabs { namespace tab_scene { namespace record_collider { namespace detail {


std::string  check_triangle_mesh_buffers_directory(boost::filesystem::path const&  buffers_dir)
{
    if (!boost::filesystem::is_directory(buffers_dir))
        return "Not a directory '" + buffers_dir.string() + "'.";
    if (!boost::filesystem::is_regular_file(buffers_dir / "vertices.txt"))
        return "No file 'vertices.txt' inside the directory '" + buffers_dir.string() + "'.";
    if (!boost::filesystem::is_regular_file(buffers_dir / "indices.txt"))
        return "No file 'indices.txt' inside the directory '" + buffers_dir.string() + "'.";
    return {};
}


collider_props_dialog::collider_props_dialog(program_window* const  wnd, collider_props* const  props)
    : QDialog(wnd)
    , m_wnd(wnd)
    , m_props(props)
    , m_ok(false)
    , m_widget_ok(
            [](collider_props_dialog* wnd) {
                    struct OK : public QPushButton {
                        OK(collider_props_dialog* wnd) : QPushButton("OK")
                        {
                            QObject::connect(this, SIGNAL(released()), wnd, SLOT(accept()));
                        }
                    };
                    return new OK(wnd);
                }(this)
            )

    // WIDGETS COMMON TO ALL COLLIDERS

    , m_widget_as_dynamic(new QCheckBox("Movable during simulation"))
    , m_widget_material(new QComboBox)
    , m_density_multiplier(new QLineEdit)

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
            QVBoxLayout* const collider_shape_layout = new QVBoxLayout;
            {
                if (m_props->m_shape_type == "capsule")
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
                    collider_shape_layout->addLayout(half_distance_between_end_points_layout);

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
                    collider_shape_layout->addLayout(thickness_from_central_line_layout);
                }
                else if (m_props->m_shape_type == "sphere")
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
                    collider_shape_layout->addLayout(radius_layout);
                }
                else if (m_props->m_shape_type == "triangle mesh")
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
                            [](collider_props_dialog* wnd) {
                                struct choose_dir : public QPushButton {
                                    choose_dir(collider_props_dialog* wnd) : QPushButton("Choose")
                                    {
                                        QObject::connect(this, SIGNAL(released()), wnd, SLOT(on_triangle_mesh_choose_buffers_directory()));
                                    }
                                };
                                return new choose_dir(wnd);
                            }(this)
                            );

                        triangle_mesh_layout->addStretch(1);

                        m_widget_as_dynamic->setDisabled(true);
                        m_widget_ok->setEnabled(
                                check_triangle_mesh_buffers_directory(
                                        qtgl::to_string(m_widget_triangle_mesh_buffers_directory->text())
                                        ).empty()
                                );
                    }
                    collider_shape_layout->addLayout(triangle_mesh_layout);
                }
                else
                {
                    UNREACHABLE();
                }

                collider_shape_layout->addStretch(1);
            }
            collider_shape_group->setLayout(collider_shape_layout);
        }
        dlg_layout->addWidget(collider_shape_group);

        QVBoxLayout* const common_props_layout = new QVBoxLayout;
        {
            QWidget* const material_group = new QGroupBox("Material (with density multiplier)");
            {
                QHBoxLayout* const material_layout = new QHBoxLayout;
                {
                    material_layout->addWidget(m_widget_material);
                    m_widget_material->setEditable(false);
                    m_widget_material->setToolTip(
                            "Defines a density of material in the volume of the constructed collider\n"
                            "and also participates in computation of friction coefficients (both static\n"
                            "and dynamic) at contact points with other colliders."
                            );
                    for (natural_8_bit  mat_id = 0U; mat_id != angeo::get_num_collision_materials(); ++mat_id)
                        m_widget_material->insertItem(mat_id, angeo::to_string(angeo::as_material(mat_id)));
                    m_widget_material->setCurrentIndex(angeo::as_number(m_props->m_material));

                    material_layout->addWidget(m_density_multiplier);
                    m_density_multiplier->setText(QString::number(m_props->m_density_multiplier));
                    m_density_multiplier->setToolTip(
                            "Allows you to diverge from real densities from the predefined materials.\n"
                            "Namely, the density of a material you choose is multiplied by the number\n"
                            "inserted here. Only positive values of the multiplier make sense."
                            );
                }
                material_group->setLayout(material_layout);
            }
            common_props_layout->addWidget(material_group);

            common_props_layout->addWidget(m_widget_as_dynamic);
            m_widget_as_dynamic->setChecked(m_props->m_as_dynamic);

            common_props_layout->addStretch(1);
        }
        dlg_layout->addLayout(common_props_layout);

        QHBoxLayout* const buttons_layout = new QHBoxLayout;
        {
            buttons_layout->addWidget(m_widget_ok);
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
        //dlg_layout->setAlignment(buttons_layout, Qt::Alignment(Qt::AlignmentFlag::AlignRight));
    }
    this->setLayout(dlg_layout);
    this->setWindowTitle((msgstream() << "Collider: " << m_props->m_shape_type).get().c_str());
    //this->resize(300,100);
}


void  collider_props_dialog::on_triangle_mesh_choose_buffers_directory()
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


void  collider_props_dialog::accept()
{
    if (m_props->m_shape_type == "capsule")
    {
        m_props->m_capsule_half_distance_between_end_points =
                std::atof(qtgl::to_string(m_widget_capsule_half_distance_between_end_points->text()).c_str());
        m_props->m_capsule_thickness_from_central_line =
                std::atof(qtgl::to_string(m_widget_capsule_thickness_from_central_line->text()).c_str());
    }
    else if (m_props->m_shape_type == "sphere")
    {
        m_props->m_sphere_radius =
            std::atof(qtgl::to_string(m_widget_sphere_radius->text()).c_str());
    }
    else if (m_props->m_shape_type == "triangle mesh")
    {
        m_props->m_triangle_mesh_buffers_directory = qtgl::to_string(m_widget_triangle_mesh_buffers_directory->text());
    }
    else
    {
        UNREACHABLE();
    }

    m_props->m_as_dynamic = m_widget_as_dynamic->isChecked();
    m_props->m_material = (angeo::COLLISION_MATERIAL_TYPE)m_widget_material->currentIndex();
    m_props->m_density_multiplier = std::atof(qtgl::to_string(m_density_multiplier->text()).c_str());

    m_ok = true;

    QDialog::accept();
}

void  collider_props_dialog::reject()
{
    QDialog::reject();
}


}}}}
