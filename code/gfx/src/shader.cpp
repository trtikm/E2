#include <gfx/shader.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/msgstream.hpp>

namespace gfx { namespace detail {


GLuint  create_opengl_shader(GLenum shader_type,
                             std::vector<GLchar const*> const&  line_pointers,
                             std::string&  error_message)
{
    TMPROF_BLOCK();

    if (!error_message.empty())
    {
        error_message = "The error_message string is initially not empty.";
        return 0U;
    }

    std::string const  shader_type_name = shader_type == GL_VERTEX_SHADER   ? "GL_VERTEX_SHADER"    :
                                          shader_type == GL_FRAGMENT_SHADER ? "GL_FRAGMENT_SHADER"  :
                                                                              "GL_??_SHADER"        ;

    GLuint const  shader = glCreateShader(shader_type);
    {
        if (shader == 0U)
        {
            error_message = "Cannot create opengl shader object of type " + shader_type_name;
            INVARIANT(glGetError() == 0U);
            return 0U;
        }
        glShaderSource(shader,(GLsizei)line_pointers.size(),(GLchar const**)&line_pointers.at(0),nullptr);
        INVARIANT(glGetError() == 0U);
        glCompileShader(shader);
        INVARIANT(glGetError() == 0U);
        {
            int  is_build_successfull;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &is_build_successfull);
            if (is_build_successfull != GL_TRUE)
            {
                int  log_length;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
                if (log_length <= 0)
                    error_message = "Compiling shader of type " + shader_type_name + " has failed. "
                                    "Cannot receive the error message, because the build log has negative length.";
                else
                {
                    error_message.resize(log_length + 1U, 0U);
                    glGetShaderInfoLog(shader, log_length, &log_length, &error_message.at(0U));
                }
                glDeleteShader(shader);
                INVARIANT(glGetError() == 0U);
                return 0U;
            }
        }
    }

    return shader;
}


GLuint  create_opengl_program(GLuint const  vertex_shader, GLuint const  fragment_shader, std::string&  error_message)
{
    TMPROF_BLOCK();

    if (vertex_shader == 0U)
    {
        error_message = "The vertex shader is not valid.";
        return 0U;
    }
    if (fragment_shader == 0U)
    {
        error_message = "The fragment shader is not valid.";
        return 0U;
    }
    if (!error_message.empty())
    {
        error_message = "The error_message string is initially not empty.";
        return 0U;
    }

    GLuint const  shader_program = glCreateProgram();
    {
        if (shader_program == 0U)
        {
            error_message = "Cannot create opengl program.";
            INVARIANT(glGetError() == 0U);
            return 0U;
        }
        glAttachShader(shader_program,vertex_shader);
        INVARIANT(glGetError() == 0U);
        glAttachShader(shader_program,fragment_shader);
        INVARIANT(glGetError() == 0U);

        glLinkProgram(shader_program);
        INVARIANT(glGetError() == 0U);
        {
            int  is_build_successfull;
            glGetProgramiv(shader_program, GL_LINK_STATUS, &is_build_successfull);
            if (is_build_successfull != GL_TRUE)
            {
                int  log_length;
                glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &log_length);
                if (log_length <= 0)
                    error_message = "Linking of program has failed. Cannot receive the error message, "
                                    "because the build log has negative length.";
                else
                {
                    error_message.resize(log_length + 1U, 0U);
                    glGetProgramInfoLog(shader_program, log_length, &log_length, &error_message.at(0U));
                }
                glDetachShader(shader_program, vertex_shader);
                glDetachShader(shader_program, fragment_shader);
                glDeleteProgram(shader_program);
                INVARIANT(glGetError() == 0U);
                return 0U;
            }
        }

        glDetachShader(shader_program, vertex_shader);
        INVARIANT(glGetError() == 0U);
        glDetachShader(shader_program, fragment_shader);
        INVARIANT(glGetError() == 0U);
    }

    INVARIANT(glGetError() == 0U);

    return shader_program;
}


}}

