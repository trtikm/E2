#include <qtgl/shader.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/msgstream.hpp>
#include <utility/canonical_path.hpp>
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

namespace qtgl { namespace detail { namespace {


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
            if (!boost::filesystem::exists(directory / include_string))
                return "The include file does not exists: " + (directory / include_string).string();
            boost::filesystem::path const include_filename =
                    canonical_path(directory / include_string);
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
                       canonical_path(boost::filesystem::current_path()),
                       visited_files,output_lines);
}

std::string  parse_lines(boost::filesystem::path const&  raw_filename,
                         GLenum const  shader_type,
                         std::vector<std::string>& output_lines)
{
    TMPROF_BLOCK();

    ASSUMPTION(output_lines.empty());

    if (!boost::filesystem::exists(raw_filename))
        return "Shader file does not exist: " + raw_filename.string();

    boost::filesystem::path const  filename =
            canonical_path(raw_filename);

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
        {
            LOG(error,error_message);
            return 0U;
        }
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

//vertex_program_ptr  create_vertex_program(std::vector<std::string> const&  lines,
//                                          vertex_program_properties_ptr const  properties,
//                                          std::string&  error_message)
//{
//    TMPROF_BLOCK();
//
//    ASSUMPTION(error_message.empty());
//    std::vector<GLchar const*>  line_pointers;
//    for (std::string const& line : lines)
//        if (!line.empty())
//            line_pointers.push_back((GLchar const*)&line.at(0));
//    ASSUMPTION(!line_pointers.empty());
//    GLuint const  id = detail::create_opengl_shader_program(GL_VERTEX_SHADER,line_pointers,error_message);
//    if (id == 0 || !error_message.empty())
//        return vertex_program_ptr();
//    return vertex_program::create(id,properties.operator bool() ? properties : std::make_shared<vertex_program_properties>(lines));
//}
//
//fragment_program_ptr  create_fragment_program(std::vector<std::string> const&  lines,
//                                              fragment_program_properties_ptr const  properties,
//                                              std::string&  error_message)
//{
//    TMPROF_BLOCK();
//
//    ASSUMPTION(error_message.empty());
//    std::vector<GLchar const*>  line_pointers;
//    for (std::string const& line : lines)
//        if (!line.empty())
//            line_pointers.push_back((GLchar const*)&line.at(0));
//    ASSUMPTION(!line_pointers.empty());
//    GLuint const  id = detail::create_opengl_shader_program(GL_FRAGMENT_SHADER,line_pointers,error_message);
//    if (id == 0 || !error_message.empty())
//        return fragment_program_ptr();
//    return fragment_program::create(id,properties.operator bool() ? properties : std::make_shared<fragment_program_properties>(lines));
//}


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


void  parse_properties_from_vertex_shader_code(
        std::vector<std::string> const&  lines_of_shader_code,
        std::unordered_set<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION>&  input_buffer_bindings,
        std::unordered_set<VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION>&  output_buffer_bindings,
        std::unordered_set<VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME>&  symbolic_names_of_used_uniforms
        )
{
    TMPROF_BLOCK();

    std::unordered_set<std::string>  tokens;
    detail::get_tokens_in_shader_code(lines_of_shader_code,tokens);
    for (auto const  location : std::vector<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION>{
            VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_POSITION ,
            VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_DIFFUSE  ,
            VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_SPECULAR ,
            VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_NORMAL   ,
            VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD0,
            VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD1,
            VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD2,
            VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD3,
            VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD4,
            VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD5,
            VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD6,
            VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD7,
            VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD8,
            VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD9,
            })
        if (tokens.count(name(location)) != 0ULL)
        {
            ASSUMPTION(value(location) <= (natural_32_bit)GL_MAX_VERTEX_ATTRIBS);
            input_buffer_bindings.insert(location);
        }

    ASSUMPTION(input_buffer_bindings.count(VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_POSITION) != 0U);

    for (auto const  location : std::vector<VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION>{
            VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION::BINDING_OUT_POSITION ,
            VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION::BINDING_OUT_DIFFUSE  ,
            VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION::BINDING_OUT_SPECULAR ,
            VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION::BINDING_OUT_NORMAL   ,
            VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION::BINDING_OUT_TEXCOORD0,
            VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION::BINDING_OUT_TEXCOORD1,
            VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION::BINDING_OUT_TEXCOORD2,
            VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION::BINDING_OUT_TEXCOORD3,
            VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION::BINDING_OUT_TEXCOORD4,
            VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION::BINDING_OUT_TEXCOORD5,
            VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION::BINDING_OUT_TEXCOORD6,
            VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION::BINDING_OUT_TEXCOORD7,
            VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION::BINDING_OUT_TEXCOORD8,
            VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION::BINDING_OUT_TEXCOORD9,
            })
        if (tokens.count(name(location)) != 0ULL)
            output_buffer_bindings.insert(location);

    //ASSUMPTION(output_buffer_bindings.count(VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION::BINDING_OUT_POSITION) != 0U);

    for (auto const  symbolic_name : std::vector<VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME>{
            VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::COLOUR_ALPHA               ,
            VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::DIFFUSE_COLOUR             ,
            VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::TRANSFORM_MATRIX_TRANSPOSED,
            VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::NUM_MATRICES_PER_VERTEX    ,
            })
    {
        if (tokens.count(uniform_name(symbolic_name)) != 0ULL || tokens.count(name(symbolic_name)) != 0ULL)
            symbolic_names_of_used_uniforms.insert(symbolic_name);
    }
}


void  parse_properties_from_fragment_shader_code(
        std::vector<std::string> const&  lines_of_shader_code,
        std::unordered_set<FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION>&  input_buffer_bindings,
        std::unordered_set<FRAGMENT_SHADER_OUTPUT_BINDING_LOCATION>&  output_buffer_bindings,
        std::unordered_set<FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME>&  texture_sampler_bindings
        )
{
    TMPROF_BLOCK();

    std::unordered_set<std::string>  tokens;
    detail::get_tokens_in_shader_code(lines_of_shader_code,tokens);
    for (auto const  location : std::vector<FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION>{
            FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_POSITION ,
            FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_DIFFUSE  ,
            FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_SPECULAR ,
            FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_NORMAL   ,
            FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD0,
            FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD1,
            FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD2,
            FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD3,
            FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD4,
            FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD5,
            FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD6,
            FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD7,
            FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD8,
            FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD9,
            })
        if (tokens.count(name(location)) != 0ULL)
            input_buffer_bindings.insert(location);

    for (auto const  location : std::vector<FRAGMENT_SHADER_OUTPUT_BINDING_LOCATION>{
            FRAGMENT_SHADER_OUTPUT_BINDING_LOCATION::BINDING_OUT_COLOUR,
            })
        if (tokens.count(name(location)) != 0ULL)
            output_buffer_bindings.insert(location);

    ASSUMPTION(output_buffer_bindings.count(FRAGMENT_SHADER_OUTPUT_BINDING_LOCATION::BINDING_OUT_COLOUR) != 0U);

    for (auto const  sampler_binding : std::vector<FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME>{
            FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::TEXTURE_SAMPLER_DIFFUSE,
            })
    {
        if (tokens.count(uniform_name(sampler_binding)) != 0ULL)
            texture_sampler_bindings.insert(sampler_binding);
    }
}


}}}

