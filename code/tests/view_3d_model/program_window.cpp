#include "./program_window.hpp"
#include "./program_info.hpp"
#include "./simulator.hpp"
#include "./widgets.hpp"
#include <qtgl/gui_utils.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/msgstream.hpp>
#include <utility/canonical_path.hpp>
#include <QString>
#include <QMenuBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QTreeView>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QPushButton>
#include <QColorDialog>
#include <QIcon>
#include <QFileDialog>
#include <boost/filesystem.hpp>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace tab_names { namespace {


inline std::string  BATCH() noexcept { return "Batch"; }
inline std::string  BUFFER() noexcept { return "Buffer"; }
inline std::string  SHADER() noexcept { return "Shader"; }
inline std::string  TEXTURE() noexcept { return "Texture"; }
inline std::string  CAMERA() noexcept { return "Camera"; }
inline std::string  DRAW() noexcept { return "Draw"; }


}}


program_window::program_window(boost::property_tree::ptree* const  ptree_ptr)
    : QMainWindow(nullptr)

    , m_ptree(ptree_ptr)

    , m_has_focus(false)
    , m_idleTimerId(-1)

    , m_glwindow( vector3(0.25f,0.25f,0.25f) )

    , m_splitter( new QSplitter(Qt::Horizontal,this) )
    , m_tabs(
            [](program_window* wnd) {
                    struct s : public QTabWidget {
                        s(program_window* wnd) : QTabWidget()
                        {
                            connect(this , SIGNAL(currentChanged(int)),wnd,SLOT(on_tab_changed(int)));
                        }
                    };
                    return new s(wnd);
            }(this)
            )

    , m_clear_colour_component_red(
            [](program_window* wnd) {
                    struct s : public QLineEdit {
                        s(program_window* wnd) : QLineEdit()
                        {
                            setValidator( new QIntValidator(0,255) );
                            setText(QString::number(wnd->ptree().get("tabs.window.clear_colour.red",64)));
                            QObject::connect(this,SIGNAL(editingFinished()),wnd,SLOT(on_clear_colour_changed()));
                        }
                    };
                    return new s(wnd);
            }(this)
            )
    , m_clear_colour_component_green(
            [](program_window* wnd) {
                    struct s : public QLineEdit {
                        s(program_window* wnd) : QLineEdit()
                        {
                            setValidator( new QIntValidator(0,255) );
                            setText(QString::number(wnd->ptree().get("tabs.window.clear_colour.green",64)));
                            QObject::connect(this,SIGNAL(editingFinished()),wnd,SLOT(on_clear_colour_changed()));
                        }
                    };
                    return new s(wnd);
            }(this)
            )
    , m_clear_colour_component_blue(
            [](program_window* wnd) {
                    struct s : public QLineEdit {
                        s(program_window* wnd) : QLineEdit()
                        {
                            setValidator( new QIntValidator(0,255) );
                            setText(QString::number(wnd->ptree().get("tabs.window.clear_colour.blue",64)));
                            QObject::connect(this,SIGNAL(editingFinished()),wnd,SLOT(on_clear_colour_changed()));
                        }
                    };
                    return new s(wnd);
            }(this)
            )

    , m_camera_pos_x(
            [](program_window* wnd) {
                    struct s : public QLineEdit {
                        s(program_window* wnd) : QLineEdit()
                        {
                            setText(QString::number(wnd->ptree().get("tabs.camera.pos.x",0.5f)));
                            QObject::connect(this,SIGNAL(editingFinished()),wnd,SLOT(on_camera_pos_changed()));
                        }
                    };
                    return new s(wnd);
            }(this)
            )
    , m_camera_pos_y(
            [](program_window* wnd) {
                    struct s : public QLineEdit {
                        s(program_window* wnd) : QLineEdit()
                        {
                            setText(QString::number(wnd->ptree().get("tabs.camera.pos.y",0.5f)));
                            QObject::connect(this,SIGNAL(editingFinished()),wnd,SLOT(on_camera_pos_changed()));
                        }
                    };
                    return new s(wnd);
            }(this)
            )
    , m_camera_pos_z(
            [](program_window* wnd) {
                    struct s : public QLineEdit {
                        s(program_window* wnd) : QLineEdit()
                        {
                            setText(QString::number(wnd->ptree().get("tabs.camera.pos.z",2.0f)));
                            QObject::connect(this,SIGNAL(editingFinished()),wnd,SLOT(on_camera_pos_changed()));
                        }
                    };
                    return new s(wnd);
            }(this)
            )

    , m_camera_rot_w(
            [](program_window* wnd) {
                    struct s : public QLineEdit {
                        s(program_window* wnd) : QLineEdit()
                        {
                            setText(QString::number(wnd->ptree().get("tabs.camera.rot.w",1.0f)));
                            QObject::connect(this,SIGNAL(editingFinished()),wnd,SLOT(on_camera_rot_changed()));
                        }
                    };
                    return new s(wnd);
            }(this)
            )
    , m_camera_rot_x(
            [](program_window* wnd) {
                    struct s : public QLineEdit {
                        s(program_window* wnd) : QLineEdit()
                        {
                            setText(QString::number(wnd->ptree().get("tabs.camera.rot.x",0.0f)));
                            QObject::connect(this,SIGNAL(editingFinished()),wnd,SLOT(on_camera_rot_changed()));
                        }
                    };
                    return new s(wnd);
            }(this)
            )
    , m_camera_rot_y(
            [](program_window* wnd) {
                    struct s : public QLineEdit {
                        s(program_window* wnd) : QLineEdit()
                        {
                            setText(QString::number(wnd->ptree().get("tabs.camera.rot.y",0.0f)));
                            QObject::connect(this,SIGNAL(editingFinished()),wnd,SLOT(on_camera_rot_changed()));
                        }
                    };
                    return new s(wnd);
            }(this)
            )
    , m_camera_rot_z(
            [](program_window* wnd) {
                    struct s : public QLineEdit {
                        s(program_window* wnd) : QLineEdit()
                        {
                            setText(QString::number(wnd->ptree().get("tabs.camera.rot.z",0.0f)));
                            QObject::connect(this,SIGNAL(editingFinished()),wnd,SLOT(on_camera_rot_changed()));
                        }
                    };
                    return new s(wnd);
            }(this)
            )

    , m_camera_yaw(
            [](program_window* wnd) {
                    struct s : public QLineEdit {
                        s(program_window* wnd) : QLineEdit()
                        {
                            QObject::connect(this,SIGNAL(editingFinished()),wnd,SLOT(on_camera_rot_tait_bryan_changed()));
                        }
                    };
                    return new s(wnd);
            }(this)
            )
    , m_camera_pitch(
            [](program_window* wnd) {
                    struct s : public QLineEdit {
                        s(program_window* wnd) : QLineEdit()
                        {
                            QObject::connect(this,SIGNAL(editingFinished()),wnd,SLOT(on_camera_rot_tait_bryan_changed()));
                        }
                    };
                    return new s(wnd);
            }(this)
            )
    , m_camera_roll(
            [](program_window* wnd) {
                    struct s : public QLineEdit {
                        s(program_window* wnd) : QLineEdit()
                        {
                            QObject::connect(this,SIGNAL(editingFinished()),wnd,SLOT(on_camera_rot_tait_bryan_changed()));
                        }
                    };
                    return new s(wnd);
            }(this)
            )

    , m_camera_save_pos_rot(new QCheckBox("Save position and rotation"))

    , m_batches_root_dir(ptree().get("tabs.batch.root_dir", boost::filesystem::current_path().string()))
    , m_batches_last_dir(ptree().get("tabs.batch.last_dir", boost::filesystem::current_path().string()))
    , m_batches_root_dir_edit(
            [](program_window* wnd) {
                    struct s : public QLineEdit {
                        s(program_window* wnd) : QLineEdit()
                        {
                            setText(QString(wnd->batches_root_dir().string().c_str()));
                            QObject::connect(this,SIGNAL(editingFinished()),wnd,SLOT(on_batches_root_dir_changed()));
                        }
                    };
                    return new s(wnd);
            }(this)
            )
    , m_batches_list_view(new QListView)
    , m_batches_list(new QStringListModel)
