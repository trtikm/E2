#ifndef COM_CONSOLE_HPP_INCLUDED
#   define COM_CONSOLE_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <string>
#   include <vector>
#   include <deque>

namespace com { struct  simulator; }


namespace com {


struct  console
{
    console();
    std::string  update(simulator&  sim);
    std::string  text() const;
    void  paste_to_command_line(std::string const&  txt);
private:
    std::string  scene_path_to_string() const;
    void  validate_scene_path(simulator const&  sim);
    void  push_to_history(std::string const&  txt);
    void  push_to_commnad_history(std::string const&  txt);
    bool  on_tab(simulator const&  sim);
    void  on_double_tab(simulator const&  sim);
    void  execute_command_line(simulator&  sim);
    void  execute_help(std::vector<std::string> const&  words, simulator const&  sim);
    void  execute_ls(std::vector<std::string> const&  words, simulator const&  sim);
    void  execute_cd(std::vector<std::string> const&  words, simulator const&  sim);

    natural_32_bit  m_cursor;
    std::string  m_command_line;
    std::vector<std::string>  m_scene_path;
    std::deque<std::string>  m_history;
    std::deque<std::string>  m_command_history;
    integer_32_bit  m_command_history_index;
    float_32_bit  m_seconds_since_last_tab_key;
};


}

#endif
