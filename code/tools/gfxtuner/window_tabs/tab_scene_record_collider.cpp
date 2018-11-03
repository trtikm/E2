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
#include <QDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QGroupBox>

namespace window_tabs { namespace tab_scene { namespace record_collider { namespace detail {


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

        boost::filesystem::path  m_triangle_mesh_vertex_buffer_pathname;
        boost::filesystem::path  m_triangle_mesh_index_buffer_pathname;     // The path is empty, when no index buffer is available.
    };

    collider_props_dialog(program_window* const  wnd, collider_props* const  props);

    bool  ok() const { return m_ok; }

public slots:

    void  accept();
    void  reject();

private:
    program_window*  m_wnd;
    collider_props*  m_props;
    bool  m_ok;

    QCheckBox*  m_widget_as_dynamic;
    QComboBox*  m_widget_material;
    QLineEdit*  m_density_multiplier;

    QLineEdit*  m_widget_capsule_half_distance_between_end_points;
    QLineEdit*  m_widget_capsule_thickness_from_central_line;

    QLineEdit*  m_widget_sphere_radius;
};


collider_props_dialog::collider_props_dialog(program_window* const  wnd, collider_props* const  props)
    : QDialog(wnd)
    , m_wnd(wnd)
    , m_props(props)
    , m_ok(false)

    , m_widget_as_dynamic(new QCheckBox("Movable during simulation"))
    , m_widget_material(new QComboBox)
    , m_density_multiplier(new QLineEdit)

    , m_widget_capsule_half_distance_between_end_points(nullptr)
    , m_widget_capsule_thickness_from_central_line(nullptr)

    , m_widget_sphere_radius(nullptr)
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
                        m_widget_sphere_radius  = new QLineEdit();
                        m_widget_sphere_radius->setText(QString::number(m_props->m_sphere_radius));
                        m_widget_sphere_radius->setToolTip("The radius of the sphere in meters.");
                        radius_layout->addWidget(m_widget_sphere_radius);
                        radius_layout->addStretch(1);
                    }
                    collider_shape_layout->addLayout(radius_layout);
                }
                else if (m_props->m_shape_type == "triangle mesh")
                {
                    // TODO!
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
        //dlg_layout->setAlignment(buttons_layout, Qt::Alignment(Qt::AlignmentFlag::AlignRight));
    }
    this->setLayout(dlg_layout);
    this->setWindowTitle((msgstream() << "Collider: " << m_props->m_shape_type).get().c_str());
    //this->resize(300,100);
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
        // TODO!
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

                    detail::collider_props_dialog::collider_props  props;
                    props.m_shape_type = shape_type;
                    props.m_as_dynamic = true;
                    props.m_material = angeo::COLLISION_MATERIAL_TYPE::WOOD;
                    props.m_density_multiplier = 1.0f;
                    if (props.m_shape_type == "capsule")
                    {
                        props.m_capsule_half_distance_between_end_points = 0.05f;
                        props.m_capsule_thickness_from_central_line = 0.05f;
                    }
                    else if (props.m_shape_type == "sphere")
                    {
                        props.m_sphere_radius = 0.05f;
                    }
                    else if (props.m_shape_type == "triangle mesh")
                    {
                        // TODO!
                    }
                    else
                    {
                        UNREACHABLE();
                    }
                    detail::collider_props_dialog  dlg(w->wnd(), &props);
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
