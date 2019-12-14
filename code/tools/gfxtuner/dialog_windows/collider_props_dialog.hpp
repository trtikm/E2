#ifndef E2_TOOL_GFXTUNER_DIALOG_WINDOWS_COLLIDER_PROPS_DIALOG_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_DIALOG_WINDOWS_COLLIDER_PROPS_DIALOG_HPP_INCLUDED

#   include <gfxtuner/program_window.hpp>
#   include <angeo/collision_material.hpp>
#   include <scene/scene.hpp>
#   include <scene/scene_node_record_id.hpp>
#   include <scene/scene_utils_specialised.hpp>
#   include <scene/records/collider/collider.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <boost/filesystem/path.hpp>
#   include <QDialog>
#   include <QPushButton>
#   include <QCheckBox>
#   include <QComboBox>
#   include <QLineEdit>

namespace dialog_windows {


std::string  check_triangle_mesh_buffers_directory(boost::filesystem::path const&  buffers_dir);


struct  collider_props_dialog : public QDialog
{
    collider_props_dialog(program_window* const  wnd, scn::collider_props* const  props);

    bool  ok() const { return m_ok; }

public slots:

    void  on_triangle_mesh_choose_buffers_directory();

    void  accept();
    void  reject();

private:
    Q_OBJECT

    program_window*  m_wnd;
    scn::collider_props*  m_props;
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


}


#endif
