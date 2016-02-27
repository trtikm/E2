#include <qtgl/shader.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string.hpp>
#include <sstream>
#include <istream>
#include <unordered_set>
#include <vector>
#include <limits>

namespace qtgl { namespace detail {


void  add_macro_definitions_after_version(std::vector<std::string>&  lines,
                                          std::vector<std::string> const&  macro_definitions)
{
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
    ASSUMPTION(output_lines.empty());
    ASSUMPTION(shader_type == GL_VERTEX_SHADER || shader_type == GL_FRAGMENT_SHADER);

    std::vector<std::string> const  macro_definitions{
            shader_type == GL_VERTEX_SHADER         ? "#define SHADER_TYPE                        0\n\0"  :
            shader_type == GL_FRAGMENT_SHADER       ? "#define SHADER_TYPE                        1\n\0"  :
                                                      "#error \"Unknown shader type.\"\n\0"               ,
            "#define VERTEX_SHADER_TYPE                 0\n\0",
            "#define FRAGMENT_SHADER_TYPE               1\n\0",
            };
    output_lines.resize(macro_definitions.size() + 1U,"");

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
    return id == 0 || !error_message.empty() ? vertex_program_ptr() : vertex_program::create(id);
}

fragment_program_ptr  create_fragment_program(std::vector<std::string> const&  lines,
                                              std::string&  error_message)
{
    TMPROF_BLOCK();

    ASSUMPTION(error_message.empty());
    std::vector<GLchar const*>  line_pointers;
    for (std::string const& line : lines)
        if (!line.empty())
            line_pointers.push_back((GLchar const*)&line.at(0));
    ASSUMPTION(!line_pointers.empty());
    GLuint const  id = detail::create_opengl_shader_program(GL_FRAGMENT_SHADER,line_pointers,error_message);
    return id == 0U || !error_message.empty() ? fragment_program_ptr() : fragment_program::create(id);
}


}}

namespace qtgl {


vertex_program_ptr  vertex_program::create(std::istream&  source_code, std::string&  error_message)
{
    ASSUMPTION(error_message.empty());
    std::vector<std::string>  lines;
    error_message = detail::parse_lines(source_code,GL_VERTEX_SHADER,lines);
    if (!error_message.empty())
        return vertex_program_ptr{};
    return detail::create_vertex_program(lines,error_message);
}

vertex_program_ptr  vertex_program::create(boost::filesystem::path const&  shader_source_file, std::string&  error_message)
{
    ASSUMPTION(error_message.empty());
    std::vector<std::string>  lines;
    error_message = detail::parse_lines(shader_source_file,GL_VERTEX_SHADER,lines);
    if (!error_message.empty())
        return vertex_program_ptr{};
    return detail::create_vertex_program(lines,error_message);
}

vertex_program_ptr  vertex_program::create(GLuint const  id)
{
    return vertex_program_ptr{new vertex_program(id)};
}


vertex_program::~vertex_program()
{
    glapi().glDeleteProgram(id());
}


fragment_program_ptr  fragment_program::create(std::istream&  source_code, std::string&  error_message)
{
    ASSUMPTION(error_message.empty());
    std::vector<std::string>  lines;
    error_message = detail::parse_lines(source_code,GL_FRAGMENT_SHADER,lines);
    if (!error_message.empty())
        return fragment_program_ptr{};
    return detail::create_fragment_program(lines,error_message);
}

fragment_program_ptr  fragment_program::create(boost::filesystem::path const&  shader_source_file,
                                               std::string&  error_message)
{
    ASSUMPTION(error_message.empty());
    std::vector<std::string>  lines;
    error_message = detail::parse_lines(shader_source_file,GL_FRAGMENT_SHADER,lines);
    if (!error_message.empty())
        return fragment_program_ptr{};
    return detail::create_fragment_program(lines,error_message);
}

fragment_program_ptr  fragment_program::create(GLuint const  id)
{
    return fragment_program_ptr{new fragment_program(id)};
}

fragment_program::~fragment_program()
{
    glapi().glDeleteProgram(id());
}


shaders_binding_ptr  shaders_binding::create(vertex_program_ptr const  vertex_program,
                                             fragment_program_ptr const  fragment_program)
{
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
    glapi().glBindProgramPipeline(shaders_binding->id());
}


void  set_uniform_variable(vertex_program_ptr const  shader_program,
                           std::string const&  variable_name,
                           float_32_bit const  value_to_store)
{
    GLint const  layout_location = glapi().glGetUniformLocation(shader_program->id(),variable_name.c_str());
    ASSUMPTION(layout_location != -1);
    glapi().glProgramUniform1f(shader_program->id(),layout_location,value_to_store);
}

void  set_uniform_variable(vertex_program_ptr const  shader_program,
                           std::string const&  variable_name,
                           matrix44 const&  value_to_store)
{
    GLint const  layout_location = glapi().glGetUniformLocation(shader_program->id(),variable_name.c_str());
    ASSUMPTION(layout_location != -1);
    glapi().glProgramUniformMatrix4fv(shader_program->id(),layout_location,1U,GL_TRUE,value_to_store.data());
}


}