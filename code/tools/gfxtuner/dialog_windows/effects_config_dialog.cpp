#include <gfxtuner/dialog_windows/effects_config_dialog.hpp>
#include <qtgl/gui_utils.hpp>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
//#include <QCheckBox>
//#include <QLineEdit>
#include <QGroupBox>
#include <QString>
#include <algorithm>

namespace dialog_windows {


std::pair<std::vector<std::string>, std::vector<qtgl::SHADER_DATA_INPUT_TYPE> >  effects_config_dialog::s_shader_data_input_types = {
    {
        "uniform",
        "buffer",
        "texture",
        "instance"
    },
    {
        qtgl::SHADER_DATA_INPUT_TYPE::UNIFORM,
        qtgl::SHADER_DATA_INPUT_TYPE::BUFFER,
        qtgl::SHADER_DATA_INPUT_TYPE::TEXTURE,
        qtgl::SHADER_DATA_INPUT_TYPE::INSTANCE
    }
};
//std::map<std::string, qtgl::SHADER_DATA_INPUT_TYPE>  effects_config_dialog::s_shader_data_input_types = {
//    {"uniform", qtgl::SHADER_DATA_INPUT_TYPE::UNIFORM },
//    {"buffer", qtgl::SHADER_DATA_INPUT_TYPE::BUFFER },
//    {"texture", qtgl::SHADER_DATA_INPUT_TYPE::TEXTURE },
//    {"instance", qtgl::SHADER_DATA_INPUT_TYPE::INSTANCE }
//};


effects_config_dialog::effects_config_dialog(
        program_window* const  wnd,
        qtgl::effects_config_data const&  old_effects_data,
        std::string const&  old_skin_name,
        std::vector<std::string> const&  available_skin_names
        )
    : QDialog(wnd)
    , m_wnd(wnd)
    , m_ok(false)
    , m_widget_ok(
            [](effects_config_dialog* wnd) {
                    struct OK : public QPushButton {
                        OK(effects_config_dialog* wnd) : QPushButton("OK")
                        {
                            QObject::connect(this, SIGNAL(released()), wnd, SLOT(accept()));
                        }
                    };
                    return new OK(wnd);
                }(this)
            )

    , m_old_effects_data(old_effects_data)
    , m_old_skin_name(old_skin_name)
    , m_new_effects_data()
    , m_new_skin_name()
    , m_available_skin_names(available_skin_names)

