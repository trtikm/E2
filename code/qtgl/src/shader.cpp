#include <qtgl/shader.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/msgstream.hpp>

namespace qtgl { namespace detail {


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

    static std::string const  shader_type_name = shader_type == GL_VERTEX_SHADER   ? "GL_VERTEX_SHADER"    :
                                                 shader_type == GL_FRAGMENT_SHADER ? "GL_FRAGMENT_SHADER"  :
                                                                                     "GL_??_SHADER"        ;

    GLuint const  shader = glapi().glCreateShader(shader_type);
    {
        if (shader == 0U)
        {
            error_message = "Cannot create opengl shader object of type " + shader_type_name;
            INVARIANT(glapi().glGetError() == 0U);
            return 0U;
        }
        glapi().glShaderSource(shader,(GLsizei)line_pointers.size(),(GLchar const**)&line_pointers.at(0),nullptr);
        glapi().glCompileShader(shader);
        {
            int  is_build_successfull;
            glapi().glGetShaderiv(shader, GL_COMPILE_STATUS, &is_build_successfull);
            if (is_build_successfull != GL_TRUE)
            {
                int  log_length;
                glapi().glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
                if (log_length <= 0)
                    error_message = "Compiling shader of type " + shader_type_name + " has failed. "
                                    "Cannot receive the error message, because the build log has negative length.";
                else
                {
                    error_message.resize(log_length + 1U, 0U);
                    glapi().glGetShaderInfoLog(shader, log_length, &log_length, &error_message.at(0U));
                }
                glapi().glDeleteShader(shader);
                INVARIANT(glapi().glGetError() == 0U);
                return 0U;
            }
        }
    }

    GLuint const  shader_program = glapi().glCreateProgram();
    {
        if (shader_program == 0U)
        {
            error_message = "Cannot create opengl shader program object after compiling a shader of type " + shader_type_name;
            glapi().glDeleteShader(shader);
            INVARIANT(glapi().glGetError() == 0U);
            return 0U;
        }
        glapi().glProgramParameteri(shader_program,GL_PROGRAM_SEPARABLE,GL_TRUE);
        glapi().glAttachShader(shader_program,shader);

        glapi().glLinkProgram(shader_program);
        {
            int  is_build_successfull;
            glapi().glGetProgramiv(shader_program, GL_LINK_STATUS, &is_build_successfull);
            if (is_build_successfull != GL_TRUE)
            {
                int  log_length;
                glapi().glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &log_length);
                if (log_length <= 0)
                    error_message = "Linking of shader program of shader of type " + shader_type_name + " has failed. "
                                    "Cannot receive the error message, because the build log has negative length.";
                else
                {
                    error_message.resize(log_length + 1U, 0U);
                    glapi().glGetProgramInfoLog(shader_program, log_length, &log_length, &error_message.at(0U));
                }
                glapi().glDetachShader(shader_program, shader);
                glapi().glDeleteProgram(shader_program);
                glapi().glDeleteShader(shader);
                INVARIANT(glapi().glGetError() == 0U);
                return 0U;
            }
        }

        glapi().glDetachShader(shader_program,shader);
    }

    glapi().glDeleteShader(shader);

    INVARIANT(glapi().glGetError() == 0U);

    return shader_program;
}


}}