//            [](program_window* wnd, qtgl::window<simulator>* const glwindow) {
//                    struct s : public QStringListModel {
//                        s(program_window* wnd, qtgl::window<simulator>* const glwindow) : QStringListModel()
//                        {
//                            QDir const  dir(wnd->batches_root_dir().string().c_str());
//                            boost::property_tree::ptree  empty;
//                            for (boost::property_tree::ptree::value_type const&  key_value :
//                                 wnd->ptree().get_child("tabs.batch.loaded", empty))
//                            {
//                                glwindow->call_later(&simulator::insert_batch,key_value.second.data());

//                                this->insertRow(this->rowCount());
//                                QModelIndex const  index = this->index(this->rowCount()-1);
//                                this->setData(index,dir.relativeFilePath(key_value.second.data().c_str()));
//                            }
//                            this->sort(0);
//                        }
//                    };
//                    return new s(wnd,glwindow);
//            }(this,&m_glwindow)
//            )
    , m_batches_failed_list(new QStringListModel)
    , m_new_batches_chache(
            [](boost::property_tree::ptree const&  props, qtgl::window<simulator>* const glwindow)
                    -> std::vector<boost::filesystem::path> {
                std::vector<boost::filesystem::path>  result;
                boost::property_tree::ptree  empty;
                for (boost::property_tree::ptree::value_type const&  key_value :
                     props.get_child("tabs.batch.loaded", empty))
                {
                    glwindow->call_later(&simulator::insert_batch,key_value.second.data());
                    result.push_back(key_value.second.data());
                }
                return result;
            }(*ptree_ptr,&m_glwindow)
            )

    , m_buffers_root_dir(ptree().get("tabs.buffer.root_dir", boost::filesystem::current_path().string()))
    , m_buffers_root_dir_edit(
            [](program_window* wnd) {
                    struct s : public QLineEdit {
                        s(program_window* wnd) : QLineEdit()
                        {
                            setText(QString(wnd->buffers_root_dir().string().c_str()));
                            QObject::connect(this,SIGNAL(editingFinished()),wnd,SLOT(on_buffers_root_dir_changed()));
                        }
                    };
                    return new s(wnd);
            }(this)
            )
    , m_buffers_cached_list(new QStringListModel)
    , m_buffers_failed_list(new QStringListModel)

    , m_shaders_root_dir(ptree().get("tabs.shader.root_dir", boost::filesystem::current_path().string()))
    , m_shaders_root_dir_edit(
            [](program_window* wnd) {
                    struct s : public QLineEdit {
                        s(program_window* wnd) : QLineEdit()
                        {
                            setText(QString(wnd->shaders_root_dir().string().c_str()));
                            QObject::connect(this,SIGNAL(editingFinished()),wnd,SLOT(on_shaders_root_dir_changed()));
                        }
                    };
                    return new s(wnd);
            }(this)
            )
    , m_shaders_cached_list(new QStringListModel)
    , m_shaders_failed_list(new QStringListModel)

    , m_textures_root_dir(ptree().get("tabs.texture.root_dir", boost::filesystem::current_path().string()))
    , m_textures_root_dir_edit(
            [](program_window* wnd) {
                    struct s : public QLineEdit {
                        s(program_window* wnd) : QLineEdit()
                        {
                            setText(QString(wnd->textures_root_dir().string().c_str()));
                            QObject::connect(this,SIGNAL(editingFinished()),wnd,SLOT(on_textures_root_dir_changed()));
                        }
                    };
                    return new s(wnd);
            }(this)
            )
    , m_textures_cached_list(new QStringListModel)
    , m_textures_failed_list(new QStringListModel)
{
    this->setWindowTitle(get_program_name().c_str());
    this->setWindowIcon(QIcon("../data/shared/gfx/icons/E2_icon.png"));
    this->move({ptree().get("program_window.pos.x",0),ptree().get("program_window.pos.y",0)});
    this->resize(ptree().get("program_window.width",1024),ptree().get("program_window.height",768));
    this->setCentralWidget(m_splitter);

//    QMenuBar* pMenuBar = new QMenuBar(this);
//    setMenuBar(pMenuBar);
//    INVARIANT(menuBar() == pMenuBar);
//    QMenu* menu = new QMenu("File", this);
//    menuBar()->addMenu(menu);
//    menu->addAction("Opne");
//    menuBar()->addMenu(new QMenu("Edit", this));
//    menuBar()->addMenu(new QMenu("View", this));
//    menuBar()->addMenu(new QMenu("Tools", this));
//    menuBar()->addMenu(new QMenu("Help", this));

    QWidget* const  gl_window_widget = m_glwindow.create_widget_container();

    m_splitter->addWidget(gl_window_widget);
    m_splitter->addWidget(m_tabs);

    // Building batches tab
    {
        QWidget* const  batch_tab = new QWidget;
        {
            QVBoxLayout* const batch_tab_layout = new QVBoxLayout;
            {
                batch_tab_layout->addWidget(new QLabel("Batch root directory:"),0);
                QHBoxLayout* const root_dir_layout = new QHBoxLayout;
                {
                    root_dir_layout->addWidget(m_batches_root_dir_edit);
                    root_dir_layout->addWidget(
                            [](program_window* wnd) {
                                    struct choose : public QPushButton {
                                        choose(program_window* wnd) : QPushButton("Choose")
                                        {
                                            QObject::connect(this,SIGNAL(released()),wnd,SLOT(on_batch_select_root_dir()));
                                        }
                                    };
                                    return new choose(wnd);
                            }(this)
                            );
                }
                batch_tab_layout->addLayout(root_dir_layout,0);

                QHBoxLayout* const buttons_layout = new QHBoxLayout;
                {
                    buttons_layout->addWidget(
                            [](program_window* wnd) {
                                    struct choose : public QPushButton {
                                        choose(program_window* wnd) : QPushButton("Insert")
                                        {
                                            QObject::connect(this,SIGNAL(released()),wnd,SLOT(on_batch_insert()));
                                        }
                                    };
                                    return new choose(wnd);
                            }(this)
                            );
                    buttons_layout->addWidget(
                            [](program_window* wnd) {
                                    struct choose : public QPushButton {
                                        choose(program_window* wnd) : QPushButton("Remove")
                                        {
                                            QObject::connect(this,SIGNAL(released()),wnd,SLOT(on_batch_remove()));
                                        }
                                    };
                                    return new choose(wnd);
                            }(this)
                            );
                }
                batch_tab_layout->addLayout(buttons_layout,0);

                batch_tab_layout->addWidget(new QLabel("Cached batches:"),0);
                m_batches_list_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
                m_batches_list_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
                m_batches_list_view->setModel(m_batches_list);
                batch_tab_layout->addWidget(m_batches_list_view,5);

                batch_tab_layout->addWidget(new QLabel("Failed batches:"),0);
                QListView*  batches_failed_list_view = new QListView;
                batches_failed_list_view->setSelectionMode(QAbstractItemView::NoSelection);
                batches_failed_list_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
                batches_failed_list_view->setModel(m_batches_failed_list);
                batch_tab_layout->addWidget(batches_failed_list_view,0);
            }
            batch_tab->setLayout(batch_tab_layout);
        }
        m_tabs->addTab(batch_tab,QString(tab_names::BATCH().c_str()));
    }

    // Building buffer tab
    {
        QWidget* const  buffer_tab = new QWidget;
        {
            QVBoxLayout* const buffer_tab_layout = new QVBoxLayout;
            {
                buffer_tab_layout->addWidget(new QLabel("Buffer root directory:"),0);
                QHBoxLayout* const root_dir_layout = new QHBoxLayout;
                {
                    root_dir_layout->addWidget(m_buffers_root_dir_edit);
                    root_dir_layout->addWidget(
                            [](program_window* wnd) {
                                    struct choose : public QPushButton {
                                        choose(program_window* wnd) : QPushButton("Choose")
                                        {
                                            QObject::connect(this,SIGNAL(released()),wnd,SLOT(on_buffers_select_root_dir()));
                                        }
                                    };
                                    return new choose(wnd);
                            }(this)
                            );
                }
                buffer_tab_layout->addLayout(root_dir_layout,0);

                buffer_tab_layout->addWidget(new QLabel("Cached buffers:"),0);
                QListView*  buffers_cached_list_view = new QListView;
                buffers_cached_list_view->setSelectionMode(QAbstractItemView::NoSelection);
                buffers_cached_list_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
                buffers_cached_list_view->setModel(m_buffers_cached_list);
                buffer_tab_layout->addWidget(buffers_cached_list_view,5);

                buffer_tab_layout->addWidget(new QLabel("Failed buffers:"),0);
                QListView*  buffers_failed_list_view = new QListView;
                buffers_failed_list_view->setSelectionMode(QAbstractItemView::NoSelection);
                buffers_failed_list_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
                buffers_failed_list_view->setModel(m_buffers_failed_list);
                buffer_tab_layout->addWidget(buffers_failed_list_view,0);

//                buffer_tab_layout->addWidget(
//                        [](program_window* wnd) {
//                                struct choose : public QPushButton {
//                                    choose(program_window* wnd) : QPushButton("Refresh")
//                                    {
//                                        QObject::connect(this,SIGNAL(released()),wnd,SLOT(on_buffers_refresh_lists()));
//                                    }
//                                };
//                                return new choose(wnd);
//                        }(this),0
//                        );

            }
            buffer_tab->setLayout(buffer_tab_layout);
        }
        m_tabs->addTab(buffer_tab,QString(tab_names::BUFFER().c_str()));
    }

    // Building Shader tab
    {
        QWidget* const  shader_tab = new QWidget;
        {
            QVBoxLayout* const shader_tab_layout = new QVBoxLayout;
            {
                shader_tab_layout->addWidget(new QLabel("Shader root directory:"),0);
                QHBoxLayout* const root_dir_layout = new QHBoxLayout;
                {
                    root_dir_layout->addWidget(m_shaders_root_dir_edit);
                    root_dir_layout->addWidget(
                            [](program_window* wnd) {
                                    struct choose : public QPushButton {
                                        choose(program_window* wnd) : QPushButton("Choose")
                                        {
                                            QObject::connect(this,SIGNAL(released()),wnd,SLOT(on_shaders_select_root_dir()));
                                        }
                                    };
                                    return new choose(wnd);
                            }(this)
                            );
                }
                shader_tab_layout->addLayout(root_dir_layout,0);

                shader_tab_layout->addWidget(new QLabel("Cached shaders:"),0);
                QListView*  shaders_cached_list_view = new QListView;
                shaders_cached_list_view->setSelectionMode(QAbstractItemView::NoSelection);
                shaders_cached_list_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
                shaders_cached_list_view->setModel(m_shaders_cached_list);
                shader_tab_layout->addWidget(shaders_cached_list_view,5);

                shader_tab_layout->addWidget(new QLabel("Failed shaders:"),0);
                QListView*  shaders_failed_list_view = new QListView;
                shaders_failed_list_view->setSelectionMode(QAbstractItemView::NoSelection);
                shaders_failed_list_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
                shaders_failed_list_view->setModel(m_shaders_failed_list);
                shader_tab_layout->addWidget(shaders_failed_list_view,0);

//                shader_tab_layout->addWidget(
//                        [](program_window* wnd) {
//                                struct choose : public QPushButton {
//                                    choose(program_window* wnd) : QPushButton("Refresh")
//                                    {
//                                        QObject::connect(this,SIGNAL(released()),wnd,SLOT(on_shaders_refresh_lists()));
//                                    }
//                                };
//                                return new choose(wnd);
//                        }(this),0
//                        );

            }
            shader_tab->setLayout(shader_tab_layout);
        }
        m_tabs->addTab(shader_tab,QString(tab_names::SHADER().c_str()));
    }

    // Building Texture tab
    {
        QWidget* const  texture_tab = new QWidget;
        {
            QVBoxLayout* const texture_tab_layout = new QVBoxLayout;
            {
                texture_tab_layout->addWidget(new QLabel("texture root directory:"),0);
                QHBoxLayout* const root_dir_layout = new QHBoxLayout;
                {
                    root_dir_layout->addWidget(m_textures_root_dir_edit);
                    root_dir_layout->addWidget(
                            [](program_window* wnd) {
                                    struct choose : public QPushButton {
                                        choose(program_window* wnd) : QPushButton("Choose")
                                        {
                                            QObject::connect(this,SIGNAL(released()),wnd,SLOT(on_textures_select_root_dir()));
                                        }
                                    };
                                    return new choose(wnd);
                            }(this)
                            );
                }
                texture_tab_layout->addLayout(root_dir_layout,0);

                texture_tab_layout->addWidget(new QLabel("Cached textures:"),0);
                QListView*  textures_cached_list_view = new QListView;
                textures_cached_list_view->setSelectionMode(QAbstractItemView::NoSelection);
                textures_cached_list_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
                textures_cached_list_view->setModel(m_textures_cached_list);
                texture_tab_layout->addWidget(textures_cached_list_view,5);

                texture_tab_layout->addWidget(new QLabel("Failed textures:"),0);
                QListView*  textures_failed_list_view = new QListView;
                textures_failed_list_view->setSelectionMode(QAbstractItemView::NoSelection);
                textures_failed_list_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
                textures_failed_list_view->setModel(m_textures_failed_list);
                texture_tab_layout->addWidget(textures_failed_list_view,0);

//                texture_tab_layout->addWidget(
//                        [](program_window* wnd) {
//                                struct choose : public QPushButton {
//                                    choose(program_window* wnd) : QPushButton("Refresh")
//                                    {
//                                        QObject::connect(this,SIGNAL(released()),wnd,SLOT(on_textures_refresh_lists()));
//                                    }
//                                };
//                                return new choose(wnd);
//                        }(this),0
//                        );

            }
            texture_tab->setLayout(texture_tab_layout);
        }
        m_tabs->addTab(texture_tab,QString(tab_names::TEXTURE().c_str()));
    }

    // Building Camera tab
    {
        QWidget* const  camera_tab = new QWidget;
        {
            QVBoxLayout* const camera_tab_layout = new QVBoxLayout;
            {
                QWidget* const position_group = new QGroupBox("Position in meters [xyz]");
                {
                    QHBoxLayout* const position_layout = new QHBoxLayout;
                    {
                        position_layout->addWidget(m_camera_pos_x);
                        position_layout->addWidget(m_camera_pos_y);
                        position_layout->addWidget(m_camera_pos_z);
                        on_camera_pos_changed();
                        m_glwindow.register_listener(notifications::camera_position_updated(),
                                                     {&program_window::camera_position_listener,this});
                    }
                    position_group->setLayout(position_layout);
                }
                camera_tab_layout->addWidget(position_group);

                QWidget* const rotation_group = new QGroupBox("Rotation");
                {
                    QVBoxLayout* const rotation_layout = new QVBoxLayout;
                    {
                        rotation_layout->addWidget(new QLabel("Quaternion [wxyz]"));
                        QHBoxLayout* const quaternion_layout = new QHBoxLayout;
                        {
                            quaternion_layout->addWidget(m_camera_rot_w);
                            quaternion_layout->addWidget(m_camera_rot_x);
                            quaternion_layout->addWidget(m_camera_rot_y);
                            quaternion_layout->addWidget(m_camera_rot_z);
                        }
                        rotation_layout->addLayout(quaternion_layout);

                        rotation_layout->addWidget(new QLabel("Tait-Bryan angles in degrees [yaw(z)-pitch(y')-roll(x'')]"));
                        QHBoxLayout* const tait_bryan_layout = new QHBoxLayout;
                        {
                            tait_bryan_layout->addWidget(m_camera_yaw);
                            tait_bryan_layout->addWidget(m_camera_pitch);
                            tait_bryan_layout->addWidget(m_camera_roll);
                        }
                        rotation_layout->addLayout(tait_bryan_layout);
                    }
                    rotation_group->setLayout(rotation_layout);

                    on_camera_rot_changed();
                    m_glwindow.register_listener(notifications::camera_orientation_updated(),
                                                 {&program_window::camera_rotation_listener,this});
                }
                camera_tab_layout->addWidget(rotation_group);

                m_camera_save_pos_rot->setCheckState(
                            ptree().get("tabs.camera.save_pos_rot",true) ? Qt::CheckState::Checked :
                                                                           Qt::CheckState::Unchecked
                            );
                camera_tab_layout->addWidget(m_camera_save_pos_rot);

                camera_tab_layout->addStretch(1);
            }
            camera_tab->setLayout(camera_tab_layout);
        }
        m_tabs->addTab(camera_tab,QString(tab_names::CAMERA().c_str()));
    }

    // Building Draw tab
    {
        QWidget* const  draw_tab = new QWidget;
        {
            QVBoxLayout* const draw_tab_layout = new QVBoxLayout;
            {
                QWidget* const clear_colour_group = new QGroupBox("Clear colour [rgb]");
                {
                    QVBoxLayout* const clear_colour_layout = new QVBoxLayout;
                    {
                        QHBoxLayout* const edit_boxes_layout = new QHBoxLayout;
                        {
                            edit_boxes_layout->addWidget(m_clear_colour_component_red);
                            edit_boxes_layout->addWidget(m_clear_colour_component_green);
                            edit_boxes_layout->addWidget(m_clear_colour_component_blue);
                            on_clear_colour_changed();
                        }
                        clear_colour_layout->addLayout(edit_boxes_layout);

                        QHBoxLayout* const buttons_layout = new QHBoxLayout;
                        {
                            buttons_layout->addWidget(
                                    [](program_window* wnd) {
                                            struct choose : public QPushButton {
                                                choose(program_window* wnd) : QPushButton("Choose")
                                                {
                                                    QObject::connect(this,SIGNAL(released()),wnd,SLOT(on_clear_colour_choose()));
                                                }
                                            };
                                            return new choose(wnd);
                                    }(this)
                                    );
                            buttons_layout->addWidget(
                                    [](program_window* wnd) {
                                            struct choose : public QPushButton {
                                                choose(program_window* wnd) : QPushButton("Default")
                                                {
                                                    QObject::connect(this,SIGNAL(released()),wnd,SLOT(on_clear_colour_reset()));
                                                }
                                            };
                                            return new choose(wnd);
                                    }(this)
                                    );
                        }
                        clear_colour_layout->addLayout(buttons_layout);

                    }
                    clear_colour_group->setLayout(clear_colour_layout);
                }
                draw_tab_layout->addWidget(clear_colour_group);
                draw_tab_layout->addStretch(1);
            }
            draw_tab->setLayout(draw_tab_layout);
        }
        m_tabs->addTab(draw_tab,QString(tab_names::DRAW().c_str()));
    }

    statusBar()->addPermanentWidget(
            [](qtgl::window<simulator>* const glwindow){
                struct s : public qtgl::widget_base<s,qtgl::window<simulator> >, public QLabel {
                    s(qtgl::window<simulator>* const  glwindow)
                        : qtgl::widget_base<s,qtgl::window<simulator> >(glwindow), QLabel()
                    {
                        setText("FPS: 0");
                        register_listener(qtgl::notifications::fps_updated(),
                                          &s::on_fps_changed);
                    }
                    void  on_fps_changed()
                    {
                        std::stringstream  sstr;
                        sstr << "FPS: " << call_now(&qtgl::real_time_simulator::FPS);
                        setText(sstr.str().c_str());
                    }
                };
                return new s(glwindow);
            }(&m_glwindow)
            );
    statusBar()->showMessage("Ready", 2000);

    if (ptree().get("program_window.show_maximised",false))
        this->showMaximized();
    else
        this->show();

//    this->menuBar()->show();
//    this->statusBar()->show();

    qtgl::set_splitter_sizes(*m_splitter,ptree().get("program_window.splitter_ratio",3.0f / 4.0f));

    gl_window_widget->setFocus();

    m_idleTimerId = startTimer(250); // In milliseconds.
}

