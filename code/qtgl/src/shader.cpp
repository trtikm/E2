#include <qtgl/shader.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/msgstream.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/functional/hash.hpp>
#include <sstream>
#include <istream>
#include <unordered_set>
#include <vector>
#include <limits>
#include <algorithm>

namespace qtgl { namespace detail {


void  add_macro_definitions_after_version(std::vector<std::string>&  lines,
                                          std::vector<std::string> const&  macro_definitions)
{
    TMPROF_BLOCK();

    std::string const  version_keyword = "#version ";
    natural_64_bit  i = 0ULL;
    for ( ; i < lines.size(); ++i)
        if (boost::starts_with(boost::trim_left_copy(lines.at(i)),version_keyword))
            break;
    if (i == lines.size())
    {
        for (i = 0ULL; i < lines.size(); ++i)
            if (!lines.at(i).empty())
                break;
        INVARIANT(i > 0ULL);
        --i;
        lines.at(i) = "#version 420";
    }
    INVARIANT(i >= macro_definitions.size());
    natural_64_bit  j = i - macro_definitions.size();
    lines.at(j) = lines.at(i);
    for (++j; j <= i; ++j)
        lines.at(j) = macro_definitions.at(macro_definitions.size() - i + j - 1U);
}

bool  is_include_command(std::string const&  line, std::string&  output_include_string)
{
    TMPROF_BLOCK();

    std::string const  keyword = "#include ";
    std::string const  trimed_line = boost::trim_copy(line);
    if (boost::starts_with(trimed_line,keyword))
    {
        output_include_string = boost::trim_copy(line.substr(keyword.size()));
        return true;
    }
    return false;
}

std::string  parse_lines(std::istream&  istr,
                         boost::filesystem::path const&  directory,
                         std::unordered_set<std::string>& visited_files,
                         std::vector<std::string>& output_lines)
{
    TMPROF_BLOCK();

    natural_32_bit  line_number = 1U;
    while (istr.good())
    {
        std::string line;
        std::getline(istr,line);
        std::string  include_string;
        if (is_include_command(line,include_string))
        {
            boost::filesystem::path const include_filename =
                    boost::filesystem::absolute(directory / include_string).normalize();
            if (visited_files.count(include_filename.string()))
                return "Cyclic dependency between included files.";
            visited_files.insert(include_filename.string());
            boost::filesystem::ifstream  include_istr(include_filename,std::ios_base::binary);
            if (!include_istr.good())
                return "Cannot open file: " + include_filename.string();
            std::string const  error_message =
                    parse_lines(include_istr,include_filename.parent_path(),visited_files,output_lines);
            if (!error_message.empty())
                return error_message;
            std::stringstream  sstr;
            sstr << "#line " << line_number << "\n\0";
            output_lines.push_back(sstr.str());
            continue;
        }
        line.push_back('\n');
        line.push_back('\0');
        output_lines.push_back(line);
        ++line_number;
    }
    output_lines.push_back(" \0");
    if (!istr.eof() || istr.bad())
        return "Failure in reading the input stream.";

    return "";
}

std::string  parse_lines(std::istream&  istr,
                         GLenum const  shader_type,
                         boost::filesystem::path const&  directory,
                         std::unordered_set<std::string>&  visited_files,
                         std::vector<std::string>& output_lines)
{
    TMPROF_BLOCK();

    ASSUMPTION(output_lines.empty());
    ASSUMPTION(shader_type == GL_VERTEX_SHADER || shader_type == GL_FRAGMENT_SHADER);

    std::vector<std::string> const  macro_definitions{
            shader_type == GL_VERTEX_SHADER         ? "#define SHADER_TYPE                        0\n\0"  :
            shader_type == GL_FRAGMENT_SHADER       ? "#define SHADER_TYPE                        1\n\0"  :
                                                      "#error \"Unknown shader type.\"\n\0"               ,
            "#define VERTEX_SHADER_TYPE                 0\n\0",
            "#define FRAGMENT_SHADER_TYPE               1\n\0",
            };
    output_lines.resize(macro_definitions.size() + 1U,"\n\0");

    std::string const  error_message = parse_lines(istr,directory,visited_files,output_lines);
    if (!error_message.empty())
        return error_message;

    add_macro_definitions_after_version(output_lines,macro_definitions);

    return error_message;
}

std::string  parse_lines(std::istream&  istr,
                         GLenum const  shader_type,
                         std::vector<std::string>& output_lines)
{
    TMPROF_BLOCK();

    std::unordered_set<std::string>  visited_files;
    return parse_lines(istr,shader_type,
                       boost::filesystem::absolute(boost::filesystem::current_path()).normalize(),
                       visited_files,output_lines);
}

std::string  parse_lines(boost::filesystem::path const&  raw_filename,
                         GLenum const  shader_type,
                         std::vector<std::string>& output_lines)
{
    TMPROF_BLOCK();

    ASSUMPTION(output_lines.empty());

    boost::filesystem::path const  filename =
            boost::filesystem::absolute(raw_filename).normalize();

    boost::filesystem::ifstream  istr(filename,std::ios_base::binary);
    if (!istr.good())
        return "Cannot open file: " + filename.string();

    std::unordered_set<std::string>  visited_files;
    visited_files.insert(filename.string());

    return parse_lines(istr,shader_type,filename.parent_path(),visited_files,output_lines);
}

void  parse_shader_build_error_message(GLuint const  shader, GLenum const  build_status_type,
                                       std::string&  error_message)
{
    TMPROF_BLOCK();

    int  is_build_successfull;
    switch (build_status_type)
    {
    case GL_COMPILE_STATUS: glapi().glGetShaderiv(shader,build_status_type,&is_build_successfull); break;
    case GL_LINK_STATUS: glapi().glGetProgramiv(shader,build_status_type,&is_build_successfull); break;
    default: error_message = "Unknown build type for parsing build success state."; return;
    }
    if (is_build_successfull == GL_TRUE)
        return;
    int  log_length;
    switch (build_status_type)
    {
    case GL_COMPILE_STATUS: glapi().glGetShaderiv(shader,GL_INFO_LOG_LENGTH,&log_length); break;
    case GL_LINK_STATUS: glapi().glGetProgramiv(shader,GL_INFO_LOG_LENGTH,&log_length); break;
    default: error_message = "Unknown build type for parsing length of the error message."; return;
    }
    if (log_length <= 0)
    {
        error_message = "Build log has negative length.";
        return;
    }
    error_message.resize(log_length+1U,0U);
    switch (build_status_type)
    {
    case GL_COMPILE_STATUS: glapi().glGetShaderInfoLog(shader,log_length,&log_length,&error_message.at(0U)); return;
    case GL_LINK_STATUS: glapi().glGetProgramInfoLog(shader,log_length,&log_length,&error_message.at(0U)); return;
    default: error_message = "Unknown build type for parsing length of the error message."; return;
    }
}

GLuint  create_opengl_shader_program(GLenum shader_type,
                                     std::vector<GLchar const*> const&  line_pointers,
                                     std::string&  error_message)
{
    TMPROF_BLOCK();

    if (!error_message.empty())
    {
        error_message = "The error_message string is initially not empty.";
        return 0U;
    }

    GLuint const  shader = glapi().glCreateShader(shader_type);
    {
        if (shader == 0U)
        {
            error_message = "Cannot create opengl shader object.";
            return 0U;
        }
        glapi().glShaderSource(shader,(GLsizei)line_pointers.size(),(GLchar const**)&line_pointers.at(0),nullptr);
        glapi().glCompileShader(shader);
        parse_shader_build_error_message(shader,GL_COMPILE_STATUS,error_message);
        if (!error_message.empty())
            return 0U;
    }

    GLuint const  shader_program = glapi().glCreateProgram();
    {
        if (shader_program == 0U)
        {
            error_message = "Cannot create opengl shader program object.";
            return 0U;
        }
        glapi().glProgramParameteri(shader_program,GL_PROGRAM_SEPARABLE,GL_TRUE);
        glapi().glAttachShader(shader_program,shader);

        glapi().glLinkProgram(shader_program);
        parse_shader_build_error_message(shader_program,GL_LINK_STATUS,error_message);
        if (!error_message.empty())
            return 0U;
        glapi().glDetachShader(shader_program,shader);
    }

    glapi().glDeleteShader(shader);

    return shader_program;
}

vertex_program_ptr  create_vertex_program(std::vector<std::string> const&  lines,
                                          std::string&  error_message)
{
    TMPROF_BLOCK();

    ASSUMPTION(error_message.empty());
    std::vector<GLchar const*>  line_pointers;
    for (std::string const& line : lines)
        if (!line.empty())
            line_pointers.push_back((GLchar const*)&line.at(0));
    ASSUMPTION(!line_pointers.empty());
    GLuint const  id = detail::create_opengl_shader_program(GL_VERTEX_SHADER,line_pointers,error_message);
    if (id == 0 || !error_message.empty())
        return vertex_program_ptr();
    vertex_program_properties_ptr const  props = std::make_shared<vertex_program_properties>(lines);
    return vertex_program::create(id,props);
}

fragment_program_ptr  create_fragment_program(std::vector<std::string> const&  lines,
                                              std::string&  error_message,
                                              boost::filesystem::path const&  shader_file)
{
    TMPROF_BLOCK();

    ASSUMPTION(error_message.empty());
    std::vector<GLchar const*>  line_pointers;
    for (std::string const& line : lines)
        if (!line.empty())
            line_pointers.push_back((GLchar const*)&line.at(0));
    ASSUMPTION(!line_pointers.empty());
    GLuint const  id = detail::create_opengl_shader_program(GL_FRAGMENT_SHADER,line_pointers,error_message);
    if (id == 0 || !error_message.empty())
        return fragment_program_ptr();
    fragment_program_properties_ptr const  props = std::make_shared<fragment_program_properties>(shader_file,lines);
    return fragment_program::create(id,props);
}


struct text_stream
{
    explicit text_stream(std::vector<std::string> const* const  lines);
    bool  eof() const;
    std::string  get_next_token();

