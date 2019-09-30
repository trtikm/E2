#ifndef E2_TOOL_GFXTUNER_DIALOG_WINDOWS_EFFECTS_CONFIG_DIALOG_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_DIALOG_WINDOWS_EFFECTS_CONFIG_DIALOG_HPP_INCLUDED

#   include <gfxtuner/program_window.hpp>
#   include <qtgl/effects_config.hpp>
#   include <QFileDialog>
#   include <QDialog>
#   include <QPushButton>
#   include <QComboBox>
#   include <QCheckBox>
#   include <vector>
#   include <map>
#   include <string>
#   include <utility>

namespace dialog_windows {


struct  effects_config_dialog : public QDialog
{
    effects_config_dialog(
            program_window* const  wnd,
            qtgl::effects_config_data const&  old_effects_data,
            std::string const&  old_skin_name,
            std::vector<std::string> const&  available_skin_names
            );

    bool  ok() const { return m_ok; }

    qtgl::effects_config_data const&  get_new_effects_data() const { return m_new_effects_data; }
    std::string const&  get_new_skin_name() const { return m_new_skin_name; }

public slots:

    void  accept();
    void  reject();

private:
    program_window*  m_wnd;
    bool  m_ok;
    QPushButton* m_widget_ok;

    qtgl::effects_config_data  m_old_effects_data;
    std::string  m_old_skin_name;
    qtgl::effects_config_data  m_new_effects_data;
    std::string  m_new_skin_name;
    std::vector<std::string>  m_available_skin_names;

    QComboBox*  m_skins_combo_box;
    std::map<qtgl::LIGHT_TYPE, QCheckBox*>  m_ligh_type_check_boxes;
    std::map<qtgl::LIGHTING_DATA_TYPE, QComboBox*>  m_ligh_data_type_combo_boxes;
    static std::pair<std::vector<std::string>, std::vector<qtgl::SHADER_DATA_INPUT_TYPE> >  s_shader_data_input_types;
    QComboBox*  m_lighting_algo_location_combo_box;
    std::map<qtgl::SHADER_DATA_OUTPUT_TYPE, QCheckBox*>  m_shader_output_types_check_boxes;
    QComboBox*  m_fog_type_combo_box;
    QComboBox*  m_fog_algo_location_combo_box;
};


}

#endif