program_window::~program_window()
{
}

bool program_window::event(QEvent* const event)
{
    switch (event->type())
    {
        case QEvent::FocusIn:
            m_has_focus = true;
            return QMainWindow::event(event);
        case QEvent::FocusOut:
            m_has_focus = false;
            return QMainWindow::event(event);
        default:
            return QMainWindow::event(event);
    }
}

void program_window::timerEvent(QTimerEvent* const event)
{
    if (event->timerId() != m_idleTimerId)
        return;

    on_batch_process_newly_inserted();
}

void  program_window::closeEvent(QCloseEvent* const  event)
{
    ptree().put("program_window.pos.x", pos().x());
    ptree().put("program_window.pos.y", pos().y());
    ptree().put("program_window.width", width());
    ptree().put("program_window.height", height());
    ptree().put("program_window.splitter_ratio", qtgl::get_splitter_sizes_ratio(*m_splitter));
    ptree().put("program_window.show_maximised", isMaximized());

    ptree().put("tabs.window.clear_colour.red", m_clear_colour_component_red->text().toInt());
    ptree().put("tabs.window.clear_colour.green", m_clear_colour_component_green->text().toInt());
    ptree().put("tabs.window.clear_colour.blue", m_clear_colour_component_blue->text().toInt());

    if (m_camera_save_pos_rot->isChecked())
    {
        ptree().put("tabs.camera.pos.x", m_camera_pos_x->text().toFloat());
        ptree().put("tabs.camera.pos.y", m_camera_pos_y->text().toFloat());
        ptree().put("tabs.camera.pos.z", m_camera_pos_z->text().toFloat());
        ptree().put("tabs.camera.rot.w", m_camera_rot_w->text().toFloat());
        ptree().put("tabs.camera.rot.x", m_camera_rot_x->text().toFloat());
        ptree().put("tabs.camera.rot.y", m_camera_rot_y->text().toFloat());
        ptree().put("tabs.camera.rot.z", m_camera_rot_z->text().toFloat());
    }
    ptree().put("tabs.camera.save_pos_rot", m_camera_save_pos_rot->isChecked());

    ptree().put("tabs.batch.root_dir", batches_root_dir().string());
    ptree().put("tabs.batch.last_dir", batches_last_dir().string());
    boost::property_tree::ptree  loaded;
    {
        QStringList const  paths = m_batches_list->stringList();
        for (int i = 0; i < paths.size(); ++i)
        {
            boost::filesystem::path  pathname = canonical_path(batches_root_dir() / qtgl::to_string(paths.at(i)));
            loaded.push_back({"",boost::property_tree::ptree(pathname.string())});
        }
    }
    ptree().put_child("tabs.batch.loaded",loaded);

    ptree().put("tabs.buffer.root_dir", buffers_root_dir().string());
    ptree().put("tabs.shader.root_dir", shaders_root_dir().string());
    ptree().put("tabs.texture.root_dir", textures_root_dir().string());
}