namespace qtgl { namespace detail {


vertex_shader_data::vertex_shader_data(boost::filesystem::path const&  path, async::finalise_load_on_destroy_ptr)
{
    TMPROF_BLOCK();

    std::vector<std::string>  lines_of_shader_code;
    std::string const  error_message = detail::parse_lines(path,GL_VERTEX_SHADER,lines_of_shader_code);
    if (!error_message.empty())
        throw std::runtime_error(error_message);
    initialise(lines_of_shader_code);
}


vertex_shader_data::~vertex_shader_data()
{
    TMPROF_BLOCK();

    destroy_gl_shader();
}


bool  vertex_shader_data::set_uniform_variable(std::string const&  variable_name, natural_32_bit const  value_to_store)
{
    TMPROF_BLOCK();

    GLint const  layout_location = glapi().glGetUniformLocation(id(),variable_name.c_str());
    if (layout_location == -1)
    {
        ASSUMPTION(get_symbolic_names_of_used_uniforms().count(to_symbolic_uniform_name_of_vertex_shader(variable_name)) == 1UL);
        return false;
    }
    glapi().glProgramUniform1ui(id(),layout_location,value_to_store);
    return true;
}


bool  vertex_shader_data::set_uniform_variable(std::string const&  variable_name, float_32_bit const  value_to_store)
{
    TMPROF_BLOCK();

    GLint const  layout_location = glapi().glGetUniformLocation(id(),variable_name.c_str());
    if (layout_location == -1)
    {
        ASSUMPTION(get_symbolic_names_of_used_uniforms().count(to_symbolic_uniform_name_of_vertex_shader(variable_name)) == 1UL);
        return false;
    }
    glapi().glProgramUniform1f(id(),layout_location,value_to_store);
    return true;
}

bool  vertex_shader_data::set_uniform_variable(std::string const&  variable_name, vector4 const&  value_to_store)
{
    TMPROF_BLOCK();

    GLint const  layout_location = glapi().glGetUniformLocation(id(), variable_name.c_str());
    if (layout_location == -1)
    {
        ASSUMPTION(get_symbolic_names_of_used_uniforms().count(to_symbolic_uniform_name_of_vertex_shader(variable_name)) == 1UL);
        return false;
    }
    glapi().glProgramUniform4fv(id(), layout_location, 1U, value_to_store.data());
    return true;
}

bool  vertex_shader_data::set_uniform_variable(std::string const&  variable_name, matrix44 const&  value_to_store)
{
    TMPROF_BLOCK();

    static_assert(sizeof(matrix44)==4*4*sizeof(float_32_bit),"");

    GLint const  layout_location = glapi().glGetUniformLocation(id(),variable_name.c_str());
    if (layout_location == -1)
    {
        ASSUMPTION(get_symbolic_names_of_used_uniforms().count(to_symbolic_uniform_name_of_vertex_shader(variable_name)) == 1UL);
        return false;
    }
    glapi().glProgramUniformMatrix4fv(id(),layout_location,1U,GL_TRUE,value_to_store.data());
    return true;
}


bool  vertex_shader_data::set_uniform_variable(std::string const&  variable_name, std::vector<matrix44> const&  value_to_store)
{
    TMPROF_BLOCK();

    static_assert(sizeof(matrix44)==4*4*sizeof(float_32_bit),"");
    ASSUMPTION(value_to_store.size() <= uniform_max_transform_matrices());

    GLint const  layout_location = glapi().glGetUniformLocation(id(),variable_name.c_str());
    if (layout_location == -1)
    {
        ASSUMPTION(get_symbolic_names_of_used_uniforms().count(to_symbolic_uniform_name_of_vertex_shader(variable_name)) == 1UL);
        return false;
    }
    glapi().glProgramUniformMatrix4fv(id(),layout_location,(GLsizei)value_to_store.size(),GL_TRUE,value_to_store.data()->data());
    return true;
}


std::string  vertex_shader_data::create_gl_shader()
{
    if (id() != 0U)
        return "";

    TMPROF_BLOCK();

    std::vector<GLchar const*>  line_pointers;
    for (std::string const& line : get_lines_of_shader_code())
        line_pointers.push_back((GLchar const*)line.data());
    ASSUMPTION(!line_pointers.empty());
    std::string  error_message;
    m_id = detail::create_opengl_shader_program(GL_VERTEX_SHADER,line_pointers,error_message);
    INVARIANT(glapi().glGetError() == 0U);
    INVARIANT((id() != 0U && error_message.empty()) || (id() == 0U && !error_message.empty()));
    return error_message;
}


void  vertex_shader_data::destroy_gl_shader()
{
    if (id() == 0U)
        return;

    TMPROF_BLOCK();

    glapi().glDeleteProgram(id());
    INVARIANT(glapi().glGetError() == 0U);
}


void  vertex_shader_data::initialise(std::vector<std::string> const&  lines_of_shader_code)
{
    TMPROF_BLOCK();

    std::unordered_set<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION>  input_buffer_bindings;
    std::unordered_set<VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION>  output_buffer_bindings;
    std::unordered_set<VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME>  symbolic_names_of_used_uniforms;
    detail::parse_properties_from_vertex_shader_code(
            lines_of_shader_code,
            input_buffer_bindings,
            output_buffer_bindings,
            symbolic_names_of_used_uniforms
            );
    initialise(
            0U,
            input_buffer_bindings,
            output_buffer_bindings,
            symbolic_names_of_used_uniforms,
            lines_of_shader_code
            );
}


void  vertex_shader_data::initialise(
        GLuint const  id, 
        std::unordered_set<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION> const&  input_buffer_bindings,
        std::unordered_set<VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION> const&  output_buffer_bindings,
        std::unordered_set<VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME> const&  symbolic_names_of_used_uniforms,
        std::vector<std::string> const&  lines_of_shader_code
        )
{
    TMPROF_BLOCK();

    m_id = id;
    m_input_buffer_bindings = input_buffer_bindings;
    m_output_buffer_bindings = output_buffer_bindings;
    m_symbolic_names_of_used_uniforms = symbolic_names_of_used_uniforms;
    m_lines_of_shader_code = lines_of_shader_code;
}


}}