    , m_skins_combo_box(new QComboBox)
    , m_ligh_type_check_boxes{
            { qtgl::LIGHT_TYPE::AMBIENT, new QCheckBox("Ambient") },
            { qtgl::LIGHT_TYPE::DIRECTIONAL, new QCheckBox("Directional") }
            }
    , m_ligh_data_type_combo_boxes{
            { qtgl::LIGHTING_DATA_TYPE::DIFFUSE, new QComboBox },
            { qtgl::LIGHTING_DATA_TYPE::NORMAL, new QComboBox },
            { qtgl::LIGHTING_DATA_TYPE::DIRECTION, new QComboBox },
            { qtgl::LIGHTING_DATA_TYPE::SPECULAR, new QComboBox },
            }
    , m_lighting_algo_location_combo_box(new QComboBox)
    , m_shader_output_types_check_boxes{
            { qtgl::SHADER_DATA_OUTPUT_TYPE::DEFAULT, new QCheckBox("Default") },
            { qtgl::SHADER_DATA_OUTPUT_TYPE::TEXTURE_POSITION, new QCheckBox("Position texture") },
            { qtgl::SHADER_DATA_OUTPUT_TYPE::TEXTURE_NORMAL, new QCheckBox("Normal texture") },
            { qtgl::SHADER_DATA_OUTPUT_TYPE::TEXTURE_DIFFUSE, new QCheckBox("Diffuse texture") },
            { qtgl::SHADER_DATA_OUTPUT_TYPE::TEXTURE_SPECULAR, new QCheckBox("Specular texture") },
            }
    , m_fog_type_combo_box(new QComboBox)
    , m_fog_algo_location_combo_box(new QComboBox)
{
    ASSUMPTION(std::find(m_available_skin_names.cbegin(), m_available_skin_names.cend(), m_old_skin_name) != m_available_skin_names.cend());

    QVBoxLayout* const dlg_layout = new QVBoxLayout;
    {
        QHBoxLayout* const skin_layout = new QHBoxLayout;
        {
            QLabel* const  skin_label = new QLabel("Skin");
            skin_label->setToolTip(
                "A 'skin' is a pre-defined collection of textures and\n"
                "alpha-blending/testing setup. Each 'batch' contains\n"
                "at least one skin, each uniquely identified by a name.\n"
                "Exactly one of the skin names must be 'default'."
                );
            skin_layout->addWidget(skin_label);

            for (auto const&  skin_name : m_available_skin_names)
                m_skins_combo_box->addItem(skin_name.c_str());
            m_skins_combo_box->setCurrentIndex(
                    std::distance(
                            m_available_skin_names.cbegin(),
                            std::find(m_available_skin_names.cbegin(), m_available_skin_names.cend(), m_old_skin_name)
                            )
                    );
            skin_layout->addWidget(m_skins_combo_box);

            skin_layout->addStretch(1);
        }
        dlg_layout->addLayout(skin_layout);

        QVBoxLayout* const effects_layout = new QVBoxLayout;
        {
            QHBoxLayout* const horisontal_layout = new QHBoxLayout;
            {
                QVBoxLayout* const vertical_layout = new QVBoxLayout;
                {
                    QWidget* const light_types_group = new QGroupBox("Light types");
                    {
                        light_types_group->setToolTip("Light types which will be considered in execution of batch's vertex/fragment program.");
                        QHBoxLayout* const light_types_layout = new QHBoxLayout;
                        {
                            for (auto const&  type_and_widget : m_ligh_type_check_boxes)
                            {
                                light_types_layout->addWidget(type_and_widget.second);
                                type_and_widget.second->setCheckState(
                                        m_old_effects_data.get_light_types().count(type_and_widget.first) ?
                                                Qt::CheckState::Checked : Qt::CheckState::Unchecked
                                        );
                            }
                            light_types_layout->addStretch(1);
                        }
                        light_types_group->setLayout(light_types_layout);
                    }
                    vertical_layout->addWidget(light_types_group);

                    QHBoxLayout* const lighting_algo_layout = new QHBoxLayout;
                    {
                        QLabel* const  label = new QLabel("Lighting program");
                        label->setToolTip(
                            "Choose where to compute the lighting. Either in vertex or fragment program."
                            );
                        lighting_algo_layout->addWidget(label);

                        m_lighting_algo_location_combo_box->addItem("vertex");
                        m_lighting_algo_location_combo_box->addItem("fragment");
                        m_lighting_algo_location_combo_box->setCurrentIndex((int)m_old_effects_data.get_lighting_algo_location());
                        lighting_algo_layout->addWidget(m_lighting_algo_location_combo_box);

                        lighting_algo_layout->addStretch(1);
                    }
                    vertical_layout->addLayout(lighting_algo_layout);

                    QHBoxLayout* const fog_type_layout = new QHBoxLayout;
                    {
                        QLabel* const  label = new QLabel("Fog type");
                        label->setToolTip(
                            "Type of fog algorithm. 'NONE' means don't use fog at all."
                            );
                        fog_type_layout->addWidget(label);

                        m_fog_type_combo_box->addItem("NONE");
                        m_fog_type_combo_box->addItem("interpolated");
                        m_fog_type_combo_box->addItem("detailed");
                        m_fog_type_combo_box->setCurrentIndex((int)m_old_effects_data.get_fog_type());
                        fog_type_layout->addWidget(m_fog_type_combo_box);

                        fog_type_layout->addStretch(1);
                    }
                    vertical_layout->addLayout(fog_type_layout);

                    QHBoxLayout* const fog_algo_layout = new QHBoxLayout;
                    {
                        QLabel* const  label = new QLabel("Fog program");
                        label->setToolTip(
                            "Choose where to compute the fog. Either in vertex or fragment program."
                            );
                        fog_algo_layout->addWidget(label);

                        m_fog_algo_location_combo_box->addItem("vertex");
                        m_fog_algo_location_combo_box->addItem("fragment");
                        m_fog_algo_location_combo_box->setCurrentIndex((int)m_old_effects_data.get_fog_algo_location());
                        fog_algo_layout->addWidget(m_fog_algo_location_combo_box);

                        fog_algo_layout->addStretch(1);
                    }
                    vertical_layout->addLayout(fog_algo_layout);

                    //vertical_layout->addStretch(1);
                }
                horisontal_layout->addLayout(vertical_layout);

                QWidget* const light_data_types_group = new QGroupBox("Lighting data locations");
                {
                    light_data_types_group->setToolTip(
                        "Binding locations of lighting-related data to inputs variables of vertex/fragment program.\n"
                        "For each data type, any of these values can be selected:\n"
                        "   uniform         - Data are stored in vertex/fragment program's 'uniform' variable of any type, except 'sampler?D'.\n"
                        "   buffer          - Data are passed to a vertex/fragment program via a varying 'in' variable.\n"
                        "   texture         - Data are stored in a ?D texture (always per fragment data), i.e. in a uniform of a 'sampler?D' type.\n"
                        "   instance        - Data are passed to a vertex/fragment program via a varying 'in' variable; data changes only per instance.\n"
                        "   NONE            - Data are NOT passed to the vertex/fragment program at all.\n"
                        );
                    QVBoxLayout* const light_data_types_layout = new QVBoxLayout;
                    {
                        auto const add_values_to_combo_box = [](QComboBox* const  combo_box)
                        {
                            for (auto const&  name : s_shader_data_input_types.first)
                                combo_box->addItem(name.c_str());
                            combo_box->addItem("NONE");
                        };
                        auto const set_current_value_in_combo_box = [this](QComboBox* const  combo_box, qtgl::LIGHTING_DATA_TYPE const  type)
                        {
                            auto it = m_old_effects_data.get_lighting_data_types().find(type);
                            if (it == m_old_effects_data.get_lighting_data_types().cend())
                                combo_box->setCurrentText("NONE");
                            else
                            {
                                auto const  begin = m_old_effects_data.get_lighting_data_types().begin();
                                combo_box->setCurrentText(s_shader_data_input_types.first.at(std::distance(begin, it)).c_str());
                            }
                        };

                        {
                            QHBoxLayout* const  layout = new QHBoxLayout;

                            QLabel* const  label = new QLabel("Light direction");
                            label->setToolTip("Binding location for the direction vector of the directional light.");
                            layout->addWidget(label);

                            QComboBox* const  combo_box = m_ligh_data_type_combo_boxes.at(qtgl::LIGHTING_DATA_TYPE::DIRECTION);
                            add_values_to_combo_box(combo_box);
                            set_current_value_in_combo_box(combo_box, qtgl::LIGHTING_DATA_TYPE::DIRECTION);
                            layout->addWidget(combo_box);
                            layout->addStretch(1);
                            light_data_types_layout->addLayout(layout);
                        }
                        {
                            QHBoxLayout* const  layout = new QHBoxLayout;

                            QLabel* const  label = new QLabel("Normals");
                            label->setToolTip("Binding location for the normal vectors of rendered surfaces.");
                            layout->addWidget(label);

                            QComboBox* const  combo_box = m_ligh_data_type_combo_boxes.at(qtgl::LIGHTING_DATA_TYPE::NORMAL);
                            add_values_to_combo_box(combo_box);
                            set_current_value_in_combo_box(combo_box, qtgl::LIGHTING_DATA_TYPE::NORMAL);
                            layout->addWidget(combo_box);
                            layout->addStretch(1);
                            light_data_types_layout->addLayout(layout);
                        }
                        {
                            QHBoxLayout* const  layout = new QHBoxLayout;

                            QLabel* const  label = new QLabel("Diffuse");
                            label->setToolTip("Binding location for the diffuse colours of rendered surfaces.");
                            layout->addWidget(label);

                            QComboBox* const  combo_box = m_ligh_data_type_combo_boxes.at(qtgl::LIGHTING_DATA_TYPE::DIFFUSE);
                            add_values_to_combo_box(combo_box);
                            set_current_value_in_combo_box(combo_box, qtgl::LIGHTING_DATA_TYPE::DIFFUSE);
                            layout->addWidget(combo_box);
                            layout->addStretch(1);
                            light_data_types_layout->addLayout(layout);
                        }
                        {
                            QHBoxLayout* const  layout = new QHBoxLayout;

                            QLabel* const  label = new QLabel("Specular");
                            label->setToolTip("Binding location for the specular colours of rendered surfaces.");
                            layout->addWidget(label);

                            QComboBox* const  combo_box = m_ligh_data_type_combo_boxes.at(qtgl::LIGHTING_DATA_TYPE::SPECULAR);
                            add_values_to_combo_box(combo_box);
                            set_current_value_in_combo_box(combo_box, qtgl::LIGHTING_DATA_TYPE::SPECULAR);
                            layout->addWidget(combo_box);
                            layout->addStretch(1);
                            light_data_types_layout->addLayout(layout);
                        }
                    }
                    light_data_types_group->setLayout(light_data_types_layout);
                }
                horisontal_layout->addWidget(light_data_types_group);

                QWidget* const  shader_output_group = new QGroupBox("Fragment program output");
                {
                    shader_output_group->setToolTip(
                        "Where the fragment program should write the computed fragments. 'Default' means the screen."
                        );
                    QVBoxLayout* const shader_output_layout = new QVBoxLayout;
                    {
                        for (auto const&  type_and_widget : m_shader_output_types_check_boxes)
                        {
                            shader_output_layout->addWidget(type_and_widget.second);
                            type_and_widget.second->setCheckState(
                                    m_old_effects_data.get_shader_output_types().count(type_and_widget.first) ?
                                            Qt::CheckState::Checked : Qt::CheckState::Unchecked
                                    );
                        }
                        shader_output_layout->addStretch(1);
                    }
                    shader_output_group->setLayout(shader_output_layout);
                }
                horisontal_layout->addWidget(shader_output_group);

                //horisontal_layout->addStretch(1);
            }
            effects_layout->addLayout(horisontal_layout);

            //dlg_layout->addStretch(1);
        }
        dlg_layout->addLayout(effects_layout);

        QWidget* const note_group = new QGroupBox();
        {
            QVBoxLayout* const note_layout = new QVBoxLayout;
            {
                note_layout->addWidget(new QLabel(
                    "NOTE: Not all commbinations of values in this dialog are supported. Therefore, the combination\n"
                    "you set here is only used as a search key to a dictionary of supported combinations - the closest\n"
                    "supported combination to yours it then actually used."
                    ));
            }
            note_group->setLayout(note_layout);
        }
        dlg_layout->addWidget(note_group);

        QHBoxLayout* const buttons_layout = new QHBoxLayout;
        {
            buttons_layout->addWidget(m_widget_ok);
            buttons_layout->addWidget(
                [](effects_config_dialog* wnd) {
                    struct Close : public QPushButton {
                        Close(effects_config_dialog* wnd) : QPushButton("Cancel")
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
    this->setWindowTitle("Effects config");
}


void  effects_config_dialog::accept()
{
    m_new_skin_name = m_available_skin_names.at(m_skins_combo_box->currentIndex());

    for (auto const&  type_and_widget : m_ligh_type_check_boxes)
        if (type_and_widget.second->isChecked())
            m_new_effects_data.get_light_types().insert(type_and_widget.first);

    for (auto const& type_and_widget : m_ligh_data_type_combo_boxes)
    {
        std::string const&  location_name = qtgl::to_string(type_and_widget.second->currentText());
        auto const  it = std::find(s_shader_data_input_types.first.cbegin(), s_shader_data_input_types.first.cend(), location_name);
        if (it != s_shader_data_input_types.first.cend())
            m_new_effects_data.get_lighting_data_types().insert({
                    type_and_widget.first,
                    s_shader_data_input_types.second.at(std::distance(s_shader_data_input_types.first.cbegin(), it))
                    });
    }

    m_new_effects_data.set_lighting_algo_location((qtgl::SHADER_PROGRAM_TYPE)m_lighting_algo_location_combo_box->currentIndex());

    for (auto const& type_and_widget : m_shader_output_types_check_boxes)
        if (type_and_widget.second->isChecked())
            m_new_effects_data.get_shader_output_types().insert(type_and_widget.first);

    m_new_effects_data.set_fog_type((qtgl::FOG_TYPE)m_fog_type_combo_box->currentIndex());

    m_new_effects_data.set_fog_algo_location((qtgl::SHADER_PROGRAM_TYPE)m_fog_algo_location_combo_box->currentIndex());

    m_ok = true;
    QDialog::accept();
}


void  effects_config_dialog::reject()
{
    QDialog::reject();
}


}