void  program_window::on_tab_changed(int const  tab_index)
{
    std::string const  tab_name = qtgl::to_string(m_tabs->tabText(tab_index));
    //if (tab_name == tab_names::BUFFER())
    //    on_buffers_refresh_lists();
    //else
    //if (tab_name == tab_names::SHADER())
    //    on_shaders_refresh_lists();
    //else
    if (tab_name == tab_names::TEXTURE())
        on_textures_refresh_lists();
}

void program_window::on_clear_colour_changed()
{
    vector3 const  colour(
                (float_32_bit)m_clear_colour_component_red->text().toInt() / 255.0f,
                (float_32_bit)m_clear_colour_component_green->text().toInt() / 255.0f,
                (float_32_bit)m_clear_colour_component_blue->text().toInt() / 255.0f
                );
    m_glwindow.call_later(&simulator::set_clear_color,colour);
}

void program_window::on_clear_colour_set(QColor const&  colour)
{
    m_clear_colour_component_red->setText(QString::number(colour.red()));
    m_clear_colour_component_green->setText(QString::number(colour.green()));
    m_clear_colour_component_blue->setText(QString::number(colour.blue()));
    on_clear_colour_changed();
}

void program_window::on_clear_colour_choose()
{
    QColor const  init_colour(m_clear_colour_component_red->text().toInt(),
                              m_clear_colour_component_green->text().toInt(),
                              m_clear_colour_component_blue->text().toInt());
    QColor const  colour = QColorDialog::getColor(init_colour,this,"Choose clear colour");
    if (!colour.isValid())
        return;
    on_clear_colour_set(colour);
}

