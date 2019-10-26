#include <gfxtuner/window_tabs/tab_statistics_collisions.hpp>
#include <gfxtuner/program_window.hpp>
#include <gfxtuner/program_options.hpp>
#include <gfxtuner/simulation/simulator.hpp>
#include <gfxtuner/simulation/simulator_notifications.hpp>
#include <angeo/collision_scene.hpp>
#include <qtgl/gui_utils.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <boost/algorithm/string.hpp>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QString>
#include <QLabel>

namespace window_tabs { namespace tab_statistics { namespace tab_collisions {


widgets::widgets(program_window* const  wnd)
    : m_wnd(wnd)
    , m_num_capsules(new QLineEdit("0"))
    , m_num_lines(new QLineEdit("0"))
    , m_num_points(new QLineEdit("0"))
    , m_num_spheres(new QLineEdit("0"))
    , m_num_triangles(new QLineEdit("0"))

    , m_num_compute_contacts_calls_in_last_frame(new QLineEdit("0"))
    , m_max_num_compute_contacts_calls_till_last_frame(new QLineEdit("0"))

    , m_num_contacts_in_last_frame(new QLineEdit("0"))
    , m_max_num_contacts_till_last_frame(new QLineEdit("0"))

    , m_proximity_static_num_objects(new QLineEdit("0"))
    , m_proximity_static_num_split_nodes(new QLineEdit("0"))
    , m_proximity_static_num_searches_by_bbox_in_last_frame(new QLineEdit("0"))
    , m_proximity_static_max_num_searches_by_bbox_till_last_frame(new QLineEdit("0"))
    , m_proximity_static_num_searches_by_line_in_last_frame(new QLineEdit("0"))
    , m_proximity_static_max_num_searches_by_line_till_last_frame(new QLineEdit("0"))
    , m_proximity_static_num_enumerate_calls_in_last_frame(new QLineEdit("0"))
    , m_proximity_static_max_num_enumerate_calls_till_last_frame(new QLineEdit("0"))

