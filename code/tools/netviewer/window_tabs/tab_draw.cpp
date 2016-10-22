#include <netviewer/window_tabs/tab_draw.hpp>
#include <netviewer/program_window.hpp>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QColorDialog>
#include <QString>
#include <QIntValidator>

namespace window_tabs { namespace tab_draw {


widgets::widgets(program_window* const  wnd)
    : m_wnd(wnd)

    , m_clear_colour_component_red(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setValidator(new QIntValidator(0, 255));
                    setText(QString::number(wnd->ptree().get("draw.clear_colour.red", 64)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_clear_colour_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_clear_colour_component_green(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setValidator(new QIntValidator(0, 255));
                    setText(QString::number(wnd->ptree().get("draw.clear_colour.green", 64)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_clear_colour_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
    , m_clear_colour_component_blue(
        [](program_window* wnd) {
            struct s : public QLineEdit {
                s(program_window* wnd) : QLineEdit()
                {
                    setValidator(new QIntValidator(0, 255));
                    setText(QString::number(wnd->ptree().get("draw.clear_colour.blue", 64)));
                    QObject::connect(this, SIGNAL(editingFinished()), wnd, SLOT(on_clear_colour_changed()));
                }
            };
            return new s(wnd);
        }(m_wnd)
        )
{

}

program_window*  widgets::wnd() const noexcept
{
    return m_wnd;
}

QLineEdit* widgets::clear_colour_component_red() const noexcept
{
    return m_clear_colour_component_red;
}

QLineEdit* widgets::clear_colour_component_green() const noexcept
{
    return m_clear_colour_component_green;
}

QLineEdit* widgets::clear_colour_component_blue() const noexcept
{
    return m_clear_colour_component_blue;
}

void widgets::on_clear_colour_changed()
{
    vector3 const  colour(
        (float_32_bit)m_clear_colour_component_red->text().toInt() / 255.0f,
        (float_32_bit)m_clear_colour_component_green->text().toInt() / 255.0f,
        (float_32_bit)m_clear_colour_component_blue->text().toInt() / 255.0f
        );
    wnd()->glwindow().call_later(&simulator::set_clear_color, colour);
}

void widgets::on_clear_colour_set(QColor const&  colour)
{
    m_clear_colour_component_red->setText(QString::number(colour.red()));
    m_clear_colour_component_green->setText(QString::number(colour.green()));
    m_clear_colour_component_blue->setText(QString::number(colour.blue()));
    on_clear_colour_changed();
}

void widgets::on_clear_colour_choose()
{
    QColor const  init_colour(m_clear_colour_component_red->text().toInt(),
        m_clear_colour_component_green->text().toInt(),
        m_clear_colour_component_blue->text().toInt());
    QColor const  colour = QColorDialog::getColor(init_colour, wnd(), "Choose clear colour");
    if (!colour.isValid())
        return;
    on_clear_colour_set(colour);
}

void widgets::on_clear_colour_reset()
{
    on_clear_colour_set(QColor(64, 64, 64));
}

void  widgets::save()
{
    wnd()->ptree().put("draw.clear_colour.red", m_clear_colour_component_red->text().toInt());
    wnd()->ptree().put("draw.clear_colour.green", m_clear_colour_component_green->text().toInt());
    wnd()->ptree().put("draw.clear_colour.blue", m_clear_colour_component_blue->text().toInt());
}

QWidget*  make_draw_tab_content(widgets const&  w)
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
                        edit_boxes_layout->addWidget(w.clear_colour_component_red());
                        edit_boxes_layout->addWidget(w.clear_colour_component_green());
                        edit_boxes_layout->addWidget(w.clear_colour_component_blue());
                        w.wnd()->on_clear_colour_changed();
                    }
                    clear_colour_layout->addLayout(edit_boxes_layout);

                    QHBoxLayout* const buttons_layout = new QHBoxLayout;
                    {
                        buttons_layout->addWidget(
                            [](program_window* wnd) {
                                struct choose : public QPushButton {
                                    choose(program_window* wnd) : QPushButton("Choose")
                                    {
                                        QObject::connect(this, SIGNAL(released()), wnd, SLOT(on_clear_colour_choose()));
                                    }
                                };
                                return new choose(wnd);
                            }(w.wnd())
                            );
                        buttons_layout->addWidget(
                            [](program_window* wnd) {
                                struct choose : public QPushButton {
                                    choose(program_window* wnd) : QPushButton("Default")
                                    {
                                        QObject::connect(this, SIGNAL(released()), wnd, SLOT(on_clear_colour_reset()));
                                    }
                                };
                                return new choose(wnd);
                            }(w.wnd())
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
    return draw_tab;
}


}}
