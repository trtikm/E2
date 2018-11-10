#include <gfxtuner/window_tabs/tab_scene_record_batch.hpp>
#include <gfxtuner/window_tabs/tab_scene.hpp>
#include <gfxtuner/window_tabs/tab_scene_utils.hpp>
#include <gfxtuner/program_window.hpp>
#include <gfxtuner/program_options.hpp>
#include <angeo/collision_material.hpp>
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
#include <memory>

namespace window_tabs { namespace tab_scene { namespace record_collider { namespace detail {


std::string  check_triangle_mesh_buffers_directory(boost::filesystem::path const&  buffers_dir)
{
    if (!boost::filesystem::is_directory(buffers_dir))
        return "Not a directory '" + buffers_dir.string() + "'.";
    if (!boost::filesystem::is_regular_file(buffers_dir / "vetrices.txt"))
        return "No file 'vertices.txt' inside the directory '" + buffers_dir.string() + "'.";
    if (!boost::filesystem::is_regular_file(buffers_dir / "indices.txt"))
        return "No file 'indices.txt' inside the directory '" + buffers_dir.string() + "'.";
    return {};
}


struct  collider_props_dialog : public QDialog
{
    struct  collider_props
    {
        std::string  m_shape_type;

        bool  m_as_dynamic;
        angeo::COLLISION_MATERIAL_TYPE  m_material;
        float_32_bit  m_density_multiplier;

        // DATA OF CAPSULE

        float_32_bit  m_capsule_half_distance_between_end_points;
        float_32_bit  m_capsule_thickness_from_central_line;

        // DATA OF SPHERE 

        float_32_bit  m_sphere_radius;

        // DATA OF TRIANGLE MESH

        boost::filesystem::path  m_triangle_mesh_buffers_directory;
    };

    collider_props_dialog(program_window* const  wnd, collider_props* const  props);

    bool  ok() const { return m_ok; }

public slots:

    void  on_triangle_mesh_choose_buffers_directory();

    void  accept();
    void  reject();

private:
    program_window*  m_wnd;
    collider_props*  m_props;
    bool  m_ok;
    QPushButton*  m_widget_ok;

    // WIDGETS COMMON TO ALL COLLIDERS

    QCheckBox*  m_widget_as_dynamic;
    QComboBox*  m_widget_material;
    QLineEdit*  m_density_multiplier;

    // WIDGETS FOR CAPSULE

    QLineEdit*  m_widget_capsule_half_distance_between_end_points;
    QLineEdit*  m_widget_capsule_thickness_from_central_line;

    // WIDGETS FOR SPHERE

    QLineEdit*  m_widget_sphere_radius;

    // WIDGETS FOR TRIANGLE MESH