    , m_proximity_dynamic_num_objects(new QLineEdit("0"))
    , m_proximity_dynamic_num_split_nodes(new QLineEdit("0"))
    , m_proximity_dynamic_num_searches_by_bbox_in_last_frame(new QLineEdit("0"))
    , m_proximity_dynamic_max_num_searches_by_bbox_till_last_frame(new QLineEdit("0"))
    , m_proximity_dynamic_num_searches_by_line_in_last_frame(new QLineEdit("0"))
    , m_proximity_dynamic_max_num_searches_by_line_till_last_frame(new QLineEdit("0"))
    , m_proximity_dynamic_num_enumerate_calls_in_last_frame(new QLineEdit("0"))
    , m_proximity_dynamic_max_num_enumerate_calls_till_last_frame(new QLineEdit("0"))
{
    m_num_capsules->setReadOnly(true);
    m_num_lines->setReadOnly(true);
    m_num_points->setReadOnly(true);
    m_num_spheres->setReadOnly(true);
    m_num_triangles->setReadOnly(true);

    m_num_compute_contacts_calls_in_last_frame->setReadOnly(true);
    m_max_num_compute_contacts_calls_till_last_frame->setReadOnly(true);

    m_num_contacts_in_last_frame->setReadOnly(true);
    m_max_num_contacts_till_last_frame->setReadOnly(true);

    m_proximity_static_num_objects->setReadOnly(true);
    m_proximity_static_num_split_nodes->setReadOnly(true);
    m_proximity_static_num_searches_by_bbox_in_last_frame->setReadOnly(true);
    m_proximity_static_max_num_searches_by_bbox_till_last_frame->setReadOnly(true);
    m_proximity_static_num_searches_by_line_in_last_frame->setReadOnly(true);
    m_proximity_static_max_num_searches_by_line_till_last_frame->setReadOnly(true);
    m_proximity_static_num_enumerate_calls_in_last_frame->setReadOnly(true);
    m_proximity_static_max_num_enumerate_calls_till_last_frame->setReadOnly(true);

    m_proximity_dynamic_num_objects->setReadOnly(true);
    m_proximity_dynamic_num_split_nodes->setReadOnly(true);
    m_proximity_dynamic_num_searches_by_bbox_in_last_frame->setReadOnly(true);
    m_proximity_dynamic_max_num_searches_by_bbox_till_last_frame->setReadOnly(true);
    m_proximity_dynamic_num_searches_by_line_in_last_frame->setReadOnly(true);
    m_proximity_dynamic_max_num_searches_by_line_till_last_frame->setReadOnly(true);
    m_proximity_dynamic_num_enumerate_calls_in_last_frame->setReadOnly(true);
    m_proximity_dynamic_max_num_enumerate_calls_till_last_frame->setReadOnly(true);
}


void  widgets::on_time_event()
{
    TMPROF_BLOCK();

    angeo::collision_scene::statistics const&  statistics =
            wnd()->glwindow().call_now(&simulator::get_collision_scene)->get_statistics();

    m_num_capsules->setText(QString::number(statistics.num_capsules));
    m_num_lines->setText(QString::number(statistics.num_lines));
    m_num_points->setText(QString::number(statistics.num_points));
    m_num_spheres->setText(QString::number(statistics.num_spheres));
    m_num_triangles->setText(QString::number(statistics.num_triangles));

    m_num_compute_contacts_calls_in_last_frame->setText(QString::number(statistics.num_compute_contacts_calls_in_last_frame));
    m_max_num_compute_contacts_calls_till_last_frame->setText(QString::number(statistics.max_num_compute_contacts_calls_till_last_frame));

    m_num_contacts_in_last_frame->setText(QString::number(statistics.num_contacts_in_last_frame));
    m_max_num_contacts_till_last_frame->setText(QString::number(statistics.max_num_contacts_till_last_frame));

    m_proximity_static_num_objects->setText(QString::number(statistics.static_objects_proximity->num_objects));
    m_proximity_static_num_split_nodes->setText(QString::number(statistics.static_objects_proximity->num_split_nodes));
    m_proximity_static_num_searches_by_bbox_in_last_frame->setText(QString::number(statistics.static_objects_proximity->num_searches_by_bbox_in_last_frame));
    m_proximity_static_max_num_searches_by_bbox_till_last_frame->setText(QString::number(statistics.static_objects_proximity->max_num_searches_by_bbox_till_last_frame));
    m_proximity_static_num_searches_by_line_in_last_frame->setText(QString::number(statistics.static_objects_proximity->num_searches_by_line_in_last_frame));
    m_proximity_static_max_num_searches_by_line_till_last_frame->setText(QString::number(statistics.static_objects_proximity->max_num_searches_by_line_till_last_frame));
    m_proximity_static_num_enumerate_calls_in_last_frame->setText(QString::number(statistics.static_objects_proximity->num_enumerate_calls_in_last_frame));
    m_proximity_static_max_num_enumerate_calls_till_last_frame->setText(QString::number(statistics.static_objects_proximity->max_num_enumerate_calls_till_last_frame));

    m_proximity_dynamic_num_objects->setText(QString::number(statistics.dynamic_objects_proximity->num_objects));
    m_proximity_dynamic_num_split_nodes->setText(QString::number(statistics.dynamic_objects_proximity->num_split_nodes));
    m_proximity_dynamic_num_searches_by_bbox_in_last_frame->setText(QString::number(statistics.dynamic_objects_proximity->num_searches_by_bbox_in_last_frame));
    m_proximity_dynamic_max_num_searches_by_bbox_till_last_frame->setText(QString::number(statistics.dynamic_objects_proximity->max_num_searches_by_bbox_till_last_frame));
    m_proximity_dynamic_num_searches_by_line_in_last_frame->setText(QString::number(statistics.dynamic_objects_proximity->num_searches_by_line_in_last_frame));
    m_proximity_dynamic_max_num_searches_by_line_till_last_frame->setText(QString::number(statistics.dynamic_objects_proximity->max_num_searches_by_line_till_last_frame));
    m_proximity_dynamic_num_enumerate_calls_in_last_frame->setText(QString::number(statistics.dynamic_objects_proximity->num_enumerate_calls_in_last_frame));
    m_proximity_dynamic_max_num_enumerate_calls_till_last_frame->setText(QString::number(statistics.dynamic_objects_proximity->max_num_enumerate_calls_till_last_frame));
}


std::string  get_tooltip()
{
    return "Collision detection statistics,\n"
           "including the proximity map.";
}


QWidget*  make_tab_content(widgets const&  w)
{
    auto const  create_colliders_group = [&w]() -> QWidget* {
        QWidget* const colliders_group = new QGroupBox("Counts of colliders");
        {
            QVBoxLayout* const colliders_layout = new QVBoxLayout;
            {
                QHBoxLayout* hbox_layout;

                hbox_layout = new QHBoxLayout;
                {
                    hbox_layout->addWidget(new QLabel("Capsules:"));
                    hbox_layout->addWidget(w.num_capsules());
                    hbox_layout->addStretch(1);
                }
                colliders_layout->addLayout(hbox_layout);

                hbox_layout = new QHBoxLayout;
                {
                    hbox_layout->addWidget(new QLabel("Lines:"));
                    hbox_layout->addWidget(w.num_lines());
                    hbox_layout->addStretch(1);
                }
                colliders_layout->addLayout(hbox_layout);

                hbox_layout = new QHBoxLayout;
                {
                    hbox_layout->addWidget(new QLabel("Points:"));
                    hbox_layout->addWidget(w.num_points());
                    hbox_layout->addStretch(1);
                }
                colliders_layout->addLayout(hbox_layout);

                hbox_layout = new QHBoxLayout;
                {
                    hbox_layout->addWidget(new QLabel("Spheres:"));
                    hbox_layout->addWidget(w.num_spheres());
                    hbox_layout->addStretch(1);
                }
                colliders_layout->addLayout(hbox_layout);

                hbox_layout = new QHBoxLayout;
                {
                    hbox_layout->addWidget(new QLabel("Triangles:"));
                    hbox_layout->addWidget(w.num_triangles());
                    hbox_layout->addStretch(1);
                }
                colliders_layout->addLayout(hbox_layout);

                colliders_layout->addStretch(1);
            }
            colliders_group->setLayout(colliders_layout);
        }
        return colliders_group;
    };
    auto const  create_contacts_calls_group= [&w]() -> QWidget* {
        QWidget* const contacts_calls_group = new QGroupBox("Calls to 'compute_contacts' function");
        {
            QVBoxLayout* const contacts_calls_layout = new QVBoxLayout;
            {
                QHBoxLayout* hbox_layout;

                hbox_layout = new QHBoxLayout;
                {
                    hbox_layout->addWidget(new QLabel("Last frame:"));
                    hbox_layout->addWidget(w.num_compute_contacts_calls_in_last_frame());
                    hbox_layout->addStretch(1);
                }
                contacts_calls_layout->addLayout(hbox_layout);

                hbox_layout = new QHBoxLayout;
                {
                    hbox_layout->addWidget(new QLabel("Maximum till last frame:"));
                    hbox_layout->addWidget(w.max_num_compute_contacts_calls_till_last_frame());
                    hbox_layout->addStretch(1);
                }
                contacts_calls_layout->addLayout(hbox_layout);

                contacts_calls_layout->addStretch(1);
            }
            contacts_calls_group->setLayout(contacts_calls_layout);
        }
        return contacts_calls_group;
    };
    auto const  create_contacts_group = [&w]() -> QWidget* {
        QWidget* const contacts_group = new QGroupBox("Contacts computed");
        {
            QVBoxLayout* const contacts_layout = new QVBoxLayout;
            {
                QHBoxLayout* hbox_layout;

                hbox_layout = new QHBoxLayout;
                {
                    hbox_layout->addWidget(new QLabel("Last frame:"));
                    hbox_layout->addWidget(w.num_contacts_in_last_frame());
                    hbox_layout->addStretch(1);
                }
                contacts_layout->addLayout(hbox_layout);

                hbox_layout = new QHBoxLayout;
                {
                    hbox_layout->addWidget(new QLabel("Maximum till last frame:"));
                    hbox_layout->addWidget(w.max_num_contacts_till_last_frame());
                    hbox_layout->addStretch(1);
                }
                contacts_layout->addLayout(hbox_layout);

                contacts_layout->addStretch(1);
            }
            contacts_group->setLayout(contacts_layout);
        }
        return contacts_group;
    };
    auto const  create_proximity_static_group = [&w]() -> QWidget* {
        QWidget* const proximity_static_group = new QGroupBox("Proximity map of static colliders");
        {
            QVBoxLayout* const proximity_static_layout = new QVBoxLayout;
            {
                QHBoxLayout* hbox_layout;

                hbox_layout = new QHBoxLayout;
                {
                    hbox_layout->addWidget(new QLabel("Colliders:"));
                    hbox_layout->addWidget(w.proximity_static_num_objects());
                    hbox_layout->addStretch(1);
                }
                proximity_static_layout->addLayout(hbox_layout);

                hbox_layout = new QHBoxLayout;
                {
                    hbox_layout->addWidget(new QLabel("Split nodes:"));
                    hbox_layout->addWidget(w.proximity_static_num_split_nodes());
                    hbox_layout->addStretch(1);
                }
                proximity_static_layout->addLayout(hbox_layout);

                QWidget* const last_frame_group = new QGroupBox("Last frame");
                {
                    QVBoxLayout* const last_frame_layout = new QVBoxLayout;
                    {
                        hbox_layout = new QHBoxLayout;
                        {
                            hbox_layout->addWidget(new QLabel("Searches by bbox:"));
                            hbox_layout->addWidget(w.proximity_static_num_searches_by_bbox_in_last_frame());
                            hbox_layout->addStretch(1);
                        }
                        last_frame_layout->addLayout(hbox_layout);

                        hbox_layout = new QHBoxLayout;
                        {
                            hbox_layout->addWidget(new QLabel("Searches by line:"));
                            hbox_layout->addWidget(w.proximity_static_num_searches_by_line_in_last_frame());
                            hbox_layout->addStretch(1);
                        }
                        last_frame_layout->addLayout(hbox_layout);

                        hbox_layout = new QHBoxLayout;
                        {
                            hbox_layout->addWidget(new QLabel("Enumerations:"));
                            hbox_layout->addWidget(w.proximity_static_num_enumerate_calls_in_last_frame());
                            hbox_layout->addStretch(1);
                        }
                        last_frame_layout->addLayout(hbox_layout);

                        last_frame_layout->addStretch(1);
                    }
                    last_frame_group->setLayout(last_frame_layout);
                }
                proximity_static_layout->addWidget(last_frame_group);

                QWidget* const till_last_frame_group = new QGroupBox("Max. till last frame");
                {
                    QVBoxLayout* const till_last_frame_layout = new QVBoxLayout;
                    {
                        hbox_layout = new QHBoxLayout;
                        {
                            hbox_layout->addWidget(new QLabel("Searches by bbox:"));
                            hbox_layout->addWidget(w.proximity_static_max_num_searches_by_bbox_till_last_frame());
                            hbox_layout->addStretch(1);
                        }
                        till_last_frame_layout->addLayout(hbox_layout);

                        hbox_layout = new QHBoxLayout;
                        {
                            hbox_layout->addWidget(new QLabel("Searches by line:"));
                            hbox_layout->addWidget(w.proximity_static_max_num_searches_by_line_till_last_frame());
                            hbox_layout->addStretch(1);
                        }
                        till_last_frame_layout->addLayout(hbox_layout);

                        hbox_layout = new QHBoxLayout;
                        {
                            hbox_layout->addWidget(new QLabel("Enumerations:"));
                            hbox_layout->addWidget(w.proximity_static_max_num_enumerate_calls_till_last_frame());
                            hbox_layout->addStretch(1);
                        }
                        till_last_frame_layout->addLayout(hbox_layout);

                        till_last_frame_layout->addStretch(1);
                    }
                    till_last_frame_group->setLayout(till_last_frame_layout);
                }
                proximity_static_layout->addWidget(till_last_frame_group);

                proximity_static_layout->addStretch(1);
            }
            proximity_static_group->setLayout(proximity_static_layout);
        }
        return proximity_static_group;
    };
    auto const  create_proximity_dynamic_group = [&w]() -> QWidget* {
        QWidget* const proximity_dynamic_group = new QGroupBox("Proximity map of dynamic colliders");
        {
            QVBoxLayout* const proximity_dynamic_layout = new QVBoxLayout;
            {
                QHBoxLayout* hbox_layout;

                hbox_layout = new QHBoxLayout;
                {
                    hbox_layout->addWidget(new QLabel("Colliders:"));
                    hbox_layout->addWidget(w.proximity_dynamic_num_objects());
                    hbox_layout->addStretch(1);
                }
                proximity_dynamic_layout->addLayout(hbox_layout);

                hbox_layout = new QHBoxLayout;
                {
                    hbox_layout->addWidget(new QLabel("Split nodes:"));
                    hbox_layout->addWidget(w.proximity_dynamic_num_split_nodes());
                    hbox_layout->addStretch(1);
                }
                proximity_dynamic_layout->addLayout(hbox_layout);

                QWidget* const last_frame_group = new QGroupBox("Last frame");
                {
                    QVBoxLayout* const last_frame_layout = new QVBoxLayout;
                    {
                        hbox_layout = new QHBoxLayout;
                        {
                            hbox_layout->addWidget(new QLabel("Searches by bbox:"));
                            hbox_layout->addWidget(w.proximity_dynamic_num_searches_by_bbox_in_last_frame());
                            hbox_layout->addStretch(1);
                        }
                        last_frame_layout->addLayout(hbox_layout);

                        hbox_layout = new QHBoxLayout;
                        {
                            hbox_layout->addWidget(new QLabel("Searches by line:"));
                            hbox_layout->addWidget(w.proximity_dynamic_num_searches_by_line_in_last_frame());
                            hbox_layout->addStretch(1);
                        }
                        last_frame_layout->addLayout(hbox_layout);

                        hbox_layout = new QHBoxLayout;
                        {
                            hbox_layout->addWidget(new QLabel("Enumerations:"));
                            hbox_layout->addWidget(w.proximity_dynamic_num_enumerate_calls_in_last_frame());
                            hbox_layout->addStretch(1);
                        }
                        last_frame_layout->addLayout(hbox_layout);

                        last_frame_layout->addStretch(1);
                    }
                    last_frame_group->setLayout(last_frame_layout);
                }
                proximity_dynamic_layout->addWidget(last_frame_group);

                QWidget* const till_last_frame_group = new QGroupBox("Max. till last frame");
                {
                    QVBoxLayout* const till_last_frame_layout = new QVBoxLayout;
                    {
                        hbox_layout = new QHBoxLayout;
                        {
                            hbox_layout->addWidget(new QLabel("Searches by bbox:"));
                            hbox_layout->addWidget(w.proximity_dynamic_max_num_searches_by_bbox_till_last_frame());
                            hbox_layout->addStretch(1);
                        }
                        till_last_frame_layout->addLayout(hbox_layout);

                        hbox_layout = new QHBoxLayout;
                        {
                            hbox_layout->addWidget(new QLabel("Searches by line:"));
                            hbox_layout->addWidget(w.proximity_dynamic_max_num_searches_by_line_till_last_frame());
                            hbox_layout->addStretch(1);
                        }
                        till_last_frame_layout->addLayout(hbox_layout);

                        hbox_layout = new QHBoxLayout;
                        {
                            hbox_layout->addWidget(new QLabel("Enumerations:"));
                            hbox_layout->addWidget(w.proximity_dynamic_max_num_enumerate_calls_till_last_frame());
                            hbox_layout->addStretch(1);
                        }
                        till_last_frame_layout->addLayout(hbox_layout);

                        till_last_frame_layout->addStretch(1);
                    }
                    till_last_frame_group->setLayout(till_last_frame_layout);
                }
                proximity_dynamic_layout->addWidget(till_last_frame_group);

                proximity_dynamic_layout->addStretch(1);
            }
            proximity_dynamic_group->setLayout(proximity_dynamic_layout);
        }
        return proximity_dynamic_group;
    };

    QWidget* const  tab = new QWidget;
    {
        QVBoxLayout* const tab_layout = new QVBoxLayout;
        {
            QHBoxLayout* const scene_layout = new QHBoxLayout;
            {
                scene_layout->addWidget(create_colliders_group());

                QVBoxLayout* const contacts_layout = new QVBoxLayout;
                {
                    contacts_layout->addWidget(create_contacts_calls_group());
                    contacts_layout->addWidget(create_contacts_group());
                }
                scene_layout->addLayout(contacts_layout);
            }
            tab_layout->addLayout(scene_layout);

            QHBoxLayout* const proximity_layout = new QHBoxLayout;
            {
                proximity_layout->addWidget(create_proximity_static_group());
                proximity_layout->addWidget(create_proximity_dynamic_group());
            }
            tab_layout->addLayout(proximity_layout);

            tab_layout->addStretch(1);
        }
        tab->setLayout(tab_layout);
    }
    return tab;
}


}}}