namespace qtgl { namespace detail {


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


bool  vertex_shader_data::set_uniform_variable(std::string const&  variable_name, natural_32_bit const  value_to_store)
{
    TMPROF_BLOCK();

    GLint const  layout_location = glapi().glGetUniformLocation(id(),variable_name.c_str());
    if (layout_location == -1)
    {
        ASSUMPTION(get_symbolic_names_of_used_uniforms().count(to_symbolic_uniform_name_of_vertex_shader(variable_name)) == 1UL);
        INVARIANT(glapi().glGetError() == 0U);
        return false;
    }
    glapi().glProgramUniform1ui(id(),layout_location,value_to_store);
    INVARIANT(glapi().glGetError() == 0U);
    return true;
}


bool  vertex_shader_data::set_uniform_variable(std::string const&  variable_name, float_32_bit const  value_to_store)
{
    TMPROF_BLOCK();

    GLint const  layout_location = glapi().glGetUniformLocation(id(),variable_name.c_str());
    if (layout_location == -1)
    {
        ASSUMPTION(get_symbolic_names_of_used_uniforms().count(to_symbolic_uniform_name_of_vertex_shader(variable_name)) == 1UL);
        INVARIANT(glapi().glGetError() == 0U);
        return false;
    }
    glapi().glProgramUniform1f(id(),layout_location,value_to_store);
    INVARIANT(glapi().glGetError() == 0U);
    return true;
}

bool  vertex_shader_data::set_uniform_variable(std::string const&  variable_name, vector3 const&  value_to_store)
{
    TMPROF_BLOCK();

    GLint const  layout_location = glapi().glGetUniformLocation(id(), variable_name.c_str());
    if (layout_location == -1)
    {
        ASSUMPTION(get_symbolic_names_of_used_uniforms().count(to_symbolic_uniform_name_of_vertex_shader(variable_name)) == 1UL);
        INVARIANT(glapi().glGetError() == 0U);
        return false;
    }
    glapi().glProgramUniform3fv(id(), layout_location, 1U, value_to_store.data());
    INVARIANT(glapi().glGetError() == 0U);
    return true;
}

bool  vertex_shader_data::set_uniform_variable(std::string const&  variable_name, vector4 const&  value_to_store)
{
    TMPROF_BLOCK();

    GLint const  layout_location = glapi().glGetUniformLocation(id(), variable_name.c_str());
    if (layout_location == -1)
    {
        ASSUMPTION(get_symbolic_names_of_used_uniforms().count(to_symbolic_uniform_name_of_vertex_shader(variable_name)) == 1UL);
        INVARIANT(glapi().glGetError() == 0U);
        return false;
    }
    glapi().glProgramUniform4fv(id(), layout_location, 1U, value_to_store.data());
    INVARIANT(glapi().glGetError() == 0U);
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
        INVARIANT(glapi().glGetError() == 0U);
        return false;
    }
    glapi().glProgramUniformMatrix4fv(id(),layout_location,1U,GL_TRUE,value_to_store.data());
    INVARIANT(glapi().glGetError() == 0U);
    return true;
}


bool  vertex_shader_data::set_uniform_variable(std::string const&  variable_name, std::vector<matrix44> const&  value_to_store)
{
    TMPROF_BLOCK();

    static_assert(sizeof(matrix44)==4*4*sizeof(float_32_bit),"");
    ASSUMPTION(value_to_store.size() <= num_elements(VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::MATRICES_FROM_MODEL_TO_CAMERA));

    GLint const  layout_location = glapi().glGetUniformLocation(id(),variable_name.c_str());
    if (layout_location == -1)
    {
        ASSUMPTION(get_symbolic_names_of_used_uniforms().count(to_symbolic_uniform_name_of_vertex_shader(variable_name)) == 1UL);
        INVARIANT(glapi().glGetError() == 0U);
        return false;
    }
    glapi().glProgramUniformMatrix4fv(id(),layout_location,(GLsizei)value_to_store.size(),GL_TRUE,value_to_store.data()->data());
    INVARIANT(glapi().glGetError() == 0U);
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


}}