void program_window::on_clear_colour_reset()
{
    on_clear_colour_set(QColor(64,64,64));
}

void  program_window::on_camera_pos_changed()
{
    vector3 const  pos(m_camera_pos_x->text().toFloat(),
                       m_camera_pos_y->text().toFloat(),
                       m_camera_pos_z->text().toFloat());
    m_glwindow.call_later(&simulator::set_camera_position,pos);
}

void  program_window::camera_position_listener()
{
    vector3 const pos = m_glwindow.call_now(&simulator::get_camera_position);
    m_camera_pos_x->setText(QString::number(pos(0)));
    m_camera_pos_y->setText(QString::number(pos(1)));
    m_camera_pos_z->setText(QString::number(pos(2)));
}

void  program_window::on_camera_rot_changed()
{
    quaternion  q(m_camera_rot_w->text().toFloat(),
                  m_camera_rot_x->text().toFloat(),
                  m_camera_rot_y->text().toFloat(),
                  m_camera_rot_z->text().toFloat());
    if (length_squared(q) < 1e-5f)
        q.z() = 1.0f;
    normalise(q);

    update_camera_rot_widgets(q);
    m_glwindow.call_later(&simulator::set_camera_orientation,q);
}

void  program_window::on_camera_rot_tait_bryan_changed()
{
    quaternion  q = rotation_matrix_to_quaternion(yaw_pitch_roll_to_rotation(
                            m_camera_yaw->text().toFloat() * PI() / 180.0f,
                            m_camera_pitch->text().toFloat() * PI() / 180.0f,
                            m_camera_roll->text().toFloat() * PI() / 180.0f
                            ));
    normalise(q);
    update_camera_rot_widgets(q);
    m_glwindow.call_later(&simulator::set_camera_orientation,q);
}

