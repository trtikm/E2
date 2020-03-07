#ifndef E2_TOOL_GFXTUNER_DIALOG_WINDOWS_SKETCH_BATCH_DIALOGS_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_DIALOG_WINDOWS_SKETCH_BATCH_DIALOGS_HPP_INCLUDED

#   include <gfxtuner/program_window.hpp>
#   include <angeo/tensor_math.hpp>
#   include <qtgl/effects_config.hpp>
#   include <boost/filesystem/path.hpp>
#   include <QDialog>
#   include <QPushButton>
#   include <QCheckBox>
#   include <QLineEdit>
#   include <QComboBox>

namespace dialog_windows {


struct  sketch_batch_kind_selection_dialog : public QDialog
{
    sketch_batch_kind_selection_dialog(program_window* const  wnd, std::string const&  kind_);

    bool  ok() const { return m_ok; }

    std::string  get_kind() const { return m_kind; }

public slots:

    void  accept();
    void  reject();

private:
    Q_OBJECT

    program_window*  m_wnd;
    std::string  m_kind;
    bool  m_ok;
    QPushButton*  m_widget_ok;

    QComboBox* m_kind_combo_box;
};


struct  sketch_batch_props_dialog : public QDialog
{
    struct  sketch_batch_props  final
    {
        sketch_batch_props();
        explicit sketch_batch_props(std::string const&  sketch_id);

        std::string  m_kind;

        vector4  m_colour;
        qtgl::FOG_TYPE  m_fog_type;
        bool  m_wireframe;

        // DATA OF CAPSULE & SPHERE

        natural_8_bit  m_num_lines_per_quarter_of_circle;

        // DATA OF BOX

        vector3  m_box_half_sizes_along_axes;

        // DATA OF CAPSULE

        float_32_bit  m_capsule_half_distance_between_end_points;
        float_32_bit  m_capsule_thickness_from_central_line;

        // DATA OF SPHERE 

        float_32_bit  m_sphere_radius;

        // DATA OF MESH & CONVEX HULL

        boost::filesystem::path  m_triangle_mesh_buffers_directory;
    };

    sketch_batch_props_dialog(program_window* const  wnd, sketch_batch_props* const  props);

    bool  ok() const { return m_ok; }

public slots:

    void  on_triangle_mesh_choose_buffers_directory();

    void  accept();
    void  reject();

private:
    Q_OBJECT

    program_window*  m_wnd;
    sketch_batch_props*  m_props;
    bool  m_ok;
    QPushButton*  m_widget_ok;

    // COMMON WIDGETS

    QLineEdit*  m_widget_colour_red;
    QLineEdit*  m_widget_colour_green;
    QLineEdit*  m_widget_colour_blue;
    QLineEdit*  m_widget_colour_alpha;

    QComboBox*  m_widget_fog_type;

    QCheckBox* m_widget_wireframe;

    // WIDGETS FOR BOX

    QLineEdit*  m_widget_box_half_size_along_x_axis;
    QLineEdit*  m_widget_box_half_size_along_y_axis;
    QLineEdit*  m_widget_box_half_size_along_z_axis;

    // WIDGETS FOR CAPSULE

    QLineEdit*  m_widget_capsule_half_distance_between_end_points;
    QLineEdit*  m_widget_capsule_thickness_from_central_line;

    // WIDGETS FOR SPHERE

    QLineEdit*  m_widget_sphere_radius;

    // WIDGETS FOR TRIANGLE MESH & CONVEX HULL

    QLineEdit*  m_widget_triangle_mesh_buffers_directory;
};


}


#endif
