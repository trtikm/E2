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


std::pair<object_guid, std::string>  build_path(
        std::vector<std::string> const&  words,
        natural_32_bit const  arg_index,
        std::string const&  current_dir,
        simulation_context const&  ctx
        )
{
    std::string  path, last;
    bool  ends_by_slash = false;
    if (words.size() > arg_index)
    {
        std::vector<std::string>  dirs;
        detail::split_string(words.at(arg_index), '/', dirs);
        ends_by_slash = words.at(arg_index).back() == '/';
        if (dirs.empty())
            path = "/";
        else
        {
            last = dirs.back();
            dirs.pop_back();

            std::stringstream  sstr;
            sstr << (words.at(arg_index).front() == '/' ? "/" : current_dir);
            for (std::string const&  dir : dirs)
                sstr << dir << '/';
            path = sstr.str();
        }
    }
    else
        path = current_dir;
    INVARIANT(!path.empty());

    object_guid  folder_guid = ctx.from_absolute_path(path + last + '/');
    if (folder_guid != invalid_object_guid())
    {
        if (!ends_by_slash && !last.empty())
        {
            object_guid const  parent_folder_guid = ctx.from_absolute_path(path);
            simulation_context::folder_content_type const&  content = ctx.folder_content(parent_folder_guid);
            std::vector<std::string>  candidates;
            for (auto const&  name_and_guid : content.child_folders)
                if (boost::starts_with(name_and_guid.first, last))
                    candidates.push_back(name_and_guid.first);
            if (candidates.size() > 1UL)
                return { parent_folder_guid, last };
        }
        return { folder_guid, "" };
    }
    folder_guid = ctx.from_absolute_path(path);
    return { folder_guid, last };
}


}}