void  program_window::camera_rotation_listener()
{
    update_camera_rot_widgets(m_glwindow.call_now(&simulator::get_camera_orientation));
}

void  program_window::update_camera_rot_widgets(quaternion const&  q)
{
    m_camera_rot_w->setText(QString::number(q.w()));
    m_camera_rot_x->setText(QString::number(q.x()));
    m_camera_rot_y->setText(QString::number(q.y()));
    m_camera_rot_z->setText(QString::number(q.z()));

    scalar  yaw,pitch,roll;
    rotation_to_yaw_pitch_roll(quaternion_to_rotation_matrix(q),yaw,pitch,roll);
    m_camera_yaw->setText(QString::number(yaw * 180.0f / PI()));
    m_camera_pitch->setText(QString::number(pitch * 180.0f / PI()));
    m_camera_roll->setText(QString::number(roll * 180.0f / PI()));
}


void  program_window::on_batches_root_dir_changed()
{
    QDir const  dir(m_batches_root_dir_edit->text());

    QStringList  updated_paths;
    {
        QStringList const  paths = m_batches_list->stringList();
        for (int i = 0; i < paths.size(); ++i)
        {
            boost::filesystem::path  pathname = batches_root_dir() / qtgl::to_string(paths.at(i));
            updated_paths.push_back(dir.relativeFilePath(pathname.string().c_str()));
        }
    }
    m_batches_list->setStringList(updated_paths);
    m_batches_list->sort(0);

    batches_root_dir() = qtgl::to_string(dir.absolutePath());
}

void  program_window::on_batch_select_root_dir()
{
    QFileDialog  dialog(this);
    dialog.setDirectory(batches_root_dir().string().c_str());
    dialog.setFileMode(QFileDialog::DirectoryOnly);
    if (dialog.exec())
    {
        QStringList const  selected = dialog.selectedFiles();
        if (selected.size() == 1)
        {
            m_batches_root_dir_edit->setText(selected.at(0));
            on_batches_root_dir_changed();
        }
    }
}

