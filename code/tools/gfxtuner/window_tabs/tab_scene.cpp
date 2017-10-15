#include <gfxtuner/window_tabs/tab_scene.hpp>
#include <gfxtuner/program_window.hpp>
#include <gfxtuner/simulator_notifications.hpp>
#include <qtgl/gui_utils.hpp>
#include <boost/property_tree/ptree.hpp>
#include <QLabel>
#include <QDialog>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QColorDialog>
#include <QString>
#include <QIntValidator>
#include <QDoubleValidator>

namespace {


struct  insert_name_dialog : public QDialog
{
    insert_name_dialog(program_window* const  wnd)
        : QDialog(wnd)
        , m_wnd(wnd)
        , m_name_edit(new QLineEdit())
        , m_node_name()
    {
        QVBoxLayout* const dlg_layout = new QVBoxLayout;
        {
            dlg_layout->addWidget(m_name_edit);

            QHBoxLayout* const buttons_layout = new QHBoxLayout;
            {
                buttons_layout->addWidget(
                    [](insert_name_dialog* wnd) {
                        struct Open : public QPushButton {
                            Open(insert_name_dialog* wnd) : QPushButton("OK")
                            {
                                QObject::connect(this, SIGNAL(released()), wnd, SLOT(accept()));
                                setDefault(true);
                                autoDefault();
                            }
                        };
                        return new Open(wnd);
                    }(this)
                    );
                buttons_layout->addWidget(
                    [](insert_name_dialog* wnd) {
                        struct Close : public QPushButton {
                            Close(insert_name_dialog* wnd) : QPushButton("Cancel")
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
        this->setWindowTitle("Node name");
        this->resize(300,100);
    }

    std::string const&  get_node_name() const { return m_node_name; }

public slots:

    void  accept()
    {
        m_node_name = qtgl::to_string(m_name_edit->text());
        QDialog::accept();
    }

    void  reject()
    {
        QDialog::reject();
    }

private:
    program_window*  m_wnd;
    QLineEdit*  m_name_edit;
    std::string  m_node_name;
};


}

namespace window_tabs { namespace tab_scene {


widgets::widgets(program_window* const  wnd)
    : m_wnd(wnd)

    , m_scene_tree(
        [](program_window* wnd) {
            struct s : public QTreeWidget {
                s(program_window* wnd) : QTreeWidget()
                {
                    QObject::connect(
                        this,
                        SIGNAL(itemClicked(QTreeWidgetItem*, int)),
                        wnd,
                        SLOT(on_scene_hierarchy_item_selected(QTreeWidgetItem*, int))
                        );
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
{}

void  widgets::on_scene_hierarchy_item_selected(QTreeWidgetItem* const tree_item, int const column)
{
    int iii = 0;
}

void  widgets::on_scene_insert_coord_system()
{
    insert_name_dialog  dlg(wnd());
    dlg.exec();
}

void  widgets::on_scene_insert_batch()
{
    int iii = 0;
}

void  widgets::on_scene_erase_selected()
{
    int iii = 0;
}

void  widgets::save()
{
    //wnd()->ptree().put("draw.show_grid", m_show_grid->isChecked());
}

QWidget*  make_scene_tab_content(widgets const&  w)
{
    QWidget* const  scene_tab = new QWidget;
    {
        QVBoxLayout* const scene_tab_layout = new QVBoxLayout;
        {
            {
                QTreeWidgetItem* headerItem = new QTreeWidgetItem();
                headerItem->setText(0, QString("Hierarchy"));
                w.scene_tree()->setHeaderItem(headerItem);
            }
            scene_tab_layout->addWidget(w.scene_tree());

            QWidget* const operation_group = new QGroupBox("Operation");
            {
                QVBoxLayout* const buttons_layout = new QVBoxLayout;
                {
                    QHBoxLayout* const insertion_layout = new QHBoxLayout;
                    {
                        insertion_layout->addWidget(
                            [](program_window* wnd) {
                                    struct choose : public QPushButton {
                                        choose(program_window* wnd) : QPushButton("Insert node")
                                        {
                                            QObject::connect(
                                                this,
                                                SIGNAL(released()),
                                                wnd,
                                                SLOT(on_scene_insert_coord_system())
                                                );
                                        }
                                    };
                                    return new choose(wnd);
                                }(w.wnd())
                                );
                        insertion_layout->addWidget(
                            [](program_window* wnd) {
                                    struct choose : public QPushButton {
                                        choose(program_window* wnd) : QPushButton("Insert batch")
                                        {
                                            QObject::connect(
                                                this,
                                                SIGNAL(released()),
                                                wnd,
                                                SLOT(on_scene_insert_batch())
                                                );
                                        }
                                    };
                                    return new choose(wnd);
                                }(w.wnd())
                                );
                    }
                    buttons_layout->addLayout(insertion_layout);

                    buttons_layout->addWidget(
                        [](program_window* wnd) {
                                struct choose : public QPushButton {
                                    choose(program_window* wnd) : QPushButton("Erase selected")
                                    {
                                        QObject::connect(
                                            this,
                                            SIGNAL(released()),
                                            wnd,
                                            SLOT(on_scene_erase_selected())
                                            );
                                    }
                                };
                                return new choose(wnd);
                            }(w.wnd())
                            );
                }
                operation_group->setLayout(buttons_layout);
            }
            scene_tab_layout->addWidget(operation_group);

            QWidget* const search_group = new QGroupBox("Search");
            {
            }
            scene_tab_layout->addWidget(search_group);

            QWidget* const selected_group = new QGroupBox("Selected properties");
            {
            }
            scene_tab_layout->addWidget(selected_group);

            scene_tab_layout->addStretch(1);
        }
        scene_tab->setLayout(scene_tab_layout);
    }
    return scene_tab;
}


}}