namespace gfx { namespace detail {


vertex_shader_data::vertex_shader_data(
        async::finalise_load_on_destroy_ptr,
        GLuint const  id, 
        std::unordered_set<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION> const&  input_buffer_bindings,
        std::unordered_set<VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION> const&  output_buffer_bindings,
        std::unordered_set<VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME> const&  symbolic_names_of_used_uniforms,
        std::vector<std::string> const&  lines_of_shader_code
        )
    : m_id(id)
    , m_input_buffer_bindings(input_buffer_bindings)
    , m_output_buffer_bindings(output_buffer_bindings)
    , m_symbolic_names_of_used_uniforms(symbolic_names_of_used_uniforms)
    , m_lines_of_shader_code(lines_of_shader_code)
{
    TMPROF_BLOCK();
}


vertex_shader_data::~vertex_shader_data()
{
    TMPROF_BLOCK();

    destroy_gl_shader();
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
    m_id = detail::create_opengl_shader(GL_VERTEX_SHADER,line_pointers,error_message);
    INVARIANT((id() != 0U && error_message.empty()) || (id() == 0U && !error_message.empty()));
    return error_message;
}


void  vertex_shader_data::destroy_gl_shader()
{
    if (id() == 0U)
        return;

    TMPROF_BLOCK();

    glDeleteShader(id());
    INVARIANT(glGetError() == 0U);

    m_id = 0U;
}


}}

namespace gfx { namespace detail {


fragment_shader_data::fragment_shader_data(
        async::finalise_load_on_destroy_ptr,
        GLuint const  id, 
        std::unordered_set<FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION> const&  input_buffer_bindings,
        std::unordered_set<FRAGMENT_SHADER_OUTPUT_BINDING_LOCATION> const&  output_buffer_bindings,
        std::unordered_set<FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME> const&  symbolic_names_of_used_uniforms,
        std::vector<std::string> const&  lines_of_shader_code
        )
    : m_id(id)
    , m_input_buffer_bindings(input_buffer_bindings)
    , m_output_buffer_bindings(output_buffer_bindings)
    , m_symbolic_names_of_used_uniforms(symbolic_names_of_used_uniforms)
    , m_lines_of_shader_code(lines_of_shader_code)
{
    TMPROF_BLOCK();
}


fragment_shader_data::~fragment_shader_data()
{
    TMPROF_BLOCK();

    destroy_gl_shader();
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
    m_id = detail::create_opengl_shader(GL_FRAGMENT_SHADER,line_pointers,error_message);
    INVARIANT((id() != 0U && error_message.empty()) || (id() == 0U && !error_message.empty()));
    return error_message;
}


void  fragment_shader_data::destroy_gl_shader()
{
    if (id() == 0U)
        return;

    TMPROF_BLOCK();

    glDeleteShader(id());
    INVARIANT(glGetError() == 0U);

    m_id = 0U;
}


}}