void  program_window::on_batch_insert()
{
    QFileDialog  dialog(this);
    dialog.setDirectory(batches_last_dir().string().c_str());
    dialog.setFileMode(QFileDialog::ExistingFiles);
    //dialog.setNameFilter("*");
    if (dialog.exec())
    {
        QDir const  dir(batches_root_dir().string().c_str());
        QStringList const  selection = dialog.selectedFiles();
//        QStringList  updated = m_batches_list->stringList();
        for (int i = 0; i < selection.size(); ++i)
        {
            boost::filesystem::path const  pathname = canonical_path(qtgl::to_string(selection.at(i)));
            batches_last_dir() = pathname.parent_path();

            m_glwindow.call_later(&simulator::insert_batch,pathname);
            m_new_batches_chache.push_back(pathname);

//            QString const  relative_pathname = dir.relativeFilePath(pathname.string().c_str());
//            if (!updated.contains(relative_pathname))
//                updated.push_back(relative_pathname);
        }
//        m_batches_list->setStringList(updated);
//        m_batches_list->sort(0);
    }
}

void  program_window::on_batch_remove()
{
    QModelIndexList  selected = m_batches_list_view->selectionModel()->selectedIndexes();
    m_batches_list_view->selectionModel()->clearSelection();
    qSort(selected);
    QStringList  paths = m_batches_list->stringList();
    for (int i = selected.count() - 1; i >= 0; --i)
    {
        int const row = selected.at(i).row();
        if (row >= 0 && row < paths.size())
        {
            boost::filesystem::path  pathname = batches_root_dir() / qtgl::to_string(paths.at(row));
            m_glwindow.call_later(&simulator::erase_batch,pathname);
            paths.removeAt(row);
        }
    }
    m_batches_list->setStringList(paths);
    m_batches_list->sort(0);
}

void  program_window::on_batch_process_newly_inserted()
{
    if (m_new_batches_chache.empty())
        return;

    qtgl::make_current_window_guard const  guard(m_glwindow.make_me_current());

    std::vector<boost::filesystem::path>  successes;
    std::vector<boost::filesystem::path>  failures;
    for (natural_64_bit  i = 0U; i < m_new_batches_chache.size(); )
    {
        auto const  state = qtgl::get_batch_chache_state(m_new_batches_chache.at(i));
        INVARIANT(!state.first || !state.second);

        if (state.first)
        {
            successes.push_back(m_new_batches_chache.at(i));
            std::swap(m_new_batches_chache.at(i),m_new_batches_chache.back());
            m_new_batches_chache.pop_back();
        }
        else if (state.second)
        {
            failures.push_back(m_new_batches_chache.at(i));
            std::swap(m_new_batches_chache.at(i),m_new_batches_chache.back());
            m_new_batches_chache.pop_back();
        }
        else
            ++i;
    }

    QDir const  dir(batches_root_dir().string().c_str());

    if (!successes.empty())
    {
        QStringList  paths = m_batches_list->stringList();
        for (auto const&  path : successes)
        {
            QString const  relative_pathname = dir.relativeFilePath(path.string().c_str());
            if (!paths.contains(relative_pathname))
                paths.push_back(relative_pathname);
        }
        m_batches_list->setStringList(paths);
        m_batches_list->sort(0);
    }

    if (!failures.empty())
    {
        std::vector< std::pair<boost::filesystem::path,std::string> >  failed;
        qtgl::get_failed_batches(failed);
        QStringList  paths;
        for (auto const&  path_error : failed)
        {
            QString const  relative_pathname = dir.relativeFilePath(path_error.first.string().c_str());
            paths.push_back(relative_pathname + QString(" : ") + QString(path_error.second.c_str()));
        }
        m_batches_failed_list->setStringList(paths);
        m_batches_failed_list->sort(0);
    }
}


void  program_window::on_buffers_root_dir_changed()
{
    QDir const  dir(m_buffers_root_dir_edit->text());

    QStringList  updated_paths;
    {
        QStringList const  paths = m_buffers_cached_list->stringList();
        for (int i = 0; i < paths.size(); ++i)
        {
            boost::filesystem::path  pathname = buffers_root_dir() / qtgl::to_string(paths.at(i));
            updated_paths.push_back(dir.relativeFilePath(pathname.string().c_str()));
        }
    }
    m_buffers_cached_list->setStringList(updated_paths);
    m_buffers_cached_list->sort(0);

    buffers_root_dir() = qtgl::to_string(dir.absolutePath());
}

void  program_window::on_buffers_select_root_dir()
{
    QFileDialog  dialog(this);
    dialog.setDirectory(batches_root_dir().string().c_str());
    dialog.setFileMode(QFileDialog::DirectoryOnly);
    if (dialog.exec())
    {
        QStringList const  selected = dialog.selectedFiles();
        if (selected.size() == 1)
        {
            m_buffers_root_dir_edit->setText(selected.at(0));
            on_buffers_root_dir_changed();
        }
    }
}

//void  program_window::on_buffers_refresh_lists()
//{
//    qtgl::make_current_window_guard const  guard(m_glwindow.make_me_current());
//
//    QDir const  dir(buffers_root_dir().string().c_str());
//
//    std::vector<qtgl::buffer_properties_ptr>  cached_buffer_props;
//    qtgl::get_properties_of_cached_buffers(cached_buffer_props);
//    {
//        QStringList values;
//        for (auto const  props : cached_buffer_props)
//            values.push_back(dir.relativeFilePath(QString(props->buffer_file().string().c_str())));
//        m_buffers_cached_list->setStringList(values);
//        m_buffers_cached_list->sort(0);
//    }
//
//    std::vector< std::pair<qtgl::buffer_properties_ptr,std::string> >  failed_buffers_info;
//    qtgl::get_properties_of_failed_buffers(failed_buffers_info);
//    {
//        QStringList values;
//        for (auto const  props_error : failed_buffers_info)
//            values.push_back(dir.relativeFilePath(QString(props_error.first->buffer_file().string().c_str())) +
//                             QString(" : ") +
//                             QString(props_error.second.c_str()));
//        m_buffers_failed_list->setStringList(values);
//        m_buffers_failed_list->sort(0);
//    }
//}


