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
#include <vector>
#include <memory>

namespace window_tabs { namespace tab_statistics { namespace tab_collisions {


widgets::widgets(program_window* const  wnd)
    : m_wnd(wnd)
    , m_num_capsules(new QLabel("0"))
    , m_num_lines(new QLabel("0"))
    , m_num_points(new QLabel("0"))
    , m_num_spheres(new QLabel("0"))
    , m_num_triangles(new QLabel("0"))
    , m_num_compute_contacts_calls_in_last_frame(new QLabel("0"))
    , m_max_num_compute_contacts_calls_till_last_frame(new QLabel("0"))
{
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
}


std::string  get_tooltip()
{
    return "Collision detection statistics,\n"
           "including the proximity map.";
}


QWidget*  make_tab_content(widgets const&  w)
{
    QWidget* const  tab = new QWidget;
    {
        QVBoxLayout* const tab_layout = new QVBoxLayout;
        {
            QHBoxLayout* hbox_layout;
            
            QWidget* const colliders_group = new QGroupBox("Counts of colliders");
            {
                QVBoxLayout* const colliders_layout = new QVBoxLayout;
                {
                    hbox_layout = new QHBoxLayout;
                    {
                        hbox_layout->addWidget(new QLabel("Capsules:"));
                        hbox_layout->addWidget(w.num_capsules());
                    }
                    colliders_layout->addLayout(hbox_layout);

                    hbox_layout = new QHBoxLayout;
                    {
                        hbox_layout->addWidget(new QLabel("Lines:"));
                        hbox_layout->addWidget(w.num_lines());
                    }
                    colliders_layout->addLayout(hbox_layout);

                    hbox_layout = new QHBoxLayout;
                    {
                        hbox_layout->addWidget(new QLabel("Points:"));
                        hbox_layout->addWidget(w.num_points());
                    }
                    colliders_layout->addLayout(hbox_layout);

                    hbox_layout = new QHBoxLayout;
                    {
                        hbox_layout->addWidget(new QLabel("Spheres:"));
                        hbox_layout->addWidget(w.num_spheres());
                    }
                    colliders_layout->addLayout(hbox_layout);

                    hbox_layout = new QHBoxLayout;
                    {
                        hbox_layout->addWidget(new QLabel("Triangles:"));
                        hbox_layout->addWidget(w.num_triangles());
                    }
                    colliders_layout->addLayout(hbox_layout);

                    colliders_layout->addStretch(1);
                }
                colliders_group->setLayout(colliders_layout);
            }
            tab_layout->addWidget(colliders_group);

            QWidget* const contacts_calls_group = new QGroupBox("Calls to 'compute_contacts' function");
            {
                QVBoxLayout* const contacts_calls_layout = new QVBoxLayout;
                {
                    hbox_layout = new QHBoxLayout;
                    {
                        hbox_layout->addWidget(new QLabel("Last frame:"));
                        hbox_layout->addWidget(w.num_compute_contacts_calls_in_last_frame());
                    }
                    contacts_calls_layout->addLayout(hbox_layout);

                    hbox_layout = new QHBoxLayout;
                    {
                        hbox_layout->addWidget(new QLabel("Maximum till last frame:"));
                        hbox_layout->addWidget(w.max_num_compute_contacts_calls_till_last_frame());
                    }
                    contacts_calls_layout->addLayout(hbox_layout);

                    contacts_calls_layout->addStretch(1);
                }
                contacts_calls_group->setLayout(contacts_calls_layout);
            }
            tab_layout->addWidget(contacts_calls_group);

            tab_layout->addStretch(1);
        }
        tab->setLayout(tab_layout);
    }
    return tab;
}


}}}