namespace qtgl { namespace detail {


fragment_shader_data::fragment_shader_data(boost::filesystem::path const&  path, async::finalise_load_on_destroy_ptr)
{
    TMPROF_BLOCK();

    std::vector<std::string>  lines_of_shader_code;
    std::string const  error_message = detail::parse_lines(path,GL_FRAGMENT_SHADER,lines_of_shader_code);
    if (!error_message.empty())
        throw std::runtime_error(error_message);
    initialise(lines_of_shader_code);
}


fragment_shader_data::~fragment_shader_data()
{
    TMPROF_BLOCK();

    destroy_gl_shader();
}


bool  fragment_shader_data::set_uniform_variable(std::string const&  variable_name, natural_32_bit const  value_to_store)
{
    TMPROF_BLOCK();

    GLint const  layout_location = glapi().glGetUniformLocation(id(),variable_name.c_str());
    if (layout_location == -1)
    {
        ASSUMPTION(get_symbolic_names_of_used_uniforms().count(to_symbolic_uniform_name_of_fragment_shader(variable_name)) == 1UL);
        return false;
    }
    glapi().glProgramUniform1ui(id(),layout_location,value_to_store);
    return true;
}


bool  fragment_shader_data::set_uniform_variable(std::string const&  variable_name, float_32_bit const  value_to_store)
{
    TMPROF_BLOCK();

    GLint const  layout_location = glapi().glGetUniformLocation(id(),variable_name.c_str());
    if (layout_location == -1)
    {
        ASSUMPTION(get_symbolic_names_of_used_uniforms().count(to_symbolic_uniform_name_of_fragment_shader(variable_name)) == 1UL);
        return false;
    }
    glapi().glProgramUniform1f(id(),layout_location,value_to_store);
    return true;
}

bool  fragment_shader_data::set_uniform_variable(std::string const&  variable_name, vector4 const&  value_to_store)
{
    TMPROF_BLOCK();

    GLint const  layout_location = glapi().glGetUniformLocation(id(), variable_name.c_str());
    if (layout_location == -1)
    {
        ASSUMPTION(get_symbolic_names_of_used_uniforms().count(to_symbolic_uniform_name_of_fragment_shader(variable_name)) == 1UL);
        return false;
    }
    glapi().glProgramUniform4fv(id(), layout_location, 1U, value_to_store.data());
    return true;
}

bool  fragment_shader_data::set_uniform_variable(std::string const&  variable_name, matrix44 const&  value_to_store)
{
    TMPROF_BLOCK();

    static_assert(sizeof(matrix44)==4*4*sizeof(float_32_bit),"");

    GLint const  layout_location = glapi().glGetUniformLocation(id(),variable_name.c_str());
    if (layout_location == -1)
    {
        ASSUMPTION(get_symbolic_names_of_used_uniforms().count(to_symbolic_uniform_name_of_fragment_shader(variable_name)) == 1UL);
        return false;
    }
    glapi().glProgramUniformMatrix4fv(id(),layout_location,1U,GL_TRUE,value_to_store.data());
    return true;
}


bool  fragment_shader_data::set_uniform_variable(std::string const&  variable_name, std::vector<matrix44> const&  value_to_store)
{
    TMPROF_BLOCK();

    static_assert(sizeof(matrix44)==4*4*sizeof(float_32_bit),"");
    ASSUMPTION(value_to_store.size() <= uniform_max_transform_matrices());

    GLint const  layout_location = glapi().glGetUniformLocation(id(),variable_name.c_str());
    if (layout_location == -1)
    {
        ASSUMPTION(get_symbolic_names_of_used_uniforms().count(to_symbolic_uniform_name_of_fragment_shader(variable_name)) == 1UL);
        return false;
    }
    glapi().glProgramUniformMatrix4fv(id(),layout_location,(GLsizei)value_to_store.size(),GL_TRUE,value_to_store.data()->data());
    return true;
}


std::string  fragment_shader_data::create_gl_shader()
{
    if (id() != 0U)
        return "";

    TMPROF_BLOCK();

    std::vector<GLchar const*>  line_pointers;
    for (std::string const& line : get_lines_of_shader_code())
        line_pointers.push_back((GLchar const*)line.data());
    ASSUMPTION(!line_pointers.empty());
    std::string  error_message;
    m_id = detail::create_opengl_shader_program(GL_FRAGMENT_SHADER,line_pointers,error_message);
    INVARIANT((id() != 0U && error_message.empty()) || (id() == 0U && !error_message.empty()));
    INVARIANT(glapi().glGetError() == 0U);
    return error_message;
}


void  fragment_shader_data::destroy_gl_shader()
{
    if (id() == 0U)
        return;

    TMPROF_BLOCK();

    glapi().glDeleteProgram(id());
    INVARIANT(glapi().glGetError() == 0U);
}


void  fragment_shader_data::initialise(std::vector<std::string> const&  lines_of_shader_code)
{
    TMPROF_BLOCK();

    std::unordered_set<FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION>  input_buffer_bindings;
    std::unordered_set<FRAGMENT_SHADER_OUTPUT_BINDING_LOCATION>  output_buffer_bindings;
    std::unordered_set<FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME>  symbolic_names_of_used_uniforms;
    detail::parse_properties_from_fragment_shader_code(
            lines_of_shader_code,
            input_buffer_bindings,
            output_buffer_bindings,
            symbolic_names_of_used_uniforms
            );
    initialise(
            0U,
            input_buffer_bindings,
            output_buffer_bindings,
            symbolic_names_of_used_uniforms,
            lines_of_shader_code
            );
}


void  fragment_shader_data::initialise(
        GLuint const  id, 
        std::unordered_set<FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION> const&  input_buffer_bindings,
        std::unordered_set<FRAGMENT_SHADER_OUTPUT_BINDING_LOCATION> const&  output_buffer_bindings,
        std::unordered_set<FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME> const&  symbolic_names_of_used_uniforms,
        std::vector<std::string> const&  lines_of_shader_code
        )
{
    TMPROF_BLOCK();

    m_id = id;
    m_input_buffer_bindings = input_buffer_bindings;
    m_output_buffer_bindings = output_buffer_bindings;
    m_symbolic_names_of_used_uniforms = symbolic_names_of_used_uniforms;
    m_lines_of_shader_code = lines_of_shader_code;
}


}}