namespace com {


console::console()
    : m_cursor(0U)
    , m_command_line()
    , m_scene_path()
    , m_history()
    , m_seconds_since_last_tab_key(-1.0f)
{
}


std::string  console::update(simulator&  sim)
{
    TMPROF_BLOCK();

    validate_scene_path(sim);

    if (m_seconds_since_last_tab_key >= 0.0f)
        m_seconds_since_last_tab_key = (m_seconds_since_last_tab_key + sim.round_seconds() > 0.5f) ?
                -1.0f : m_seconds_since_last_tab_key + sim.round_seconds();

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
        else if (kb.keys_just_pressed().count(osi::KEY_TAB()))
        {
            if (m_cursor == m_command_line.size())
            {
                if (m_seconds_since_last_tab_key < 0.0f)
                {
                    if (on_tab(sim))
                        m_cursor = (natural_32_bit)m_command_line.size();
                    else
                        m_seconds_since_last_tab_key = 0.0f;
                }
                else
                {
                    on_double_tab(sim);
                    m_seconds_since_last_tab_key = -1.0f;
                }
            }
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


bool  console::on_tab(simulator const&  sim)
{
    std::vector<std::string>  words;
    detail::split_string(m_command_line, ' ', words);
    if (words.empty())
        return false;
    if (words.front() == "ls" || words.front() == "cd")
    {
        simulation_context const&  ctx = *sim.context();
        auto const  folder_guid_and_last = detail::build_path(words, 1U, scene_path_to_string(), ctx);
        if (folder_guid_and_last.first != invalid_object_guid() && !folder_guid_and_last.second.empty())
        {
            simulation_context::folder_content_type const&  content = ctx.folder_content(folder_guid_and_last.first);
            std::vector<std::string>  candidates;
            for (auto const&  name_and_guid : content.child_folders)
                if (boost::starts_with(name_and_guid.first, folder_guid_and_last.second))
                    candidates.push_back(name_and_guid.first);
            if (!candidates.empty())
            {
                std::stringstream  sstr;
                natural_32_bit  i = 0U;
                for ( ; true; ++i)
                {
                    bool  match = true;
                    for (std::string const&  txt : candidates)
                        if (txt.size() <= i || txt.at(i) != candidates.front().at(i))
                        {
                            match = false;
                            break;
                        }
                    if (!match)
                        break;
                }
                std::string const  name = candidates.front().substr(0UL, i);
                if (name.size() > folder_guid_and_last.second.size())
                {
                    natural_32_bit  longest_candidate = (natural_32_bit)candidates.front().size();
                    for (std::string const&  txt : candidates)
                        longest_candidate = std::max(longest_candidate, (natural_32_bit)txt.size());
                    m_command_line = m_command_line.substr(0UL, m_command_line.size() - folder_guid_and_last.second.size()) + name;
                    if (longest_candidate == name.size())
                        m_command_line.push_back('/');
                    return true;
                }
            }
        }
    }
    return false;
}


void  console::on_double_tab(simulator const&  sim)
{
    std::vector<std::string>  words;
    detail::split_string(m_command_line, ' ', words);
    if (words.empty() || words.size() > 2UL)
        return;
    if (words.front() == "ls" || words.front() == "cd")
    {
        simulation_context const&  ctx = *sim.context();
        auto const  folder_guid_and_last = detail::build_path(words, 1U, scene_path_to_string(), ctx);
        if (folder_guid_and_last.first != invalid_object_guid())
        {
            push_to_history(scene_path_to_string() + '#' + m_command_line);
            simulation_context::folder_content_type const&  content = ctx.folder_content(folder_guid_and_last.first);
            for (auto const&  name_and_guid : content.child_folders)
                if (folder_guid_and_last.second.empty() || boost::starts_with(name_and_guid.first, folder_guid_and_last.second))
                    push_to_history(msgstream() << '<' << to_string(OBJECT_KIND::FOLDER) << "> " << name_and_guid.first);
        }
    }
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
    push_to_history("ls [<path>]            Lists content of the folder given the");
    push_to_history("                       passed <path>. If no path is passed,");
    push_to_history("                       the <path> is set to the path '.'.");
    push_to_history("cd <path>              Change the current scene folder based");
    push_to_history("                       on the passed <path>, which may be");
    push_to_history("                       either absolute (starting with '/')");
    push_to_history("                       or relative.");
    push_to_history("");
    push_to_history("Special keys:");
    push_to_history("TAB                    Auto-complete of paths in commands");
    push_to_history("                       ls and cd based on the typed prefix.");
    push_to_history("TAB+TAB                When TAB key is pressed twice in short");
    push_to_history("                       time window, then folders of the typed");
    push_to_history("                       path are printed. This works only for");
    push_to_history("                       commands ls and cd.");
    //push_to_history("");
}


void  console::execute_ls(std::vector<std::string> const&  words, simulator const&  sim)
{
    if (words.size() > 2ULL)
    {
        push_to_history("ls: Invalid number of parameters; none or a path is expected.");
        return;
    }

    std::string const  path = words.size() == 1UL ? scene_path_to_string() :
            (words.at(1).front() == '/' ? "" : scene_path_to_string()) +
            words.at(1) +
            (words.at(1).back() == '/' ? "" : "/")
            ;

    simulation_context const&  ctx = *sim.context();
    object_guid const  current_folder_guid = ctx.from_absolute_path(path);
    if (ctx.is_valid_folder_guid(current_folder_guid))
    {
        simulation_context::folder_content_type const&  content = ctx.folder_content(current_folder_guid);
        for (auto const&  name_and_guid : content.child_folders)
            push_to_history(msgstream() << '<' << to_string(OBJECT_KIND::FOLDER) << "> " << name_and_guid.first);
        for (auto const&  kind_and_names : content.content_index)
            for (auto const&  name : kind_and_names.second)
                push_to_history(msgstream() << '<' << to_string(kind_and_names.first) << "> " << name);
    }
    else
        push_to_history(msgstream() << "Error: The path '" << path << "' is NOT a valid folder.");
}


void  console::execute_cd(std::vector<std::string> const&  words, simulator const&  sim)
{
    if (words.size() != 2ULL)
    {
        push_to_history("cd: Invalid number of parameters; a path is expected.");
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
