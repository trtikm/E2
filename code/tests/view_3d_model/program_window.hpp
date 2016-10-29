#ifndef PROGRAM_WINDOW_HPP_INCLUDED
#   define PROGRAM_WINDOW_HPP_INCLUDED

#   include "./simulator.hpp"
#   include <qtgl/window.hpp>
#   include <qtgl/buffer.hpp>
#   include <angeo/tensor_math.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <boost/filesystem/path.hpp>
#   include <QMainWindow>
#   include <QSplitter>
#   include <QTabWidget>
#   include <QLabel>
#   include <QLineEdit>
#   include <QListView>
#   include <QTextEdit>
#   include <QCheckBox>
#   include <QAction>
#   include <QColor>
#   include <QStringListModel>


struct program_window : public QMainWindow
{
    program_window(boost::property_tree::ptree* const  ptree_ptr);
    ~program_window();

    boost::property_tree::ptree&  ptree() { return *m_ptree; }

    boost::filesystem::path const&  batches_root_dir() const noexcept { return m_batches_root_dir; }
    boost::filesystem::path&  batches_root_dir() { return m_batches_root_dir; }
    boost::filesystem::path const&  batches_last_dir() const noexcept { return m_batches_last_dir; }
    boost::filesystem::path&  batches_last_dir() { return m_batches_last_dir; }

    boost::filesystem::path const&  buffers_root_dir() const noexcept { return m_buffers_root_dir; }
    boost::filesystem::path&  buffers_root_dir() { return m_buffers_root_dir; }

    boost::filesystem::path const&  shaders_root_dir() const noexcept { return m_shaders_root_dir; }
    boost::filesystem::path&  shaders_root_dir() { return m_shaders_root_dir; }

    boost::filesystem::path const&  textures_root_dir() const noexcept { return m_textures_root_dir; }
    boost::filesystem::path&  textures_root_dir() { return m_textures_root_dir; }

public slots:

    void  on_tab_changed(int const  tab_index);

    void  on_clear_colour_changed();
    void  on_clear_colour_set(QColor const&  colour);
    void  on_clear_colour_choose();
    void  on_clear_colour_reset();

    void  on_camera_pos_changed();
    void  camera_position_listener();

    void  on_camera_rot_changed();
    void  on_camera_rot_tait_bryan_changed();
    void  camera_rotation_listener();
    void  update_camera_rot_widgets(quaternion const&  q);

    void  on_batches_root_dir_changed();
    void  on_batch_select_root_dir();
    void  on_batch_insert();
    void  on_batch_remove();

    void  on_buffers_root_dir_changed();
    void  on_buffers_select_root_dir();
    void  on_buffers_refresh_lists();

    void  on_shaders_root_dir_changed();
    void  on_shaders_select_root_dir();
    void  on_shaders_refresh_lists();

    void  on_textures_root_dir_changed();
    void  on_textures_select_root_dir();
    void  on_textures_refresh_lists();

private:

    bool  event(QEvent* const event);
    void  timerEvent(QTimerEvent* const event);
    void  closeEvent(QCloseEvent* const  event);

    void  on_batch_process_newly_inserted();

    Q_OBJECT

    boost::property_tree::ptree*  m_ptree;

    bool  m_has_focus;
    int  m_idleTimerId;

    qtgl::window<simulator>  m_glwindow;

    //QAction*  m_menu_file_open_

    QSplitter*  m_splitter;
    QTabWidget*  m_tabs;

    QLineEdit*  m_clear_colour_component_red;
    QLineEdit*  m_clear_colour_component_green;
    QLineEdit*  m_clear_colour_component_blue;

    QLineEdit*  m_camera_pos_x;
    QLineEdit*  m_camera_pos_y;
    QLineEdit*  m_camera_pos_z;

    QLineEdit*  m_camera_rot_w;
    QLineEdit*  m_camera_rot_x;
    QLineEdit*  m_camera_rot_y;
    QLineEdit*  m_camera_rot_z;

    QLineEdit*  m_camera_yaw;
    QLineEdit*  m_camera_pitch;
    QLineEdit*  m_camera_roll;

    QCheckBox*  m_camera_save_pos_rot;

    boost::filesystem::path  m_batches_root_dir;
    boost::filesystem::path  m_batches_last_dir;
    QLineEdit*  m_batches_root_dir_edit;
    QListView*  m_batches_list_view;
    QStringListModel*  m_batches_list;
    QStringListModel*  m_batches_failed_list;
    std::vector<boost::filesystem::path>  m_new_batches_chache;

    boost::filesystem::path  m_buffers_root_dir;
    QLineEdit*  m_buffers_root_dir_edit;
    QStringListModel*  m_buffers_cached_list;
    QStringListModel*  m_buffers_failed_list;

    boost::filesystem::path  m_shaders_root_dir;
    QLineEdit*  m_shaders_root_dir_edit;
    QStringListModel*  m_shaders_cached_list;
    QStringListModel*  m_shaders_failed_list;

    boost::filesystem::path  m_textures_root_dir;
    QLineEdit*  m_textures_root_dir_edit;
    QStringListModel*  m_textures_cached_list;
    QStringListModel*  m_textures_failed_list;
};


#endif