    natural_64_bit  line() const noexcept { return m_line; }

private:

    static bool  is_white(natural_8_bit const  value);
    void  skip_whites();
    natural_8_bit  current() const;
    natural_8_bit  next();

    natural_64_bit  m_line;
    natural_64_bit  m_column;
    std::vector<std::string> const*  m_lines;
};

text_stream::text_stream(std::vector<std::string> const* const  lines)
    : m_line(0ULL)
    , m_column(0ULL)
    , m_lines(lines)
{
    TMPROF_BLOCK();

    ASSUMPTION(m_lines != nullptr);
    ASSUMPTION(
        [](std::vector<std::string> const&  lines) -> bool {
            for (auto const& l : lines)
                if (l.empty())
                    return false;
            return true;
            }(*m_lines)
        );
    skip_whites();
}

bool  text_stream::eof() const
{
    return m_line >= m_lines->size();
}

std::string  text_stream::get_next_token()
{
    TMPROF_BLOCK();

    std::string  token;
    while (!eof())
    {
        static  std::unordered_set<natural_8_bit> const  separators = {
            '\0',
            '\r',
            '\r',
            '\t',
            ' ',
            '(',
            ')',
            '[',
            ']',
            '{',
            '}',
            '+',
            '-',
            '=',
            '<',
            '>',
            '!',
            '&',
            '|',
            '^',
            '~',
            '?',
            '\'',
            '\"',
            '.',
            ':',
            ',',
            ';',
            '\\',
        };

        if (separators.count(current()) != 0ULL)
        {
            next();
            if (!token.empty())
                break;
        }
        else if (current() == '\n' || current() == '#')
        {
            if (token.empty())
                token.push_back(next());
            break;
        }
        else if (current() == '/')
        {
            if (token.empty())
            {
                next();
                if (!eof() && (current() == '/' || current() == '*'))
                {
                    token.push_back('/');
                    token.push_back(next());
                    break;
                }
            }
            else
            {
                if (token == "/" || token == "*")
                    token.push_back(next());
                break;
            }
        }
        else if (current() == '*')
        {
            if (token.empty())
            {
                next();
                if (!eof() && current() == '/')
                {
                    token.push_back('*');
                    token.push_back(next());
                    break;
                }
            }
            else
            {
                if (token == "/")
                    token.push_back(next());
                break;
            }
        }
        else
            token.push_back(next());
    }

    skip_whites();

    return token;
}

bool  text_stream::is_white(natural_8_bit const  value)
{
    return value == ' ' || value == '\t' || value == '\r' || value == '\0';
}

void  text_stream::skip_whites()
{
    while (!eof() && is_white(current()))
        next();
}

natural_8_bit  text_stream::current() const
{
    INVARIANT(m_line < m_lines->size());
    INVARIANT(m_column < m_lines->at(m_line).size());
    return m_lines->at(m_line).at(m_column);
}

natural_8_bit  text_stream::next()
{
    INVARIANT(m_line < m_lines->size());
    INVARIANT(m_column < m_lines->at(m_line).size());
    natural_8_bit const  result = m_lines->at(m_line).at(m_column);
    ++m_column;
    if (m_column == m_lines->at(m_line).size())
    {
        m_column = 0ULL;
        ++m_line;
    }
    return result;
}

void  get_tokens_in_shader_code(
        std::vector<std::string> const&  code_lines,
        std::unordered_set<std::string>&  output,
        std::function<bool(std::string const&)> const&  token_filter =
                [](std::string const&  token) -> bool {
                    ASSUMPTION(!token.empty());
                    if (!(token.front() == '_' || (token.front() >= 'A' && token.front() <= 'Z')))
                        return true;
                    for (auto c : token)
                        if (!(c == '_' || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')))
                            return true;
                    return false;
                }
                )
{
    TMPROF_BLOCK();

    text_stream  text(&code_lines);
    bool  is_in_preprocessor_line = false;
    bool  is_in_line_comment = false;
    bool  is_in_multiline_comment = false;
    while (true)
    {
        INVARIANT(!is_in_line_comment || !is_in_multiline_comment);

        if (text.line() >= 87)
        {
            int iii = 0;
        }

        std::string const  token = text.get_next_token();
        if (token.empty())
            return;

        if (token == "\n")
        {
            if (!is_in_multiline_comment)
            {
                is_in_line_comment = false;
                is_in_preprocessor_line = false;
            }
        }
        else if (token == "#")
        {
            if (!is_in_multiline_comment && !is_in_line_comment)
                is_in_preprocessor_line = true;
        }
        else if (token == "//")
        {
            if (!is_in_multiline_comment)
                is_in_line_comment = true;
        }
        else if (token == "/*")
        {
            if (!is_in_line_comment)
                is_in_multiline_comment = true;
        }
        else if (token == "*/")
        {
            if (!is_in_line_comment)
                is_in_multiline_comment = false;
        }
        else if (!is_in_line_comment && !is_in_multiline_comment && !is_in_preprocessor_line && !token_filter(token))
            output.insert(token);
    }
}


}}

namespace qtgl {


vertex_program_properties_ptr  vertex_program_properties::create(
        std::unordered_set<vertex_shader_input_buffer_binding_location> const&  input_buffer_bindings,
        std::unordered_set<vertex_shader_output_buffer_binding_location> const&  output_buffer_bindings,
        std::unordered_set<vertex_shader_uniform_symbolic_name> const&  symbolic_names_of_used_uniforms
        )
{
    return std::make_shared<vertex_program_properties>(input_buffer_bindings,
                                                       output_buffer_bindings,
                                                       symbolic_names_of_used_uniforms);
}

vertex_program_properties::vertex_program_properties(
        std::unordered_set<vertex_shader_input_buffer_binding_location> const&  input_buffer_bindings,
        std::unordered_set<vertex_shader_output_buffer_binding_location> const&  output_buffer_bindings,
        std::unordered_set<vertex_shader_uniform_symbolic_name> const&  symbolic_names_of_used_uniforms
        )
    : m_input_buffer_bindings(input_buffer_bindings)
    , m_output_buffer_bindings(output_buffer_bindings)
    , m_symbolic_names_of_used_uniforms(symbolic_names_of_used_uniforms)
{
    ASSUMPTION(m_input_buffer_bindings.count(vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION) != 0U);
    ASSUMPTION(m_output_buffer_bindings.count(vertex_shader_output_buffer_binding_location::BINDING_OUT_POSITION) != 0U);
}

vertex_program_properties::vertex_program_properties(std::vector<std::string> const&  lines_of_shader_code)
{
    TMPROF_BLOCK();

    std::unordered_set<std::string>  tokens;
    detail::get_tokens_in_shader_code(lines_of_shader_code,tokens);
    for (auto const  location : std::vector<vertex_shader_input_buffer_binding_location>{
            vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION ,
            vertex_shader_input_buffer_binding_location::BINDING_IN_COLOUR   ,
            vertex_shader_input_buffer_binding_location::BINDING_IN_NORMAL   ,
            vertex_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD0,
            vertex_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD1,
            vertex_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD2,
            vertex_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD3,
            vertex_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD4,
            vertex_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD5,
            vertex_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD6,
            vertex_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD7,
            vertex_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD8,
            vertex_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD9,
            })
        if (tokens.count(binding_location_name(location)) != 0ULL)
            m_input_buffer_bindings.insert(location);

    ASSUMPTION(m_input_buffer_bindings.count(vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION) != 0U);

    for (auto const  location : std::vector<vertex_shader_output_buffer_binding_location>{
            vertex_shader_output_buffer_binding_location::BINDING_OUT_POSITION ,
            vertex_shader_output_buffer_binding_location::BINDING_OUT_COLOUR   ,
            vertex_shader_output_buffer_binding_location::BINDING_OUT_NORMAL   ,
            vertex_shader_output_buffer_binding_location::BINDING_OUT_TEXCOORD0,
            vertex_shader_output_buffer_binding_location::BINDING_OUT_TEXCOORD1,
            vertex_shader_output_buffer_binding_location::BINDING_OUT_TEXCOORD2,
            vertex_shader_output_buffer_binding_location::BINDING_OUT_TEXCOORD3,
            vertex_shader_output_buffer_binding_location::BINDING_OUT_TEXCOORD4,
            vertex_shader_output_buffer_binding_location::BINDING_OUT_TEXCOORD5,
            vertex_shader_output_buffer_binding_location::BINDING_OUT_TEXCOORD6,
            vertex_shader_output_buffer_binding_location::BINDING_OUT_TEXCOORD7,
            vertex_shader_output_buffer_binding_location::BINDING_OUT_TEXCOORD8,
            vertex_shader_output_buffer_binding_location::BINDING_OUT_TEXCOORD9,
            })
        if (tokens.count(binding_location_name(location)) != 0ULL)
            m_output_buffer_bindings.insert(location);

    ASSUMPTION(m_output_buffer_bindings.count(vertex_shader_output_buffer_binding_location::BINDING_OUT_POSITION) != 0U);

    for (auto const  symbolic_name : std::vector<vertex_shader_uniform_symbolic_name>{
            vertex_shader_uniform_symbolic_name::COLOUR_ALPHA               ,
            vertex_shader_uniform_symbolic_name::TRANSFORM_MATRIX_TRANSPOSED,
            })
    {
        if (tokens.count(uniform_name(symbolic_name)) != 0ULL || tokens.count(uniform_symbolic_name(symbolic_name)) != 0ULL)
            m_symbolic_names_of_used_uniforms.insert(symbolic_name);
    }
}

bool  operator==(vertex_program_properties const&  props0, vertex_program_properties const&  props1)
{
    return props0.input_buffer_bindings() == props1.input_buffer_bindings() &&
           props0.output_buffer_bindings() == props1.output_buffer_bindings() &&
           props0.symbolic_names_of_used_uniforms() == props1.symbolic_names_of_used_uniforms()
           ;
}

size_t  hasher_of_vertex_program_properties(vertex_program_properties const&  props)
{
    std::size_t seed = 0ULL;
    for (auto const  location : props.input_buffer_bindings())
        boost::hash_combine(seed,static_cast<natural_8_bit>(location));
    for (auto const  location : props.output_buffer_bindings())
        boost::hash_combine(seed,static_cast<natural_8_bit>(location));
    for (auto const  name : props.symbolic_names_of_used_uniforms())
        boost::hash_combine(seed,static_cast<natural_8_bit>(name));
    return seed;
}


}

namespace qtgl {


vertex_program_ptr  vertex_program::create(std::istream&  source_code, std::string&  error_message)
{
    TMPROF_BLOCK();

    ASSUMPTION(error_message.empty());
    std::vector<std::string>  lines;
    error_message = detail::parse_lines(source_code,GL_VERTEX_SHADER,lines);
    if (!error_message.empty())
        return vertex_program_ptr{};
    return detail::create_vertex_program(lines,error_message);
}

vertex_program_ptr  vertex_program::create(boost::filesystem::path const&  shader_source_file, std::string&  error_message)
{
    TMPROF_BLOCK();

    ASSUMPTION(error_message.empty());
    std::vector<std::string>  lines;
    error_message = detail::parse_lines(shader_source_file,GL_VERTEX_SHADER,lines);
    if (!error_message.empty())
        return vertex_program_ptr{};
    return detail::create_vertex_program(lines,error_message);
}

vertex_program_ptr  vertex_program::create(std::vector<std::string> const& source_code_lines, std::string&  error_message)
{
    return detail::create_vertex_program(source_code_lines,error_message);
}

vertex_program_ptr  vertex_program::create(GLuint const  id, vertex_program_properties_ptr const  properties)
{
    TMPROF_BLOCK();

    return vertex_program_ptr{new vertex_program(id,properties)};
}


vertex_program::~vertex_program()
{
    glapi().glDeleteProgram(id());
}


std::string  load_vertex_shader_file(boost::filesystem::path const&  filename,
                                     std::vector<std::string>& output_lines)
{
    return detail::parse_lines(filename,GL_VERTEX_SHADER,output_lines);
}


}

namespace qtgl {


fragment_program_properties_ptr  fragment_program_properties::create(
        boost::filesystem::path const&  shader_file,
        std::unordered_set<fragment_shader_input_buffer_binding_location> const&  input_buffer_bindings,
        std::unordered_set<fragment_shader_output_buffer_binding_location> const&  output_buffer_bindings,
        std::unordered_set<fragment_shader_texture_sampler_binding> const&  texture_sampler_bindings
        )
{
    return std::make_shared<fragment_program_properties>(shader_file,
                                                         input_buffer_bindings,
                                                         output_buffer_bindings,
                                                         texture_sampler_bindings);
}

fragment_program_properties::fragment_program_properties(
        boost::filesystem::path const&  shader_file,
        std::unordered_set<fragment_shader_input_buffer_binding_location> const&  input_buffer_bindings,
        std::unordered_set<fragment_shader_output_buffer_binding_location> const&  output_buffer_bindings,
        std::unordered_set<fragment_shader_texture_sampler_binding> const&  texture_sampler_bindings
        )
    : m_shader_file(shader_file)
    , m_input_buffer_bindings(input_buffer_bindings)
    , m_output_buffer_bindings(output_buffer_bindings)
    , m_texture_sampler_bindings(texture_sampler_bindings)
{
    ASSUMPTION(m_input_buffer_bindings.count(fragment_shader_input_buffer_binding_location::BINDING_IN_COLOUR) != 0U);
    ASSUMPTION(m_output_buffer_bindings.count(fragment_shader_output_buffer_binding_location::BINDING_OUT_COLOUR) != 0U);
}

fragment_program_properties::fragment_program_properties(
        boost::filesystem::path const&  shader_file,
        std::vector<std::string> const&  lines_of_shader_code
        )
    : m_shader_file(shader_file)
{
    TMPROF_BLOCK();

    std::unordered_set<std::string>  tokens;
    detail::get_tokens_in_shader_code(lines_of_shader_code,tokens);
    for (auto const  location : std::vector<fragment_shader_input_buffer_binding_location>{
            fragment_shader_input_buffer_binding_location::BINDING_IN_POSITION ,
            fragment_shader_input_buffer_binding_location::BINDING_IN_COLOUR   ,
            fragment_shader_input_buffer_binding_location::BINDING_IN_NORMAL   ,
            fragment_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD0,
            fragment_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD1,
            fragment_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD2,
            fragment_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD3,
            fragment_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD4,
            fragment_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD5,
            fragment_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD6,
            fragment_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD7,
            fragment_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD8,
            fragment_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD9,
            })
        if (tokens.count(binding_location_name(location)) != 0ULL)
            m_input_buffer_bindings.insert(location);

    for (auto const  location : std::vector<fragment_shader_output_buffer_binding_location>{
            fragment_shader_output_buffer_binding_location::BINDING_OUT_COLOUR,
            })
        if (tokens.count(binding_location_name(location)) != 0ULL)
            m_output_buffer_bindings.insert(location);

    ASSUMPTION(m_output_buffer_bindings.count(fragment_shader_output_buffer_binding_location::BINDING_OUT_COLOUR) != 0U);

    for (auto const  sampler_binding : std::vector<fragment_shader_texture_sampler_binding>{
            fragment_shader_texture_sampler_binding::BINDING_TEXTURE_DIFFUSE,
            })
    {
        if (tokens.count(sampler_binding_name(sampler_binding)) != 0ULL)
            m_texture_sampler_bindings.insert(sampler_binding);
    }
}

bool  operator==(fragment_program_properties const&  props0, fragment_program_properties const&  props1)
{
    return props0.shader_file() == props1.shader_file() &&
           props0.input_buffer_bindings() == props1.input_buffer_bindings() &&
           props0.output_buffer_bindings() == props1.output_buffer_bindings() &&
           props0.texture_sampler_bindings() == props1.texture_sampler_bindings()
           ;
}

size_t  hasher_of_fragment_program_properties(fragment_program_properties const&  props)
{
    std::size_t seed = 0ULL;
    boost::hash_combine(seed,props.shader_file().string());
    for (auto const  location : props.input_buffer_bindings())
        boost::hash_combine(seed,static_cast<natural_8_bit>(location));
    for (auto const  location : props.output_buffer_bindings())
        boost::hash_combine(seed,static_cast<natural_8_bit>(location));
    for (auto const  sampler_binding : props.texture_sampler_bindings())
        boost::hash_combine(seed,static_cast<natural_8_bit>(sampler_binding));
    return seed;
}


}

namespace qtgl {


fragment_program_ptr  fragment_program::create(std::istream&  source_code, std::string&  error_message)
{
    TMPROF_BLOCK();

    ASSUMPTION(error_message.empty());
    std::vector<std::string>  lines;
    error_message = detail::parse_lines(source_code,GL_FRAGMENT_SHADER,lines);
    if (!error_message.empty())
        return fragment_program_ptr{};
    return detail::create_fragment_program(lines,error_message,"/fragment-program-file-was-not-specified");
}

fragment_program_ptr  fragment_program::create(boost::filesystem::path const&  shader_source_file,
                                               std::string&  error_message)
{
    TMPROF_BLOCK();

    ASSUMPTION(error_message.empty());
    std::vector<std::string>  lines;
    error_message = detail::parse_lines(shader_source_file,GL_FRAGMENT_SHADER,lines);
    if (!error_message.empty())
        return fragment_program_ptr{};
    return detail::create_fragment_program(lines,error_message,shader_source_file);
}

fragment_program_ptr  fragment_program::create(GLuint const  id, fragment_program_properties_ptr const  properties)
{
    TMPROF_BLOCK();

    return fragment_program_ptr{new fragment_program(id,properties)};
}

fragment_program::~fragment_program()
{
    glapi().glDeleteProgram(id());
}


}

namespace qtgl {


void  set_uniform_variable(vertex_program_ptr const  shader_program,
                           std::string const&  variable_name,
                           float_32_bit const  value_to_store)
{
    TMPROF_BLOCK();

    GLint const  layout_location = glapi().glGetUniformLocation(shader_program->id(),variable_name.c_str());
    ASSUMPTION(layout_location != -1);
    glapi().glProgramUniform1f(shader_program->id(),layout_location,value_to_store);
}

void  set_uniform_variable(vertex_program_ptr const  shader_program,
                           std::string const&  variable_name,
                           matrix44 const&  value_to_store)
{
    TMPROF_BLOCK();

    GLint const  layout_location = glapi().glGetUniformLocation(shader_program->id(),variable_name.c_str());
    ASSUMPTION(layout_location != -1);
    glapi().glProgramUniformMatrix4fv(shader_program->id(),layout_location,1U,GL_TRUE,value_to_store.data());
}


}

namespace qtgl {


shaders_binding_ptr  shaders_binding::create(vertex_program_ptr const  vertex_program,
                                             fragment_program_ptr const  fragment_program)
{
    TMPROF_BLOCK();

    return shaders_binding_ptr( new shaders_binding(vertex_program,fragment_program) );
}

shaders_binding::shaders_binding(vertex_program_ptr const  vertex_program,
                                 fragment_program_ptr const  fragment_program)
    : m_id(0U)
{
    glapi().glGenProgramPipelines(1U,&m_id);
    ASSUMPTION(id() != 0U);
    glapi().glUseProgramStages(id(),GL_VERTEX_SHADER_BIT,vertex_program->id());
    glapi().glUseProgramStages(id(),GL_FRAGMENT_SHADER_BIT,fragment_program->id());

}

shaders_binding::~shaders_binding()
{
    qtgl::glapi().glDeleteProgramPipelines(1,&m_id);
}


void make_current(shaders_binding_ptr const  shaders_binding)
{
    TMPROF_BLOCK();

    glapi().glBindProgramPipeline(shaders_binding->id());
}


}