namespace qtgl { namespace detail {


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


bool  fragment_shader_data::set_uniform_variable(std::string const&  variable_name, integer_32_bit const  value_to_store)
{
    TMPROF_BLOCK();

    GLint const  layout_location = glapi().glGetUniformLocation(id(), variable_name.c_str());
    if (layout_location == -1)
    {
        ASSUMPTION(get_symbolic_names_of_used_uniforms().count(to_symbolic_uniform_name_of_fragment_shader(variable_name)) == 1UL);
        INVARIANT(glapi().glGetError() == 0U);
        return false;
    }
    glapi().glProgramUniform1i(id(), layout_location, (int)value_to_store);
    INVARIANT(glapi().glGetError() == 0U);
    return true;
}


bool  fragment_shader_data::set_uniform_variable(std::string const&  variable_name, natural_32_bit const  value_to_store)
{
    TMPROF_BLOCK();

    GLint const  layout_location = glapi().glGetUniformLocation(id(),variable_name.c_str());
    if (layout_location == -1)
    {
        ASSUMPTION(get_symbolic_names_of_used_uniforms().count(to_symbolic_uniform_name_of_fragment_shader(variable_name)) == 1UL);
        INVARIANT(glapi().glGetError() == 0U);
        return false;
    }
    glapi().glProgramUniform1ui(id(),layout_location,value_to_store);
    INVARIANT(glapi().glGetError() == 0U);
    return true;
}


bool  fragment_shader_data::set_uniform_variable(std::string const&  variable_name, float_32_bit const  value_to_store)
{
    TMPROF_BLOCK();

    GLint const  layout_location = glapi().glGetUniformLocation(id(),variable_name.c_str());
    if (layout_location == -1)
    {
        ASSUMPTION(get_symbolic_names_of_used_uniforms().count(to_symbolic_uniform_name_of_fragment_shader(variable_name)) == 1UL);
        INVARIANT(glapi().glGetError() == 0U);
        return false;
    }
    glapi().glProgramUniform1f(id(),layout_location,value_to_store);
    INVARIANT(glapi().glGetError() == 0U);
    return true;
}

bool  fragment_shader_data::set_uniform_variable(std::string const&  variable_name, vector3 const&  value_to_store)
{
    TMPROF_BLOCK();

    GLint const  layout_location = glapi().glGetUniformLocation(id(), variable_name.c_str());
    if (layout_location == -1)
    {
        ASSUMPTION(get_symbolic_names_of_used_uniforms().count(to_symbolic_uniform_name_of_fragment_shader(variable_name)) == 1UL);
        INVARIANT(glapi().glGetError() == 0U);
        return false;
    }
    glapi().glProgramUniform3fv(id(), layout_location, 1U, value_to_store.data());
    INVARIANT(glapi().glGetError() == 0U);
    return true;
}

bool  fragment_shader_data::set_uniform_variable(std::string const&  variable_name, vector4 const&  value_to_store)
{
    TMPROF_BLOCK();

    GLint const  layout_location = glapi().glGetUniformLocation(id(), variable_name.c_str());
    if (layout_location == -1)
    {
        ASSUMPTION(get_symbolic_names_of_used_uniforms().count(to_symbolic_uniform_name_of_fragment_shader(variable_name)) == 1UL);
        INVARIANT(glapi().glGetError() == 0U);
        return false;
    }
    glapi().glProgramUniform4fv(id(), layout_location, 1U, value_to_store.data());
    INVARIANT(glapi().glGetError() == 0U);
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
        INVARIANT(glapi().glGetError() == 0U);
        return false;
    }
    glapi().glProgramUniformMatrix4fv(id(),layout_location,1U,GL_TRUE,value_to_store.data());
    INVARIANT(glapi().glGetError() == 0U);
    return true;
}


bool  fragment_shader_data::set_uniform_variable(std::string const&  variable_name, std::vector<matrix44> const&  value_to_store)
{
    TMPROF_BLOCK();

    static_assert(sizeof(matrix44)==4*4*sizeof(float_32_bit),"");
    ASSUMPTION(value_to_store.size() <= num_elements(VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::MATRICES_FROM_MODEL_TO_CAMERA));

    GLint const  layout_location = glapi().glGetUniformLocation(id(),variable_name.c_str());
    if (layout_location == -1)
    {
        ASSUMPTION(get_symbolic_names_of_used_uniforms().count(to_symbolic_uniform_name_of_fragment_shader(variable_name)) == 1UL);
        INVARIANT(glapi().glGetError() == 0U);
        return false;
    }
    glapi().glProgramUniformMatrix4fv(id(),layout_location,(GLsizei)value_to_store.size(),GL_TRUE,value_to_store.data()->data());
    INVARIANT(glapi().glGetError() == 0U);
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


}}

namespace qtgl { namespace detail {


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
    INVARIANT(glapi().glGetError() == 0U);

    return true;
}


bool  make_current(shaders_binding const&  binding)
{
    return binding.make_current();
}


}