namespace gfx { namespace detail {


shaders_binding_data::~shaders_binding_data()
{
    TMPROF_BLOCK();

    destroy_gl_binding();
}

bool  shaders_binding_data::set_uniform_variable(std::string const&  variable_name, integer_32_bit const  value_to_store)
{
    TMPROF_BLOCK();

    GLint const  layout_location = glGetUniformLocation(id(), variable_name.c_str());
    if (layout_location == -1)
    {
        INVARIANT(glGetError() == 0U);
        return false;
    }
    glUniform1i(layout_location, (int)value_to_store);
    INVARIANT(glGetError() == 0U);
    return true;
}


bool  shaders_binding_data::set_uniform_variable(std::string const&  variable_name, natural_32_bit const  value_to_store)
{
    TMPROF_BLOCK();

    GLint const  layout_location = glGetUniformLocation(id(),variable_name.c_str());
    if (layout_location == -1)
    {
        INVARIANT(glGetError() == 0U);
        return false;
    }
    glUniform1ui(layout_location,value_to_store);
    INVARIANT(glGetError() == 0U);
    return true;
}


bool  shaders_binding_data::set_uniform_variable(std::string const&  variable_name, float_32_bit const  value_to_store)
{
    TMPROF_BLOCK();

    GLint const  layout_location = glGetUniformLocation(id(),variable_name.c_str());
    if (layout_location == -1)
    {
        INVARIANT(glGetError() == 0U);
        return false;
    }
    glUniform1f(layout_location,value_to_store);
    INVARIANT(glGetError() == 0U);
    return true;
}

bool  shaders_binding_data::set_uniform_variable(std::string const&  variable_name, vector3 const&  value_to_store)
{
    TMPROF_BLOCK();

    GLint const  layout_location = glGetUniformLocation(id(), variable_name.c_str());
    if (layout_location == -1)
    {
        INVARIANT(glGetError() == 0U);
        return false;
    }
    glUniform3fv(layout_location, 1U, value_to_store.data());
    INVARIANT(glGetError() == 0U);
    return true;
}

bool  shaders_binding_data::set_uniform_variable(std::string const&  variable_name, vector4 const&  value_to_store)
{
    TMPROF_BLOCK();

    GLint const  layout_location = glGetUniformLocation(id(), variable_name.c_str());
    if (layout_location == -1)
    {
        INVARIANT(glGetError() == 0U);
        return false;
    }
    glUniform4fv(layout_location, 1U, value_to_store.data());
    INVARIANT(glGetError() == 0U);
    return true;
}

bool  shaders_binding_data::set_uniform_variable(std::string const&  variable_name, matrix44 const&  value_to_store)
{
    TMPROF_BLOCK();

    static_assert(sizeof(matrix44)==4*4*sizeof(float_32_bit),"");

    GLint const  layout_location = glGetUniformLocation(id(),variable_name.c_str());
    if (layout_location == -1)
    {
        INVARIANT(glGetError() == 0U);
        return false;
    }
    glUniformMatrix4fv(layout_location,1U,GL_TRUE,value_to_store.data());
    INVARIANT(glGetError() == 0U);
    return true;
}


bool  shaders_binding_data::set_uniform_variable(std::string const&  variable_name, std::vector<matrix44> const&  value_to_store)
{
    TMPROF_BLOCK();

    static_assert(sizeof(matrix44)==4*4*sizeof(float_32_bit),"");
    ASSUMPTION(value_to_store.size() <= num_elements(VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::MATRICES_FROM_MODEL_TO_CAMERA));

    GLint const  layout_location = glGetUniformLocation(id(),variable_name.c_str());
    if (layout_location == -1)
    {
        INVARIANT(glGetError() == 0U);
        return false;
    }
    glUniformMatrix4fv(layout_location,(GLsizei)value_to_store.size(),GL_TRUE,value_to_store.data()->data());
    INVARIANT(glGetError() == 0U);
    return true;
}


std::string  shaders_binding_data::create_gl_binding()
{
    if (id() != 0U)
        return "";

    TMPROF_BLOCK();

    std::string  error_message;

    error_message = get_vertex_shader().create_gl_shader();
    if (!error_message.empty())
        return error_message;

    error_message = get_fragment_shader().create_gl_shader();
    if (!error_message.empty())
        return error_message;

    m_id = create_opengl_program(get_vertex_shader().id(),get_fragment_shader().id(),error_message);
    INVARIANT((id() != 0U && error_message.empty()) || (id() == 0U && !error_message.empty()));
    if (!error_message.empty())
        return error_message;

#if PLATFORM() == PLATFORM_WEBASSEMBLY()
    for (VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION const location : get_vertex_shader().get_input_buffer_bindings())
    {
        GLint const gl_location = glGetAttribLocation(m_id, name(location).c_str());
        INVARIANT(gl_location >= 0);
        m_locations.insert({ location, gl_location });
    }
#endif

    return error_message;
}


void  shaders_binding_data::destroy_gl_binding()
{
    if (id() == 0U)
        return;

    TMPROF_BLOCK();

    glDeleteProgram(m_id);
    INVARIANT(glGetError() == 0U);

    m_id = 0U;
#if PLATFORM() == PLATFORM_WEBASSEMBLY()
    m_locations.clear();
#endif
    m_ready = false;
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
#if PLATFORM() == PLATFORM_WEBASSEMBLY()
    m_locations.clear();
#endif
    m_ready = false;

    ASSUMPTION(compatible(get_vertex_shader().get_output_buffer_bindings(),get_fragment_shader().get_input_buffer_bindings()));
}


}}

namespace gfx {


bool  shaders_binding::ready() const
{
    TMPROF_BLOCK();

    if (!loaded_successfully())
        return false;

    if (!resource().ready())
    {
        if (!get_vertex_shader().loaded_successfully() || !get_fragment_shader().loaded_successfully())
            return false;

        shaders_binding* const  mutable_this = const_cast<shaders_binding*>(this);

        if (!mutable_this->create_gl_binding().empty())
            return false;

        mutable_this->set_ready();
    }

    return true;
}


bool  shaders_binding::make_current() const
{
    TMPROF_BLOCK();

    if (!ready())
        return false;

    glUseProgram(id());
    INVARIANT(glGetError() == 0U);

    return true;
}


bool  make_current(shaders_binding const&  binding)
{
    return binding.make_current();
}


}