void  program_window::on_shaders_root_dir_changed()
{
    QDir const  dir(m_shaders_root_dir_edit->text());

    QStringList  updated_paths;
    {
        QStringList const  paths = m_shaders_cached_list->stringList();
        for (int i = 0; i < paths.size(); ++i)
        {
            boost::filesystem::path  pathname = shaders_root_dir() / qtgl::to_string(paths.at(i));
            updated_paths.push_back(dir.relativeFilePath(pathname.string().c_str()));
        }
    }
    m_shaders_cached_list->setStringList(updated_paths);
    m_shaders_cached_list->sort(0);

    shaders_root_dir() = qtgl::to_string(dir.absolutePath());
}

void  program_window::on_shaders_select_root_dir()
{
    QFileDialog  dialog(this);
    dialog.setDirectory(batches_root_dir().string().c_str());
    dialog.setFileMode(QFileDialog::DirectoryOnly);
    if (dialog.exec())
    {
        QStringList const  selected = dialog.selectedFiles();
        if (selected.size() == 1)
        {
            m_shaders_root_dir_edit->setText(selected.at(0));
            on_shaders_root_dir_changed();
        }
    }
}

//void  program_window::on_shaders_refresh_lists()
//{
//    qtgl::make_current_window_guard const  guard(m_glwindow.make_me_current());
//
//    QDir const  dir(shaders_root_dir().string().c_str());
//
//    QStringList cached_values;
//    QStringList failed_values;
//    {
//        std::vector< std::pair<boost::filesystem::path,qtgl::vertex_program_properties_ptr> >  cached_props;
//        qtgl::get_properties_of_cached_vertex_programs(cached_props);
//        {
//            for (auto const  props : cached_props)
//                cached_values.push_back(dir.relativeFilePath(QString(props.first.string().c_str())));
//        }
//        std::vector< std::pair<boost::filesystem::path,std::string> > failed_props;
//        qtgl::get_properties_of_failed_vertex_programs(failed_props);
//        {
//            QStringList values;
//            for (auto const  props : failed_props)
//                failed_values.push_back(dir.relativeFilePath(QString(props.first.string().c_str())) +
//                                        QString(" : ") +
//                                        QString(props.second.c_str()));
//        }
//    }
//    {
//        std::vector< std::pair<boost::filesystem::path,qtgl::fragment_program_properties_ptr> >  cached_props;
//        qtgl::get_properties_of_cached_fragment_programs(cached_props);
//        {
//            QStringList values;
//            for (auto const  props : cached_props)
//                cached_values.push_back(dir.relativeFilePath(QString(props.first.string().c_str())));
//        }
//        std::vector< std::pair<boost::filesystem::path,std::string> > failed_props;
//        qtgl::get_properties_of_failed_fragment_programs(failed_props);
//        {
//            QStringList values;
//            for (auto const  props : failed_props)
//                failed_values.push_back(dir.relativeFilePath(QString(props.first.string().c_str())) +
//                                        QString(" : ") +
//                                        QString(props.second.c_str()));
//        }
//    }
//
//    m_shaders_cached_list->setStringList(cached_values);
//    m_shaders_cached_list->sort(0);
//
//    m_shaders_failed_list->setStringList(failed_values);
//    m_shaders_failed_list->sort(0);
//}


void  program_window::on_textures_root_dir_changed()
{
    QDir const  dir(m_textures_root_dir_edit->text());

    QStringList  updated_paths;
    {
        QStringList const  paths = m_textures_cached_list->stringList();
        for (int i = 0; i < paths.size(); ++i)
        {
            boost::filesystem::path  pathname = textures_root_dir() / qtgl::to_string(paths.at(i));
            updated_paths.push_back(dir.relativeFilePath(pathname.string().c_str()));
        }
    }
    m_textures_cached_list->setStringList(updated_paths);
    m_textures_cached_list->sort(0);

    textures_root_dir() = qtgl::to_string(dir.absolutePath());
}

void  program_window::on_textures_select_root_dir()
{
    QFileDialog  dialog(this);
    dialog.setDirectory(batches_root_dir().string().c_str());
    dialog.setFileMode(QFileDialog::DirectoryOnly);
    if (dialog.exec())
    {
        QStringList const  selected = dialog.selectedFiles();
        if (selected.size() == 1)
        {
            m_textures_root_dir_edit->setText(selected.at(0));
            on_textures_root_dir_changed();
        }
    }
}

void  program_window::on_textures_refresh_lists()
{
    qtgl::make_current_window_guard const  guard(m_glwindow.make_me_current());

    QDir const  dir(textures_root_dir().string().c_str());

    //std::vector< std::pair<boost::filesystem::path,qtgl::texture_properties_ptr> >  cached_texture_props;
    //qtgl::get_properties_of_cached_textures(cached_texture_props);
    //{
    //    QStringList values;
    //    for (auto const  props : cached_texture_props)
    //        if (props.first.empty())
    //            values.push_back(dir.relativeFilePath(QString(props.second->image_file().string().c_str())));
    //        else
    //            values.push_back(dir.relativeFilePath(QString(props.first.string().c_str())));
    //    m_textures_cached_list->setStringList(values);
    //    m_textures_cached_list->sort(0);
    //}

    //std::vector< std::pair<boost::filesystem::path,std::string> >  failed_textures_info;
    //qtgl::get_properties_of_failed_textures(failed_textures_info);
    //{
    //    QStringList values;
    //    for (auto const  props_error : failed_textures_info)
    //        values.push_back(dir.relativeFilePath(QString(props_error.first.string().c_str())) +
    //                         QString(" : ") +
    //                         QString(props_error.second.c_str()));
    //    m_textures_failed_list->setStringList(values);
    //    m_textures_failed_list->sort(0);
    //}
}
