#include <netviewer/menu_bar.hpp>
#include <netviewer/program_window.hpp>
#include <netviewer/simulator.hpp>
#include <netviewer/simulator_notifications.hpp>
#include <netexp/experiment_factory.hpp>
#include <qtgl/window.hpp>
#include <qtgl/gui_utils.hpp>
#include <qtgl/widget_base.hpp>
#include <utility/msgstream.hpp>
#include <QString>
#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QGroupBox>
#include <QTextEdit>
#include <QLabel>
#include <QCheckBox>

struct  open_network_dialog : public QDialog
{
    open_network_dialog(program_window* const  wnd, bool* const  pause_applies_to_network_open_ptr)
        : QDialog(wnd)
        , m_wnd(wnd)
        , m_list(
            [](open_network_dialog* wnd) {
                struct s : public QListWidget {
                    s(open_network_dialog* wnd) : QListWidget()
                    {
                        QObject::connect(this, &QListWidget::currentTextChanged,wnd, &open_network_dialog::experiment_selected);
                    }
                };
                return new s(wnd);
            }(this)
            )
        , m_info(new QTextEdit("Select an experiment."))
        , m_selected_experiment()
        , m_pause_initialisation(
            [](bool const  pause_applies_to_network_open) {
                struct s : public QCheckBox {
                    s(bool const  pause_applies_to_network_open) : QCheckBox("Pause applies also to network construction")
                    {
                        setChecked(pause_applies_to_network_open);
                    }
                };
                return new s(pause_applies_to_network_open);
            }(*pause_applies_to_network_open_ptr)
            )
        , m_pause_applies_to_network_open_ptr(pause_applies_to_network_open_ptr)
    {
        ASSUMPTION(m_pause_applies_to_network_open_ptr != nullptr);
        std::vector<std::string>  experiments;
        netexp::experiment_factory::instance().get_names_of_registered_experiments(experiments);
        m_list->setSortingEnabled(true);
        for (auto const&  experiment_name : experiments)
            m_list->addItem(QString(experiment_name.c_str()));
        auto const current = m_list->findItems(m_wnd->glwindow().call_now(&simulator::get_experiment_name).c_str(),0);
        if (current.size() == 1ULL)
            m_list->setItemSelected(current.front(), true);
        else if (!experiments.empty())
            m_list->setItemSelected(m_list->item(0), true);

        m_info->setReadOnly(true);

        QVBoxLayout* const dlg_layout = new QVBoxLayout;
        {
            QHBoxLayout* const experiments_layout = new QHBoxLayout;
            {
                QVBoxLayout* const list_layout = new QVBoxLayout;
                {
                    list_layout->addWidget(new QLabel("Experiments:"));
                    list_layout->addWidget(m_list);
                }
                experiments_layout->addLayout(list_layout);
                experiments_layout->setStretch(0, 3);

                QVBoxLayout* const info_layout = new QVBoxLayout;
                {
                    info_layout->addWidget(new QLabel("Description:"));
                    info_layout->addWidget(m_info);
                }
                experiments_layout->addLayout(info_layout);
                experiments_layout->setStretch(1,4);
            }
            dlg_layout->addLayout(experiments_layout);

            dlg_layout->addWidget(m_pause_initialisation);

            QHBoxLayout* const buttons_layout = new QHBoxLayout;
            {
                buttons_layout->addWidget(
                    [](open_network_dialog* wnd) {
                        struct Open : public QPushButton {
                            Open(open_network_dialog* wnd) : QPushButton("Open")
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
                    [](open_network_dialog* wnd) {
                        struct Close : public QPushButton {
                            Close(open_network_dialog* wnd) : QPushButton("Close")
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
        this->setWindowTitle("Open network");
        this->resize(600,600);
    }

    std::string const&  selected_experiment() const noexcept { return m_selected_experiment; }

public slots:

    void  accept()
    {
        *m_pause_applies_to_network_open_ptr = m_pause_initialisation->isChecked();

        auto const  selected = m_list->selectedItems();
        if (selected.size() != 1ULL)
            return;
        m_selected_experiment = qtgl::to_string(selected.front()->text());
        QDialog::accept();
    }

    void experiment_selected(QString const&  text)
    {
        if (m_list->selectedItems().empty())
            return;
        std::string const  experiment_name = qtgl::to_string(text);
        m_info->setText(netexp::experiment_factory::instance().get_experiment_description(experiment_name).c_str());
    }

    void  reject()
    {
        *m_pause_applies_to_network_open_ptr = m_pause_initialisation->isChecked();

        QDialog::reject();
    }

private:
    program_window*  m_wnd;
    QListWidget*  m_list;
    QTextEdit*  m_info;
    std::string  m_selected_experiment;
    QCheckBox*  m_pause_initialisation;
    bool*  m_pause_applies_to_network_open_ptr;
};


menu_bar::menu_bar(program_window* const  wnd)
    : m_wnd(wnd)

    , m_menu_bar(new QMenuBar(wnd))
    , m_menu_network(new QMenu("&Network",wnd))
    , m_action_network_open(new QAction(QString("&Open"), wnd))
    , m_action_network_close(new QAction(QString("&Close"), wnd))

    , m_pause_applies_to_network_open(wnd->ptree().get("network.pause_construction", false))
{}

void  menu_bar::on_menu_network_open()
{
    if (wnd()->glwindow().call_now(&simulator::is_network_being_constructed))
        return;

    open_network_dialog  dlg(wnd(),&m_pause_applies_to_network_open);
    dlg.exec();

    if (dlg.selected_experiment().empty())
        return;

    on_menu_network_close();
    wnd()->glwindow().call_later(
            &simulator::initiate_network_construction,
            dlg.selected_experiment(),
            does_pause_apply_to_network_open()
            );
}

void  menu_bar::on_menu_network_close()
{
    wnd()->glwindow().call_later(&simulator::destroy_network);
}


void  menu_bar::save()
{
    wnd()->ptree().put("network.pause_construction",m_pause_applies_to_network_open);
}


void  make_menu_bar_content(menu_bar const&  w)
{
    w.get_menu_network()->addAction(w.get_action_network_open());
    QObject::connect(w.get_action_network_open(), &QAction::triggered, w.wnd(),&program_window::on_menu_network_open);

    w.get_menu_network()->addAction(w.get_action_network_close());
    QObject::connect(w.get_action_network_close(), &QAction::triggered, w.wnd(),&program_window::on_menu_network_close);

    w.get_menu_bar()->addMenu(w.get_menu_network());

    w.wnd()->setMenuBar(w.get_menu_bar());
}
