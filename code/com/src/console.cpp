#include <com/console.hpp>
#include <com/simulator.hpp>
#include <com/simulation_context.hpp>
#include <utility/msgstream.hpp>
#include <utility/timeprof.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <boost/algorithm/string.hpp>
#include <sstream>

namespace com { namespace detail {


void  split_string(std::string const&  txt, natural_8_bit const  by_char, std::vector<std::string>&  output_words)
{
    std::vector<std::string>  words;
    boost::split(words, txt, [by_char](std::string::value_type c) -> bool { return c == by_char; });
    for (std::string const&  raw_word : words)
    {
        std::string const word = boost::trim_copy(raw_word);
        if (!word.empty())
            output_words.push_back(word);
    }
}


}}

namespace com {


console::console()
    : m_cursor(0U)
    , m_command_line()
    , m_scene_path()
    , m_history()
{
}


std::string  console::update(simulator&  sim)
{
    TMPROF_BLOCK();

    validate_scene_path(sim);

    std::string const&  typed_text = sim.get_keyboard_props().typed_text();
    if (typed_text.empty())
    {
        osi::keyboard_props const&  kb = sim.get_keyboard_props();
        if (kb.keys_just_pressed().count(osi::KEY_RETURN()))
        {
            push_to_history(scene_path_to_string() + '#' + m_command_line);
            execute_command_line(sim);
            m_command_line.clear();
            m_cursor = 0U;
        }
        else if (kb.keys_just_pressed().count(osi::KEY_LEFT()) && m_cursor > 0U)
            --m_cursor;
        else if (kb.keys_just_pressed().count(osi::KEY_RIGHT()) && m_cursor < (natural_32_bit)m_command_line.size())
            ++m_cursor;
        else if (kb.keys_just_pressed().count(osi::KEY_BACKSPACE()) && m_cursor > 0U)
        {
            m_command_line.erase(std::next(m_command_line.begin(), m_cursor - 1U));
            --m_cursor;
        }
        else if (kb.keys_just_pressed().count(osi::KEY_DELETE()) && m_cursor < (natural_32_bit)m_command_line.size())
            m_command_line.erase(std::next(m_command_line.begin(), m_cursor));
        else if (kb.keys_just_pressed().count(osi::KEY_HOME()))
            m_cursor = 0U;
        else if (kb.keys_just_pressed().count(osi::KEY_END()))
            m_cursor = (natural_32_bit)m_command_line.size();
    }
    else
    {
        m_command_line = m_command_line.substr(0U, m_cursor) + typed_text + m_command_line.substr(m_cursor);
        m_cursor += (natural_32_bit)typed_text.size();
    }

    return text();
}


std::string  console::text() const
{
    std::stringstream  sstr;
    for (auto  rit = m_history.rbegin(); rit != m_history.rend(); ++rit)
        sstr << *rit << std::endl;
    sstr << scene_path_to_string()
         << '#'
         << m_command_line.substr(0U, m_cursor)
         << (natural_8_bit)0xffU
         << m_command_line.substr(m_cursor)
         ;
    return sstr.str();
}


std::string  console::scene_path_to_string() const
{
    std::stringstream  sstr;
    sstr << '/';
    for (std::string const&  txt : m_scene_path)
        sstr << txt << '/';
    return sstr.str();
}


void  console::validate_scene_path(simulator const&  sim)
{
    simulation_context const&  ctx = *sim.context();
    while (!m_scene_path.empty())
    {
        if (ctx.is_valid_folder_guid(ctx.from_absolute_path(scene_path_to_string())))
            break;
        m_scene_path.pop_back();
    }
}


void  console::push_to_history(std::string const&  txt)
{
    m_history.push_front(txt);
    if (m_history.size() > 100ULL)
        m_history.pop_back();
}


void  console::execute_command_line(simulator&  sim)
{
    std::vector<std::string>  words;
    detail::split_string(m_command_line, ' ', words);
    if (words.empty())
        return;
    if (words.front() == "help")
        execute_help(words, sim);
    else if (words.front() == "ls")
        execute_ls(words, sim);
    else if (words.front() == "cd")
        execute_cd(words, sim);
    else
        push_to_history(msgstream() << "Unknown command '" << words.front() << "'. Use the command 'help'.");
}


void  console::execute_help(std::vector<std::string> const&  words, simulator const&  sim)
{
    push_to_history("Available commands:");
    push_to_history("help                   Print this help message.");
    push_to_history("ls                     List current scene folder.");
    push_to_history("cd <path>              Change the current scene folder based");
    push_to_history("                       on the passed <path>, which may be");
    push_to_history("                       either absolute (starting with '/')");
    push_to_history("                       or relative.");
}


void  console::execute_ls(std::vector<std::string> const&  words, simulator const&  sim)
{
    if (words.size() != 1ULL)
    {
        push_to_history("ls: Invalid number of parameters (0 is expected).");
        return;
    }

    simulation_context const&  ctx = *sim.context();
    object_guid const  current_folder_guid = ctx.from_absolute_path(scene_path_to_string());
    simulation_context::folder_content_type const&  content = ctx.folder_content(current_folder_guid);
    for (auto const&  name_and_guid : content.child_folders)
        push_to_history(msgstream() << '<' << to_string(OBJECT_KIND::FOLDER) << "> " << name_and_guid.first);
    for (auto const&  kind_and_names : content.content_index)
        for (auto const&  name : kind_and_names.second)
            push_to_history(msgstream() << '<' << to_string(kind_and_names.first) << "> " << name);
}


void  console::execute_cd(std::vector<std::string> const&  words, simulator const&  sim)
{
    if (words.size() != 2ULL)
    {
        push_to_history("cd: Invalid number of parameters (a path is expected).");
        return;
    }

    std::string const  path =
            (words.at(1).front() == '/' ? "" : scene_path_to_string()) +
            words.at(1) +
            (words.at(1).back() == '/' ? "" : "/")
            ;

    simulation_context const&  ctx = *sim.context();
    object_guid const  folder_guid = ctx.from_absolute_path(path);
    if (ctx.is_valid_folder_guid(folder_guid))
    {
        m_scene_path.clear();
        detail::split_string(ctx.to_absolute_path(folder_guid), '/', m_scene_path);
    }
    else
        push_to_history(msgstream() << "Error: The path '" << path << "' is NOT a valid folder.");
}


}