namespace qtgl { namespace detail {


shaders_binding_data::shaders_binding_data(
        async::finalise_load_on_destroy_ptr  finaliser,
        boost::filesystem::path const&  vertex_shader_path,
        boost::filesystem::path const&  fragment_shader_path
        )
    : m_id(0U)
    , m_vertex_shader()
    , m_fragment_shader()
    , m_ready(false)
{
    TMPROF_BLOCK();

    auto const  on_shaders_load_finished = 
        std::bind(
            &shaders_binding_data::on_shaders_load_finished,
            this,
            finaliser,
            vertex_shader_path,
            fragment_shader_path,
            std::shared_ptr<bool>(new bool(false))  // See comment in shaders_binding_data::on_shaders_load_finished
                                                    // to see why we need this shared variable.
            );

    m_vertex_shader.insert_load_request(vertex_shader_path.string(), 1UL, on_shaders_load_finished);
    m_fragment_shader.insert_load_request(fragment_shader_path.string(), 1UL, on_shaders_load_finished);
}


shaders_binding_data::~shaders_binding_data()
{
    TMPROF_BLOCK();

    destroy_gl_binding();
}

void  shaders_binding_data::create_gl_binding()
{
    if (id() != 0U)
        return;

    TMPROF_BLOCK();

    glapi().glGenProgramPipelines(1U,&m_id);
    INVARIANT(id() != 0U);
    INVARIANT(glapi().glGetError() == 0U);
}


void  shaders_binding_data::destroy_gl_binding()
{
    if (id() == 0U)
        return;

    TMPROF_BLOCK();

    glapi().glDeleteProgramPipelines(1,&m_id);
    INVARIANT(glapi().glGetError() == 0U);
}


void  shaders_binding_data::on_shaders_load_finished(
        async::finalise_load_on_destroy_ptr  finaliser,
        boost::filesystem::path const&  vertex_shader_path,
        boost::filesystem::path const&  fragment_shader_path,
        std::shared_ptr<bool>  visited
        )
{
    TMPROF_BLOCK();

    // The load is finished when both vertex and fragment shaders are loaded.
    // Due to async load, any of the two can finish first. So, we have boolean
    // shared variable 'visited', which tells us whether we already processed
    // the first of the two shaders. Namely, for the first shader we only mark
    // the variable and terminate (i.e. we wait till the second is loaded).
    // And we do the proper load-finishing-work once the second shader is loaded.
    if (!*visited)
    {
        *visited = true;
        return;
    }

    if (get_vertex_shader().get_load_state() != async::LOAD_STATE::FINISHED_SUCCESSFULLY)
    {
        // Oh no! We failed to load the vertex shader!
        // So, let's also fail the load of the whole 'shaders_binding_data' resource.

        finaliser->force_finalisation_as_failure(
            "Load of vertex shader '" + vertex_shader_path.string() + "' has FAILED!"
            );
        return;
    }
    if (get_fragment_shader().get_load_state() != async::LOAD_STATE::FINISHED_SUCCESSFULLY)
    {
        // Oh no! We failed to load the fragment shader!
        // So, let's also fail the load of the whole 'shaders_binding_data' resource.

        finaliser->force_finalisation_as_failure(
            "Load of fragment shader '" + fragment_shader_path.string() + "' has FAILED!"
            );
        return;
    }

    initialise(0U, get_vertex_shader(), get_fragment_shader());
}


void  shaders_binding_data::initialise(
        GLuint const  id,
        vertex_shader const  vertex_shader,
        fragment_shader const  fragment_shader
        )
{
    TMPROF_BLOCK();

    m_id = id;
    m_vertex_shader = vertex_shader;
    m_fragment_shader = fragment_shader;
    m_ready = false;

    ASSUMPTION(compatible(get_vertex_shader().get_output_buffer_bindings(),get_fragment_shader().get_input_buffer_bindings()));
}


}}

namespace qtgl {


bool  shaders_binding::ready() const
{
    TMPROF_BLOCK();

    if (!loaded_successfully())
        return false;

    if (!resource().ready())
    {
        if (!get_vertex_shader().loaded_successfully() || !get_fragment_shader().loaded_successfully())
            return false;
        if (!get_vertex_shader().create_gl_shader().empty() || !get_fragment_shader().create_gl_shader().empty())
            return false;

        shaders_binding* const  mutable_this = const_cast<shaders_binding*>(this);

        mutable_this->create_gl_binding();
        glapi().glBindProgramPipeline(id());

        glapi().glUseProgramStages(id(), GL_VERTEX_SHADER_BIT, get_vertex_shader().id());
        glapi().glUseProgramStages(id(), GL_FRAGMENT_SHADER_BIT, get_fragment_shader().id());

        mutable_this->set_ready();
    }

    return true;
}


bool  shaders_binding::make_current() const
{
    TMPROF_BLOCK();

    if (!ready())
        return false;

    glapi().glBindProgramPipeline(id());

    return true;
}


bool  make_current(shaders_binding const&  binding)
{
    return binding.make_current();
}


}