    QLineEdit*  m_widget_triangle_mesh_buffers_directory;
};


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
        std::unordered_map<std::string, std::pair<bool,
                           std::function<std::pair<std::string, std::function<void(scn::scene_record_id const&)>>
                                         (widgets*, std::string const&, std::unordered_set<std::string> const&)>> >&
                insert_record_handlers
        )
{
    insert_record_handlers.insert({
            scn::get_collider_folder_name(),
            {
                false, // there cannot be multiple records in the folder.
                [](widgets* const  w, std::string const&  shape_type, std::unordered_set<std::string> const&  used_names)
                    -> std::pair<std::string, std::function<void(scn::scene_record_id const&)>> {
                        if (used_names.size() != 0UL)
                        {
                            w->wnd()->print_status_message("ERROR: A coordinate system node may contain at most one collider.", 10000);
                            return {"", {}};
                        }

                        std::shared_ptr<detail::collider_props_dialog::collider_props>  props =
                                std::make_shared<detail::collider_props_dialog::collider_props>();
                        props->m_shape_type = shape_type;
                        props->m_as_dynamic = true;
                        props->m_material = angeo::COLLISION_MATERIAL_TYPE::WOOD;
                        props->m_density_multiplier = 1.0f;
                        if (props->m_shape_type == "capsule")
                        {
                            props->m_capsule_half_distance_between_end_points = 0.05f;
                            props->m_capsule_thickness_from_central_line = 0.05f;
                        }
                        else if (props->m_shape_type == "sphere")
                        {
                            props->m_sphere_radius = 0.05f;
                        }
                        else if (props->m_shape_type == "triangle mesh")
                        {
                            props->m_as_dynamic = false;
                            props->m_triangle_mesh_buffers_directory =
                                canonical_path(boost::filesystem::absolute(
                                    boost::filesystem::path(get_program_options()->dataRoot()) / "shared" / "gfx" / "meshes"
                                    ));
                        }
                        else
                        {
                            UNREACHABLE();
                        }
                        detail::collider_props_dialog  dlg(w->wnd(), props.get());
                        dlg.exec();
                        if (!dlg.ok())
                            return{ "",{} };
                        return {
                            props->m_shape_type,
                            [w, props](scn::scene_record_id const&  record_id) -> void {
                                    if (props->m_shape_type == "capsule")
                                    {
                                        w->wnd()->glwindow().call_now(
                                                &simulator::insert_collision_capsule_to_scene_node,
                                                props->m_capsule_half_distance_between_end_points,
                                                props->m_capsule_thickness_from_central_line,
                                                props->m_material,
                                                props->m_density_multiplier,
                                                props->m_as_dynamic,
                                                record_id
                                                );
                                        //w->get_scene_history()->insert<scn::scene_history_batch_insert>(record_id, false);
                                    }
                                    else if (props->m_shape_type == "sphere")
                                    {
                                        w->wnd()->glwindow().call_now(
                                                &simulator::insert_collision_sphere_to_scene_node,
                                                props->m_sphere_radius,
                                                props->m_material,
                                                props->m_density_multiplier,
                                                props->m_as_dynamic,
                                                record_id
                                                );
                                        //w->get_scene_history()->insert<scn::scene_history_batch_insert>(record_id, false);
                                    }
                                    else if (props->m_shape_type == "triangle mesh")
                                    {
                                        qtgl::buffer  vertex_buffer(props->m_triangle_mesh_buffers_directory / "vertices.txt");
                                        qtgl::buffer  index_buffer(props->m_triangle_mesh_buffers_directory / "indices.txt");

                                        if (!vertex_buffer.wait_till_load_is_finished())
                                        {
                                            w->wnd()->print_status_message("ERROR: Failed to load vertex buffer '" + vertex_buffer.key().get_unique_id() + "'.", 10000);
                                            return;
                                        }
                                        if (!index_buffer.wait_till_load_is_finished())
                                        {
                                            w->wnd()->print_status_message("ERROR: Failed to load index buffer '" + index_buffer.key().get_unique_id() + "'.", 10000);
                                            return;
                                        }

                                        w->wnd()->glwindow().call_now(
                                                &simulator::insert_collision_trianle_mesh_to_scene_node,
                                                vertex_buffer,
                                                index_buffer,
                                                props->m_material,
                                                props->m_density_multiplier,
                                                // props->m_as_dynamic, <-- Is always assumed to be 'false'.
                                                record_id
                                                );
                                        //w->get_scene_history()->insert<scn::scene_history_batch_insert>(record_id, false);
                                    }
                                    else
                                    {
                                        UNREACHABLE();
                                    }
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
            scn::get_collider_folder_name(),
            [](widgets* const  w, scn::scene_record_id const&  record_id) -> void {
                    detail::collider_props_dialog::collider_props  props;
                    props.m_shape_type = record_id.get_record_name();
                    if (props.m_shape_type == "capsule")
                        w->wnd()->glwindow().call_now(
                                &simulator::get_collision_capsule_info,
                                record_id,
                                std::ref(props.m_capsule_half_distance_between_end_points),
                                std::ref(props.m_capsule_thickness_from_central_line),
                                std::ref(props.m_material),
                                std::ref(props.m_density_multiplier),
                                std::ref(props.m_as_dynamic)
                                );
                    else if (props.m_shape_type == "sphere")
                        w->wnd()->glwindow().call_now(
                                &simulator::get_collision_sphere_info,
                                record_id,
                                std::ref(props.m_sphere_radius),
                                std::ref(props.m_material),
                                std::ref(props.m_density_multiplier),
                                std::ref(props.m_as_dynamic)
                                );
                    else if (props.m_shape_type == "triangle mesh")
                    {
                        qtgl::buffer  vertex_buffer;
                        qtgl::buffer  index_buffer;
                        w->wnd()->glwindow().call_now(
                                &simulator::get_collision_triangle_mesh_info,
                                record_id,
                                std::ref(vertex_buffer),
                                std::ref(index_buffer),
                                std::ref(props.m_material),
                                std::ref(props.m_density_multiplier)
                                );
                        props.m_triangle_mesh_buffers_directory =
                                boost::filesystem::path(vertex_buffer.key().get_unique_id()).parent_path();
                        props.m_as_dynamic = false;
                    }
                    else
                    {
                        UNREACHABLE();
                    }
                    detail::collider_props_dialog  dlg(w->wnd(), &props);
                    dlg.exec();
                    if (!dlg.ok())
                        return;
                    w->wnd()->glwindow().call_now(
                            &simulator::erase_collision_object_from_scene_node,
                            record_id
                            );
                    if (props.m_shape_type == "capsule")
                        w->wnd()->glwindow().call_now(
                                &simulator::insert_collision_capsule_to_scene_node,
                                props.m_capsule_half_distance_between_end_points,
                                props.m_capsule_thickness_from_central_line,
                                props.m_material,
                                props.m_density_multiplier,
                                props.m_as_dynamic,
                                record_id
                                );
                    else if (props.m_shape_type == "sphere")
                        w->wnd()->glwindow().call_now(
                                &simulator::insert_collision_sphere_to_scene_node,
                                props.m_sphere_radius,
                                props.m_material,
                                props.m_density_multiplier,
                                props.m_as_dynamic,
                                record_id
                                );
                    else if (props.m_shape_type == "triangle mesh")
                    {
                        qtgl::buffer  vertex_buffer(props.m_triangle_mesh_buffers_directory / "vertices.txt");
                        qtgl::buffer  index_buffer(props.m_triangle_mesh_buffers_directory / "indices.txt");

                        if (!vertex_buffer.wait_till_load_is_finished())
                        {
                            w->wnd()->print_status_message("ERROR: Failed to load vertex buffer '" + vertex_buffer.key().get_unique_id() + "'.", 10000);
                            return;
                        }
                        if (!index_buffer.wait_till_load_is_finished())
                        {
                            w->wnd()->print_status_message("ERROR: Failed to load index buffer '" + index_buffer.key().get_unique_id() + "'.", 10000);
                            return;
                        }

                        w->wnd()->glwindow().call_now(
                                &simulator::insert_collision_trianle_mesh_to_scene_node,
                                vertex_buffer,
                                index_buffer,
                                props.m_material,
                                props.m_density_multiplier,
                                // props.m_as_dynamic, <-- Is always assumed to be 'false'.
                                record_id
                                );
                    }
                    else
                    {
                        UNREACHABLE();
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
                            id
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
            scn::get_collider_folder_name(),
            [](widgets* const  w, scn::scene_record_id const&  id, boost::property_tree::ptree const&  data) -> void {
                    w->wnd()->glwindow().call_now(
                            &simulator::load_collider,
                            std::cref(data),
                            id.get_node_name()
                            );
                    insert_record_to_tree_widget(
                            w->scene_tree(),
                            id,
                            w->get_record_icon(scn::get_collider_folder_name()),
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
            scn::get_collider_folder_name(),
            []( widgets* const  w,
                scn::scene_node_ptr const  node_ptr,
                scn::scene_node_record_id const&  id,
                boost::property_tree::ptree&  data) -> void {
                    w->wnd()->glwindow().call_now(
                            &simulator::save_collider,
                            *scn::get_collider(*node_ptr),
                            std::ref(data)
                            );
                }
            });
}


}}}
