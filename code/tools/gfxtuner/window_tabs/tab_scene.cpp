#include <gfxtuner/window_tabs/tab_scene.hpp>
#include <gfxtuner/program_window.hpp>
#include <gfxtuner/program_options.hpp>
#include <gfxtuner/simulator_notifications.hpp>
#include <qtgl/gui_utils.hpp>
#include <utility/msgstream.hpp>
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
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

namespace {


struct  tree_widget_item : public QTreeWidgetItem
{
    explicit tree_widget_item(bool const  represents_coord_system)
        : QTreeWidgetItem()
        , m_represents_coord_system(represents_coord_system)
    {}

    bool  represents_coord_system() const { return m_represents_coord_system; }

private:
    
    bool  m_represents_coord_system;
};


struct  insert_name_dialog : public QDialog
{
    insert_name_dialog(program_window* const  wnd, std::string const&  initial_name)
        : QDialog(wnd)
        , m_wnd(wnd)
        , m_name_edit(
            [](insert_name_dialog* dlg, std::string const&  initial_name) {
                struct s : public QLineEdit {
                    s(insert_name_dialog* dlg, std::string const&  initial_name) : QLineEdit()
                    {
                        setText(QString(initial_name.c_str()));
                        QObject::connect(this, &QLineEdit::textChanged, dlg, &insert_name_dialog::on_name_changed);
                    }
                };
                return new s(dlg, initial_name);
            }(this, initial_name)
            )
        , m_name_taken_indicator(new QLabel("WARNING: The name is already in use."))
        , m_OK_button(
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
            )
        , m_name()
    {
        QVBoxLayout* const dlg_layout = new QVBoxLayout;
        {
            dlg_layout->addWidget(m_name_edit);
            dlg_layout->addWidget(m_name_taken_indicator);

            QHBoxLayout* const buttons_layout = new QHBoxLayout;
            {
                buttons_layout->addWidget(m_OK_button);
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
        this->setWindowTitle("Insert name");
        this->resize(300,100);

        on_name_changed(QString(initial_name.c_str()));
    }

    std::string const&  get_name() const { return m_name; }

    void  on_name_changed(QString const&  qname)
    {
        if (m_wnd->glwindow().call_now(&simulator::get_scene_node, qtgl::to_string(qname)) != nullptr)
        {
            m_name_taken_indicator->setText(QString("WARNING: the name is already in use."));
            m_OK_button->setEnabled(false);
        }
        else
        {
            m_name_taken_indicator->setText(QString("OK: The name is fresh."));
            m_OK_button->setEnabled(true);
        }
    }

public slots:

    void  accept()
    {
        m_name = qtgl::to_string(m_name_edit->text());
        QDialog::accept();
    }

    void  reject()
    {
        QDialog::reject();
    }
    //
private:
    program_window*  m_wnd;
    QLineEdit*  m_name_edit;
    QLabel*  m_name_taken_indicator;
    QPushButton*  m_OK_button;

    std::string  m_name;
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
    , m_node_icon((boost::filesystem::path{ get_program_options()->dataRoot() } /
                   "shared/gfx/icons/coord_system.png").string().c_str())
{
    m_scene_tree->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
}

void  widgets::on_scene_hierarchy_item_selected(QTreeWidgetItem* const tree_item, int const column)
{
    int iii = 0;
}

void  widgets::on_scene_insert_coord_system()
{
    static natural_64_bit  counter = 0ULL;
    std::string const  name = msgstream() << "coord_system_" << counter;
    ++counter;
    insert_name_dialog  dlg(wnd(), name);
    dlg.exec();
    if (!dlg.get_name().empty())
    {
        m_wnd->glwindow().call_now(&simulator::insert_scene_node, dlg.get_name());

        QTreeWidgetItem* const  tree_node = new tree_widget_item(true);
        tree_node->setText(0, QString(dlg.get_name().c_str()));
        tree_node->setIcon(0, m_node_icon);
        m_scene_tree->addTopLevelItem(tree_node);
    }
}

void  widgets::on_scene_insert_batch()
{
    int iii = 0;
}

void  widgets::on_scene_erase_selected()
{
    foreach(QTreeWidgetItem* const  item, m_scene_tree->selectedItems())
    {
        tree_widget_item* const  tree_item = dynamic_cast<tree_widget_item*>(item);
        INVARIANT(tree_item != nullptr);
        std::string const  tree_item_name = qtgl::to_string(tree_item->text(0));
        if (tree_item->represents_coord_system())
            m_wnd->glwindow().call_now(&simulator::erase_scene_node, tree_item_name);
        else
        {
            tree_widget_item* const  parent_tree_item = dynamic_cast<tree_widget_item*>(tree_item->parent());
            INVARIANT(parent_tree_item != nullptr);
            INVARIANT(parent_tree_item->represents_coord_system());
            std::string const  parent_tree_item_name = qtgl::to_string(parent_tree_item->text(0));
            m_wnd->glwindow().call_now(&simulator::erase_batch_from_scene_node, tree_item_name, parent_tree_item_name);
        }

        auto const  taken_item = item->parent() != nullptr ?
            item->parent()->takeChild(item->parent()->indexOfChild(item)) :
            m_scene_tree->takeTopLevelItem(m_scene_tree->indexOfTopLevelItem(item))
            ;
        INVARIANT(taken_item == tree_item); (void)taken_item;

        delete tree_item;
    }
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
