#include <com/console.hpp>
#include <com/simulator.hpp>
#include <com/simulation_context.hpp>
#include <ai/cortex_default.hpp>
#include <ai/cortex_mock.hpp>
#include <ai/cortex_netlab.hpp>
#include <ai/cortex_random.hpp>
#include <ai/cortex_robot.hpp>
#include <utility/msgstream.hpp>
#include <utility/timeprof.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <boost/algorithm/string.hpp>
#include <sstream>


namespace {


std::string  print_vector3(vector3 const&  v)
{
    return msgstream() << "[ " << v(0) << ", " << v(1) << ", " << v(2) << " ]";
}


std::string  print_quaternion(quaternion const&  q)
{
    return msgstream() << "[ " << q.w() << "; " << q.x() << ", " << q.y() << ", " << q.z() << " ]";
}


std::unordered_set<std::string> const&  get_tab_key_using_commands()
{
    static std::unordered_set<std::string> const  cmds {
        "ls", "cd", "tree",
        "pa", "pc", "pf"
    };
    return cmds;
}


bool is_tab_key_using_command(std::string const&  cmd)
{
    return get_tab_key_using_commands().count(cmd) != 0ULL;
}


}

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
    , m_command_history()
    , m_command_history_index(-1)
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
        if (kb.keys_pressed().count(osi::KEY_LCTRL()) || kb.keys_pressed().count(osi::KEY_RCTRL()))
        {
            auto const  is_white = [](char  c) { return c == ' ' || c == '\t'; };
            auto const  is_terminal = [](char  c) { return c == ' ' || c == '\t' || c == '/'; };

            if (kb.keys_just_pressed().count(osi::KEY_LEFT()))
            {
                if (m_cursor > 0U)
                {
                    if (is_white(m_command_line.at(m_cursor - 1U)))
                        do --m_cursor; while (m_cursor > 0U && is_white(m_command_line.at(m_cursor - 1U)));
                    else if (m_command_line.at(m_cursor - 1U) == '/')
                        --m_cursor;
                    else
                        do --m_cursor; while (m_cursor > 0U && !is_terminal(m_command_line.at(m_cursor - 1U)));
                }
            }
            else if (kb.keys_just_pressed().count(osi::KEY_RIGHT()))
            {
                if (m_cursor < (natural_32_bit)m_command_line.size())
                {
                    if (is_white(m_command_line.at(m_cursor)))
                        do ++m_cursor; while (m_cursor < (natural_32_bit)m_command_line.size() && is_white(m_command_line.at(m_cursor)));
                    else if (m_command_line.at(m_cursor) == '/')
                        ++m_cursor;
                    else
                        do ++m_cursor; while (m_cursor < (natural_32_bit)m_command_line.size() && !is_terminal(m_command_line.at(m_cursor)));
                }
            }
            else if (kb.keys_just_pressed().count(osi::KEY_BACKSPACE()))
            {
                if (m_cursor > 0U)
                {
                    natural_32_bit const  old_cursor = m_cursor;
                    if (is_white(m_command_line.at(m_cursor - 1U)))
                        do --m_cursor; while (m_cursor > 0U && is_white(m_command_line.at(m_cursor - 1U)));
                    else if (m_command_line.at(m_cursor - 1U) == '/')
                        --m_cursor;
                    else
                        do --m_cursor; while (m_cursor > 0U && !is_terminal(m_command_line.at(m_cursor - 1U)));
                    m_command_line.erase(m_cursor, old_cursor - m_cursor);
                }
            }
        }
        else
        {
            if (kb.keys_just_pressed().count(osi::KEY_RETURN()))
            {
                push_to_history(scene_path_to_string() + '#' + m_command_line);
                execute_command_line(sim);
                m_command_line.clear();
                m_cursor = 0U;
            }
            else if (kb.keys_just_pressed().count(osi::KEY_ESCAPE()))
            {
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
            else if (kb.keys_just_pressed().count(osi::KEY_UP()))
            {
                if (m_command_history_index + 1 < (integer_32_bit)m_command_history.size())
                {
                    ++m_command_history_index;
                    m_command_line = m_command_history.at(m_command_history_index);
                    m_cursor = (natural_32_bit)m_command_line.size();
                }
            }
            else if (kb.keys_just_pressed().count(osi::KEY_DOWN()))
            {
                if (m_command_history_index > 0)
                {
                    --m_command_history_index;
                    m_command_line = m_command_history.at(m_command_history_index);
                    m_cursor = (natural_32_bit)m_command_line.size();
                }
                else if (m_command_history_index == 0)
                {
                    m_command_history_index = -1;
                    m_command_line.clear();
                    m_cursor = 0U;
                }
            }
            else if (kb.keys_just_pressed().count(osi::KEY_LEFT()) && m_cursor > 0U)
                --m_cursor;
            else if (kb.keys_just_pressed().count(osi::KEY_RIGHT()) && m_cursor < (natural_32_bit)m_command_line.size())
                ++m_cursor;
            else if (kb.keys_just_pressed().count(osi::KEY_BACKSPACE()) && m_cursor > 0U)
            {
                m_command_line.erase(m_cursor - 1U, 1ULL);
                --m_cursor;
            }
            else if (kb.keys_just_pressed().count(osi::KEY_DELETE()) && m_cursor < (natural_32_bit)m_command_line.size())
                m_command_line.erase(m_cursor, 1ULL);
            else if (kb.keys_just_pressed().count(osi::KEY_HOME()))
                m_cursor = 0U;
            else if (kb.keys_just_pressed().count(osi::KEY_END()))
                m_cursor = (natural_32_bit)m_command_line.size();
        }
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


void  console::paste_to_command_line(std::string const&  txt, bool const  replace_text_after_cursor, bool const  move_cursor)
{
    m_command_line = m_command_line.substr(0U, m_cursor) + ' ' + txt + (replace_text_after_cursor ? "" : m_command_line.substr(m_cursor));
    if (move_cursor)
        m_cursor += (natural_32_bit)txt.size() + 1U;
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
    if (m_history.size() > 500ULL)
        m_history.pop_back();
}


void  console::push_to_commnad_history(std::string const&  txt)
{
    std::deque<std::string>  tmp;
    tmp.swap(m_command_history);
    m_command_history.push_back(txt);
    for (std::string const&  cmd : tmp)
        if (cmd != txt)
            m_command_history.push_back(cmd);
    while (m_command_history.size() > 10ULL)
        m_command_history.pop_back();
    m_command_history_index = -1;
}


bool  console::on_tab(simulator const&  sim)
{
    std::vector<std::string>  words;
    detail::split_string(m_command_line, ' ', words);
    if (words.empty())
        return false;
    if (is_tab_key_using_command(words.front()))
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
    if (is_tab_key_using_command(words.front()))
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
    else if (words.front() == "pa")
        execute_pa(words, sim);
    else if (words.front() == "pb")
        execute_pb(words, sim);
    else if (words.front() == "pc")
        execute_pc(words, sim);
    else if (words.front() == "pf")
        execute_pf(words, sim);
    else if (words.front() == "pr")
        execute_pr(words, sim);
    else if (words.front() == "ps")
        execute_ps(words, sim);
    else if (words.front() == "pt")
        execute_pt(words, sim);
    else if (words.front() == "tree")
        execute_tree(words, sim);
    else
        push_to_history(msgstream() << "Unknown command '" << words.front() << "'. Use the command 'help'.");

    push_to_commnad_history(m_command_line);
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
    push_to_history("tree [<path>]          Prints the tree of the scene hierarchy");
    push_to_history("                       under the passed <path>. If no path is");
    push_to_history("                       passed, the <path> is set to '.'.");
    push_to_history("p<a|f|r> [<path>]      Prints properties of object type");
    push_to_history("                       AGENT|FRAME|RIGID_BODY located in the");
    push_to_history("                       <path> folder. If no path is passed,");
    push_to_history("                       the <path> is set to '.'.");
    push_to_history("p<b|c|s|t> [<name>]    Prints properties of object type");
    push_to_history("                       BATCH|COLLIDER|SENSOR|TIMER of the name");
    push_to_history("                       <name> located in the active folder.");
    push_to_history("                       If there is exactly one object of the");
    push_to_history("                       kind in the active folder, then <name>");
    push_to_history("                       can be omitted.");
    push_to_history("");
    push_to_history("Special keys:");
    push_to_history("TAB                    Auto-complete of paths in commands");
    push_to_history("                       accepting a path based on the typed prefix.");
    push_to_history("TAB+TAB                When TAB key is pressed twice in short");
    push_to_history("                       time window, then folders of the typed");
    push_to_history("                       path are printed. This works only for");
    push_to_history("                       commands ls and cd.");
    push_to_history("UP                     Overwrites the current command line by");
    push_to_history("                       the text of the previosly executed");
    push_to_history("                       command, if there was any. More times you");
    push_to_history("                       press the key, older command is used from");
    push_to_history("                       the commands history.");
    push_to_history("DOWN                   Same as the key UP, but it moves in the");
    push_to_history("                       commands history in the opposite direction.");
    push_to_history("CTRL                   With the key LEFT/RIGHT the cursor jumps");
    push_to_history("                       over whole worlds in left/right direction.");
    push_to_history("                       With BACKSPACE whole word in front of the");
    push_to_history("                       cursor is erased.");
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


void  console::execute_pa(std::vector<std::string> const&  words, simulator const&  sim)
{
    if (words.size() > 2ULL)
    {
        push_to_history("pa: The command accepts at most one argument (folder path).");
        return;
    }

    std::string const  path = words.size() == 1UL ? scene_path_to_string() :
            (words.at(1).front() == '/' ? "" : scene_path_to_string()) +
            words.at(1) +
            (words.at(1).back() == '/' ? "" : "/")
            ;

    simulation_context const&  ctx = *sim.context();
    object_guid const  current_folder_guid = ctx.from_absolute_path(path);
    if (!ctx.is_valid_folder_guid(current_folder_guid))
    {
        push_to_history(msgstream() << "Error: The path '" << path << "' is NOT a valid folder.");
        return;
    }

    object_guid const  agent_guid = ctx.folder_content_agent(current_folder_guid);
    if (!ctx.is_valid_agent_guid(agent_guid))
    {
        push_to_history(msgstream() << "There is no AGENT in the folder.");
        return;
    }

    push_to_history(msgstream() << "ID: " << const_cast<simulation_context&>(ctx).from_agent_guid(agent_guid));

    push_to_history("State variables:");
    for (auto const&  name_and_props : ctx.agent_state_variables(agent_guid))
        push_to_history(msgstream() << "   " << name_and_props.first << " = "
                                    << name_and_props.second.get_value()
                                    << " [ " << name_and_props.second.get_min_value() << ", " << name_and_props.second.get_max_value() << " ] "
                                    << name_and_props.second.get_ideal_value()
                                    );

    push_to_history("System variables:");
    for (auto const&  name_and_value : ctx.agent_system_variables(agent_guid))
        push_to_history(msgstream() << "   " << name_and_value.first << " = " << name_and_value.second);

    ai::agent_system_state const&  sys_state = ctx.agent_system_state(agent_guid);
    push_to_history("System state:");
    push_to_history("   Motion object frame:");
    push_to_history(msgstream() << "      origin = " << print_vector3(sys_state.motion_object_frame.origin()));
    push_to_history(msgstream() << "      I = " << print_vector3(sys_state.motion_object_frame.basis_vector_x()));
    push_to_history(msgstream() << "      J = " << print_vector3(sys_state.motion_object_frame.basis_vector_y()));
    push_to_history(msgstream() << "      K = " << print_vector3(sys_state.motion_object_frame.basis_vector_z()));
    push_to_history("   Camera frame:");
    push_to_history(msgstream() << "      origin = " << print_vector3(sys_state.camera_frame.origin()));
    push_to_history(msgstream() << "      I = " << print_vector3(sys_state.camera_frame.basis_vector_x()));
    push_to_history(msgstream() << "      J = " << print_vector3(sys_state.camera_frame.basis_vector_y()));
    push_to_history(msgstream() << "      K = " << print_vector3(sys_state.camera_frame.basis_vector_z()));

    ai::scene_binding_ptr const  binding = ctx.agent_scene_binding(agent_guid);
    push_to_history("Binding folders:");
    push_to_history(msgstream() << "   agent                     : " << ctx.to_relative_path(binding->folder_guid_of_agent, current_folder_guid));
    push_to_history(msgstream() << "   motion object             : " << ctx.to_relative_path(binding->folder_guid_of_motion_object, current_folder_guid));
    push_to_history(msgstream() << "   motion object frame       : " << ctx.to_relative_path(ctx.folder_of_frame(binding->frame_guid_of_motion_object), current_folder_guid));
    push_to_history(msgstream() << "   skeleton                  : " << ctx.to_relative_path(binding->folder_guid_of_skeleton, current_folder_guid));
    push_to_history(msgstream() << "   skeleton frame            : " << ctx.to_relative_path(ctx.folder_of_frame(binding->frame_guid_of_skeleton), current_folder_guid));
    push_to_history(msgstream() << "   skeleton sync             : " << ctx.to_relative_path(binding->folder_guid_of_skeleton_sync, current_folder_guid));
    push_to_history(msgstream() << "   skeleton sync source frame: " << ctx.to_relative_path(ctx.folder_of_frame(binding->frame_guid_of_skeleton_sync_source), current_folder_guid));
    push_to_history(msgstream() << "   skeleton sync target frame: " << ctx.to_relative_path(ctx.folder_of_frame(binding->frame_guid_of_skeleton_sync_target), current_folder_guid));

    push_to_history("Motion templates:");
    for (auto const&  name_and_template : ctx.agent_motion_templates(agent_guid).motions_map())
        push_to_history(msgstream() << "   " << name_and_template.first);

    ai::action_controller const&  action_ctrl = ctx.agent_action_controller(agent_guid);
    push_to_history("Action controller:");
    push_to_history("   Current action:");
    push_to_history(msgstream() << "      name: " << action_ctrl.get_current_action()->get_name());
    push_to_history(msgstream() << "      cyclic: " << action_ctrl.get_current_action()->is_cyclic());
    push_to_history(msgstream() << "      complete: " << action_ctrl.get_current_action()->is_complete());
    push_to_history(msgstream() << "      ghost complete: " << action_ctrl.get_current_action()->is_ghost_complete());
    push_to_history(msgstream() << "      guard valid: " << action_ctrl.get_current_action()->is_guard_valid());
    push_to_history(msgstream() << "      interpolation param: " << action_ctrl.get_current_action()->interpolation_parameter());

    ai::sight_controller const&  sight_ctrl = ctx.agent_sight_controller(agent_guid);
    push_to_history("Sight controller:");
    push_to_history("   Camera config:");
    ai::sight_controller::camera_config const& camera_cfg = sight_ctrl.get_camera_config();
    push_to_history(msgstream() << "      origin z-shift: " << camera_cfg.origin_z_shift);
    push_to_history(msgstream() << "      horizontal fov: " << camera_cfg.horizontal_fov_angle);
    push_to_history(msgstream() << "      vertical fov  : " << camera_cfg.vertical_fov_angle);
    push_to_history(msgstream() << "      near plane    : " << camera_cfg.near_plane);
    push_to_history(msgstream() << "      far plane     : " << camera_cfg.far_plane);
    ai::sight_controller::camera_perspective_ptr const  camera = sight_ctrl.get_camera();
    if (camera != nullptr)
    {
        push_to_history("   Camera:");
        push_to_history(msgstream() << "      near plane: " << camera->near_plane());
        push_to_history(msgstream() << "      far plane: " << camera->far_plane());
        push_to_history(msgstream() << "      left: " << camera->left());
        push_to_history(msgstream() << "      right: " << camera->right());
        push_to_history(msgstream() << "      bottom: " << camera->bottom());
        push_to_history(msgstream() << "      top: " << camera->top());
    }
    else
        push_to_history("   Camera: NONE");
    push_to_history("   Ray cast config:");
    ai::sight_controller::ray_cast_config const&  ray_cast = sight_ctrl.get_ray_cast_config();
    push_to_history(msgstream() << "      do direct ray casts   : " << ray_cast.do_directed_ray_casts);
    push_to_history(msgstream() << "      random rays per second: " << ray_cast.num_random_ray_casts_per_second);
    push_to_history(msgstream() << "      ray seconds in history: " << ray_cast.max_ray_cast_info_life_time_in_seconds);
    push_to_history(msgstream() << "      update depth image    : " << ray_cast.do_update_depth_image);
    push_to_history(msgstream() << "      image cells x         : " << ray_cast.num_cells_along_x_axis);
    push_to_history(msgstream() << "      image cells y         : " << ray_cast.num_cells_along_y_axis);
    push_to_history(msgstream() << "   Num direct ray casts in history: " << sight_ctrl.get_directed_ray_casts_in_time().size());
    push_to_history(msgstream() << "   Num random ray casts in history: " << sight_ctrl.get_random_ray_casts_in_time().size());

    push_to_history("Cortex:");
    if (ai::cortex_robot const * cortex_ptr = dynamic_cast<ai::cortex_robot const *>(&ctx.agent_cortex(agent_guid)))
    {
        push_to_history("   Type: ROBOT");
    }
    else if (ai::cortex_netlab const * cortex_ptr = dynamic_cast<ai::cortex_netlab const *>(&ctx.agent_cortex(agent_guid)))
    {
        push_to_history("   Type: NETLAB");
    }
    else if (ai::cortex_random const * cortex_ptr = dynamic_cast<ai::cortex_random const *>(&ctx.agent_cortex(agent_guid)))
    {
        push_to_history("   Type: RANDOM");
    }
    else if (ai::cortex_default const * cortex_ptr = dynamic_cast<ai::cortex_default const *>(&ctx.agent_cortex(agent_guid)))
    {
        push_to_history("   Type: DEFAULT");
    }
    else if (ai::cortex_mock const * cortex_ptr = dynamic_cast<ai::cortex_mock const *>(&ctx.agent_cortex(agent_guid)))
    {
        push_to_history("   Type: MOCK");
    }
    else
        push_to_history("Error: Unknown cortex type.");
}


void  console::execute_pb(std::vector<std::string> const&  words, simulator const&  sim)
{
    push_to_history(msgstream() << "NOT IMPLEMENTED YET!");
}


void  console::execute_pc(std::vector<std::string> const&  words, simulator const&  sim)
{
    if (words.size() > 2ULL)
    {
        push_to_history("pc: The command accepts zero or one argument (collider name).");
        return;
    }

    simulation_context const&  ctx = *sim.context();
    object_guid const  current_folder_guid = ctx.from_absolute_path(scene_path_to_string());

    std::string  collider_name;
    if (words.size() == 1ULL)
    {
        simulation_context::folder_content_type const&  fct = ctx.folder_content(current_folder_guid);
        auto const  content_it = fct.content_index.find(OBJECT_KIND::COLLIDER);
        if (content_it == fct.content_index.end() || content_it->second.empty())
        {
            push_to_history(msgstream() << "There is no COLLIDER in the folder.");
            return;
        }
        if (content_it->second.size() > 1ULL)
        {
            push_to_history(msgstream() << "There is more than one COLLIDER in the folder.");
            push_to_history(msgstream() << "    => Pass COLLIDER name to 'pc' command.");
            push_to_history(msgstream() << "[Use 'ls' command to see folder content.]");
            return;
        }
        collider_name = *content_it->second.begin();
    }
    else
        collider_name = words.at(1);

    object_guid const  collider_guid = ctx.folder_content_of_name(current_folder_guid, collider_name);
    if (!ctx.is_valid_collider_guid(collider_guid))
    {
        push_to_history(msgstream() << "There is no COLLIDER of the name '" << collider_name << "' in the folder.");
        return;
    }

    push_to_history(msgstream() << "Scene index: " << (natural_32_bit)ctx.collider_scene_index(collider_guid));
    push_to_history(msgstream() << "Material: " << angeo::to_string(ctx.collision_material_of(collider_guid)));
    push_to_history(msgstream() << "Class: " << angeo::to_string(ctx.collision_class_of(collider_guid)));
    push_to_history(msgstream() << "Density multiplier: " << ctx.collider_density_multiplier(collider_guid));
    push_to_history(msgstream() << "Enabled: " << ctx.is_collider_enabled(collider_guid));
    push_to_history(msgstream() << "Type: " << angeo::as_string(ctx.collider_shape_type(collider_guid)));

    switch (ctx.collider_shape_type(collider_guid))
    {
    case angeo::COLLISION_SHAPE_TYPE::BOX:
        push_to_history(msgstream() << "   Half sizes = " << print_vector3(ctx.collider_box_half_sizes_along_axes(collider_guid)));
        break;
    case angeo::COLLISION_SHAPE_TYPE::CAPSULE:
        push_to_history(msgstream() << "   End points distance = " << ctx.collider_capsule_half_distance_between_end_points(collider_guid));
        push_to_history(msgstream() << "   Thickness = " << ctx.collider_capsule_thickness_from_central_line(collider_guid));
        break;
    case angeo::COLLISION_SHAPE_TYPE::SPHERE:
        push_to_history(msgstream() << "   Radius = " << ctx.collider_sphere_radius(collider_guid));
        break;
    case angeo::COLLISION_SHAPE_TYPE::TRIANGLE:
        push_to_history(msgstream() << "   Num triangles = " << ctx.collider_num_coids(collider_guid));
        break;
    default:
        push_to_history("Error: Unknown collider type.");
        break;
    }

    push_to_history(msgstream() << "Frame = " << ctx.to_relative_path(ctx.folder_of_frame(ctx.frame_of_collider(collider_guid)), current_folder_guid));

    if (ctx.owner_of_collider(collider_guid) != invalid_object_guid())
        push_to_history(msgstream() << "Owner = " << ctx.to_relative_path(ctx.folder_of(ctx.owner_of_collider(collider_guid)), current_folder_guid));
    else
        push_to_history("Owner = NONE");

    if (ctx.is_valid_rigid_body_guid(ctx.rigid_body_of_collider(collider_guid)))
        push_to_history(msgstream() << "Rigid body = " << ctx.to_relative_path(ctx.folder_of(ctx.rigid_body_of_collider(collider_guid)), current_folder_guid));
    else
        push_to_history("Rigid body = NONE");
}


void  console::execute_pf(std::vector<std::string> const&  words, simulator const&  sim)
{
    if (words.size() > 2ULL)
    {
        push_to_history("pf: The command accepts at most one argument (folder path).");
        return;
    }

    std::string const  path = words.size() == 1UL ? scene_path_to_string() :
            (words.at(1).front() == '/' ? "" : scene_path_to_string()) +
            words.at(1) +
            (words.at(1).back() == '/' ? "" : "/")
            ;

    simulation_context const&  ctx = *sim.context();
    object_guid const  current_folder_guid = ctx.from_absolute_path(path);
    if (!ctx.is_valid_folder_guid(current_folder_guid))
    {
        push_to_history(msgstream() << "Error: The path '" << path << "' is NOT a valid folder.");
        return;
    }

    object_guid const  frame_guid = ctx.folder_content_frame(current_folder_guid);
    if (!ctx.is_valid_frame_guid(frame_guid))
    {
        push_to_history(msgstream() << "There is no FRAME in the folder.");
        return;
    }

    object_guid const  parent_frame_guid = ctx.parent_frame(frame_guid);

    angeo::coordinate_system const&  local_frame = ctx.frame_coord_system(frame_guid);
    angeo::coordinate_system_explicit const&  local_frame_ex = ctx.frame_explicit_coord_system(frame_guid);
    push_to_history(msgstream() << "Local:");
    push_to_history(msgstream() << "   origin = " << print_vector3(local_frame.origin()));
    push_to_history(msgstream() << "   orientation = " << print_quaternion(local_frame.orientation()));
    push_to_history(msgstream() << "   I = " << print_vector3(local_frame_ex.basis_vector_x()));
    push_to_history(msgstream() << "   J = " << print_vector3(local_frame_ex.basis_vector_y()));
    push_to_history(msgstream() << "   K = " << print_vector3(local_frame_ex.basis_vector_z()));
    if (ctx.is_valid_frame_guid(parent_frame_guid))
    {
        angeo::coordinate_system const&  world_frame = ctx.frame_coord_system_in_world_space(frame_guid);
        angeo::coordinate_system_explicit const&  world_frame_ex = ctx.frame_explicit_coord_system_in_world_space(frame_guid);
        push_to_history(msgstream() << "World:");
        push_to_history(msgstream() << "   origin = " << print_vector3(world_frame.origin()));
        push_to_history(msgstream() << "   orientation = " << print_quaternion(world_frame.orientation()));
        push_to_history(msgstream() << "   I = " << print_vector3(world_frame_ex.basis_vector_x()));
        push_to_history(msgstream() << "   J = " << print_vector3(world_frame_ex.basis_vector_y()));
        push_to_history(msgstream() << "   K = " << print_vector3(world_frame_ex.basis_vector_z()));

        push_to_history(msgstream() << "Parent:");
        push_to_history(msgstream() << "    absolute = " << ctx.to_absolute_path(ctx.folder_of_frame(parent_frame_guid)));
        push_to_history(msgstream() << "    relative = " << ctx.to_relative_path(ctx.folder_of_frame(parent_frame_guid), current_folder_guid));
    }
    else
        push_to_history(msgstream() << "Parent: NONE");
}


void  console::execute_pr(std::vector<std::string> const&  words, simulator const&  sim)
{
    if (words.size() > 2ULL)
    {
        push_to_history("pr: The command accepts at most one argument (folder path).");
        return;
    }

    std::string const  path = words.size() == 1UL ? scene_path_to_string() :
            (words.at(1).front() == '/' ? "" : scene_path_to_string()) +
            words.at(1) +
            (words.at(1).back() == '/' ? "" : "/")
            ;

    simulation_context const&  ctx = *sim.context();
    object_guid const  current_folder_guid = ctx.from_absolute_path(path);
    if (!ctx.is_valid_folder_guid(current_folder_guid))
    {
        push_to_history(msgstream() << "Error: The path '" << path << "' is NOT a valid folder.");
        return;
    }

    push_to_history(msgstream() << "NOT IMPLEMENTED YET!");
}


void  console::execute_ps(std::vector<std::string> const&  words, simulator const&  sim)
{
    push_to_history(msgstream() << "NOT IMPLEMENTED YET!");
}


void  console::execute_pt(std::vector<std::string> const&  words, simulator const&  sim)
{
    push_to_history(msgstream() << "NOT IMPLEMENTED YET!");
}


void  console::execute_tree(std::vector<std::string> const&  words, simulator const&  sim)
{
    if (words.size() > 2ULL)
    {
        push_to_history("tree: The command accepts zero or one argument (path).");
        return;
    }

    simulation_context const&  ctx = *sim.context();

    object_guid  current_folder_guid;
    if (words.size() == 1ULL)
        current_folder_guid = ctx.from_absolute_path(scene_path_to_string());
    else
    {
        std::string const  path =
                (words.at(1).front() == '/' ? "" : scene_path_to_string()) +
                words.at(1) +
                (words.at(1).back() == '/' ? "" : "/")
                ;
        current_folder_guid = ctx.from_absolute_path(path);
        if (!ctx.is_valid_folder_guid(current_folder_guid))
        {
            push_to_history(msgstream() << "Error: The path '" << path << "' is NOT a valid folder.");
            return;
        }
    }

    ctx.for_each_child_folder(current_folder_guid, true, true,
        [this, &ctx, current_folder_guid](object_guid const  folder_guid, simulation_context::folder_content_type const&  fct) -> bool {
            std::stringstream  sstr;
            if (current_folder_guid != folder_guid)
            {
                for (object_guid  guid = ctx.parent_folder(folder_guid); guid != current_folder_guid; guid = ctx.parent_folder(guid))
                    sstr << "|  ";
                push_to_history(msgstream() << sstr.str() << "+ " << fct.folder_name);
            }
            sstr << "|  ";
            for (auto const&  kind_and_names : fct.content_index)
                for (auto const&  name : kind_and_names.second)
                    push_to_history(msgstream() << sstr.str() << '<' << to_string(kind_and_names.first) << "> " << name);
            return true;
        });
}


}
