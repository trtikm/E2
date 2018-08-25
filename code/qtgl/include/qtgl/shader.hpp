#ifndef QTGL_SHADER_HPP_INCLUDED
#   define QTGL_SHADER_HPP_INCLUDED

#   include <qtgl/shader_data_bindings.hpp>
#   include <qtgl/glapi.hpp>
#   include <angeo/tensor_math.hpp>
#   include <utility/async_resource_load.hpp>
#   include <boost/filesystem/path.hpp>
#   include <unordered_set>
#   include <string>
#   include <vector>

namespace qtgl { namespace detail {


struct  vertex_shader_data
{
    vertex_shader_data(
            async::finalise_load_on_destroy_ptr,
            GLuint const  id, 
            std::unordered_set<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION> const&  input_buffer_bindings,
            std::unordered_set<VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION> const&  output_buffer_bindings,
            std::unordered_set<VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME> const&  symbolic_names_of_used_uniforms,
            std::vector<std::string> const&  lines_of_shader_code
            );

    ~vertex_shader_data();

    GLuint  id() const { return m_id; }
    std::unordered_set<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION> const&  get_input_buffer_bindings() const
    { return m_input_buffer_bindings; }
    std::unordered_set<VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION> const&  get_output_buffer_bindings() const
    { return m_output_buffer_bindings; }
    std::unordered_set<VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME> const&  get_symbolic_names_of_used_uniforms() const
    { return m_symbolic_names_of_used_uniforms; }
    std::vector<std::string> const&  get_lines_of_shader_code() const
    { return m_lines_of_shader_code; }

    bool  set_uniform_variable(std::string const&  variable_name, natural_32_bit const  value_to_store);
    bool  set_uniform_variable(std::string const&  variable_name, float_32_bit const  value_to_store);
    bool  set_uniform_variable(std::string const&  variable_name, vector3 const&  value_to_store);
    bool  set_uniform_variable(std::string const&  variable_name, vector4 const&  value_to_store);
    bool  set_uniform_variable(std::string const&  variable_name, matrix44 const&  value_to_store);
    bool  set_uniform_variable(std::string const&  variable_name, std::vector<matrix44> const&  value_to_store);

    std::string  create_gl_shader();
    void  destroy_gl_shader();

private:

    GLuint  m_id;
    std::unordered_set<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION>  m_input_buffer_bindings;
    std::unordered_set<VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION>  m_output_buffer_bindings;
    std::unordered_set<VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME>  m_symbolic_names_of_used_uniforms;
    std::vector<std::string>  m_lines_of_shader_code;
};


}}

namespace qtgl {


struct  vertex_shader : public async::resource_accessor<detail::vertex_shader_data>
{
    vertex_shader()
        : async::resource_accessor<detail::vertex_shader_data>()
    {}

    vertex_shader(
            std::unordered_set<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION> const&  input_buffer_bindings,
            std::unordered_set<VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION> const&  output_buffer_bindings,
            std::unordered_set<VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME> const&  symbolic_names_of_used_uniforms,
            std::vector<std::string> const&  lines_of_shader_code,
            std::string const&  key = "",
            GLuint const  id = 0U
            )
        : async::resource_accessor<detail::vertex_shader_data>(
                key.empty() ? async::key_type("qtgl::vertex_shader") : async::key_type{ "qtgl::vertex_shader", key },
                async::notification_callback_type(),
                id,
                input_buffer_bindings,
                output_buffer_bindings,
                symbolic_names_of_used_uniforms,
                lines_of_shader_code
                )
    {}

    GLuint  id() const { return resource().id(); }

    std::unordered_set<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION> const&  get_input_buffer_bindings() const
    { return resource().get_input_buffer_bindings(); }
    
    std::unordered_set<VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION> const&  get_output_buffer_bindings() const
    { return resource().get_output_buffer_bindings(); }
    
    std::unordered_set<VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME> const&  get_symbolic_names_of_used_uniforms() const
    { return resource().get_symbolic_names_of_used_uniforms(); }
    
    std::vector<std::string> const&  get_lines_of_shader_code() const
    { return resource().get_lines_of_shader_code(); }

    template<typename value_type>
    bool  set_uniform_variable(VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME const  symbolic_name, value_type const&  value_to_store)
    {
        return set_uniform_variable(name(symbolic_name), value_to_store);
    }

    bool  set_uniform_variable(std::string const&  variable_name, natural_32_bit const  value_to_store)
    { return resource().set_uniform_variable(variable_name, value_to_store); }
    
    bool  set_uniform_variable(std::string const&  variable_name, float_32_bit const  value_to_store)
    { return resource().set_uniform_variable(variable_name, value_to_store); }
    
    bool  set_uniform_variable(std::string const&  variable_name, vector3 const&  value_to_store)
    { return resource().set_uniform_variable(variable_name, value_to_store); }

    bool  set_uniform_variable(std::string const&  variable_name, vector4 const&  value_to_store)
    { return resource().set_uniform_variable(variable_name, value_to_store); }

    bool  set_uniform_variable(std::string const&  variable_name, matrix44 const&  value_to_store)
    { return resource().set_uniform_variable(variable_name, value_to_store); }
    
    bool  set_uniform_variable(std::string const&  variable_name, std::vector<matrix44> const&  value_to_store)
    { return resource().set_uniform_variable(variable_name, value_to_store); }

    std::string  create_gl_shader() { return resource().create_gl_shader(); }
    void  destroy_gl_shader() { resource().destroy_gl_shader(); }
};


}

namespace qtgl { namespace detail {


struct  fragment_shader_data
{
    fragment_shader_data(
            async::finalise_load_on_destroy_ptr,
            GLuint const  id, 
            std::unordered_set<FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION> const&  input_buffer_bindings,
            std::unordered_set<FRAGMENT_SHADER_OUTPUT_BINDING_LOCATION> const&  output_buffer_bindings,
            std::unordered_set<FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME> const&  symbolic_names_of_used_uniforms,
            std::vector<std::string> const&  lines_of_shader_code
            );

    ~fragment_shader_data();

    GLuint  id() const { return m_id; }
    std::unordered_set<FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION> const&  get_input_buffer_bindings() const
    { return m_input_buffer_bindings; }
    std::unordered_set<FRAGMENT_SHADER_OUTPUT_BINDING_LOCATION> const&  get_output_buffer_bindings() const
    { return m_output_buffer_bindings; }
    std::unordered_set<FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME> const&  get_symbolic_names_of_used_uniforms() const
    { return m_symbolic_names_of_used_uniforms; }
    std::vector<std::string> const&  get_lines_of_shader_code() const
    { return m_lines_of_shader_code; }

    bool  set_uniform_variable(std::string const&  variable_name, natural_32_bit const  value_to_store);
    bool  set_uniform_variable(std::string const&  variable_name, float_32_bit const  value_to_store);
    bool  set_uniform_variable(std::string const&  variable_name, vector3 const&  value_to_store);
    bool  set_uniform_variable(std::string const&  variable_name, vector4 const&  value_to_store);
    bool  set_uniform_variable(std::string const&  variable_name, matrix44 const&  value_to_store);
    bool  set_uniform_variable(std::string const&  variable_name, std::vector<matrix44> const&  value_to_store);

    std::string  create_gl_shader();
    void  destroy_gl_shader();

private:

    GLuint  m_id;
    std::unordered_set<FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION>  m_input_buffer_bindings;
    std::unordered_set<FRAGMENT_SHADER_OUTPUT_BINDING_LOCATION>  m_output_buffer_bindings;
    std::unordered_set<FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME>  m_symbolic_names_of_used_uniforms;
    std::vector<std::string>  m_lines_of_shader_code;
};


}}

namespace qtgl {


struct  fragment_shader : public async::resource_accessor<detail::fragment_shader_data>
{
    fragment_shader()
        : async::resource_accessor<detail::fragment_shader_data>()
    {}

    fragment_shader(
            std::unordered_set<FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION> const&  input_buffer_bindings,
            std::unordered_set<FRAGMENT_SHADER_OUTPUT_BINDING_LOCATION> const&  output_buffer_bindings,
            std::unordered_set<FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME> const&  symbolic_names_of_used_uniforms,
            std::vector<std::string> const&  lines_of_shader_code,
            std::string const&  key = "",
            GLuint const  id = 0U 
            )
        : async::resource_accessor<detail::fragment_shader_data>(
                key.empty() ? async::key_type("qtgl::fragment_shader") : async::key_type{ "qtgl::fragment_shader", key },
                async::notification_callback_type(),
                id,
                input_buffer_bindings,
                output_buffer_bindings,
                symbolic_names_of_used_uniforms,
                lines_of_shader_code
                )
    {}

    GLuint  id() const { return resource().id(); }
    std::unordered_set<FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION> const&  get_input_buffer_bindings() const
    { return resource().get_input_buffer_bindings(); }
    std::unordered_set<FRAGMENT_SHADER_OUTPUT_BINDING_LOCATION> const&  get_output_buffer_bindings() const
    { return resource().get_output_buffer_bindings(); }
    std::unordered_set<FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME> const&  get_symbolic_names_of_used_uniforms() const
    { return resource().get_symbolic_names_of_used_uniforms(); }
    std::vector<std::string> const&  get_lines_of_shader_code() const
    { return resource().get_lines_of_shader_code(); }

    template<typename value_type>
    bool  set_uniform_variable(FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME const  symbolic_name, value_type const&  value_to_store)
    {
        return set_uniform_variable(name(symbolic_name), value_to_store);
    }

    bool  set_uniform_variable(std::string const&  variable_name, natural_32_bit const  value_to_store)
    { return resource().set_uniform_variable(variable_name, value_to_store); }
    
    bool  set_uniform_variable(std::string const&  variable_name, float_32_bit const  value_to_store)
    { return resource().set_uniform_variable(variable_name, value_to_store); }
    
    bool  set_uniform_variable(std::string const&  variable_name, vector3 const&  value_to_store)
    { return resource().set_uniform_variable(variable_name, value_to_store); }
    
    bool  set_uniform_variable(std::string const&  variable_name, vector4 const&  value_to_store)
    { return resource().set_uniform_variable(variable_name, value_to_store); }
    
    bool  set_uniform_variable(std::string const&  variable_name, matrix44 const&  value_to_store)
    { return resource().set_uniform_variable(variable_name, value_to_store); }
    
    bool  set_uniform_variable(std::string const&  variable_name, std::vector<matrix44> const&  value_to_store)
    { return resource().set_uniform_variable(variable_name, value_to_store); }

    std::string  create_gl_shader() { return resource().create_gl_shader(); }
    void  destroy_gl_shader() { resource().destroy_gl_shader(); }
};


}

namespace qtgl { namespace detail {


struct shaders_binding_data
{
    shaders_binding_data(
            async::finalise_load_on_destroy_ptr,
            GLuint const  id,
            vertex_shader const  vertex_shader,
            fragment_shader const  fragment_shader
            )
    {
        initialise(id, vertex_shader, fragment_shader);
    }

    ~shaders_binding_data();

    GLuint  id() const { return m_id; }
    vertex_shader  get_vertex_shader() const { return m_vertex_shader; }
    fragment_shader  get_fragment_shader() const { return m_fragment_shader; }

    bool  ready() const { return m_ready; }
    void  set_ready() { m_ready = true; }

    void  create_gl_binding();
    void  destroy_gl_binding();

private:

    void  initialise(
            GLuint const  id,
            vertex_shader const  vertex_shader,
            fragment_shader const  fragment_shader
            );

    GLuint  m_id;
    vertex_shader  m_vertex_shader;
    fragment_shader  m_fragment_shader;

    bool  m_ready;
};


}}

namespace qtgl {


struct  shaders_binding : public async::resource_accessor<detail::shaders_binding_data>
{
    shaders_binding()
        : async::resource_accessor<detail::shaders_binding_data>()
    {}

    shaders_binding(
            vertex_shader const  vertex_shader,
            fragment_shader const  fragment_shader,
            std::string const&  key = "",
            GLuint const  id = 0U
            )
        : async::resource_accessor<detail::shaders_binding_data>(
                key.empty() ? async::key_type("qtgl::shaders_binding") : async::key_type{ "qtgl::shaders_binding", key },
                async::notification_callback_type(),
                id,
                vertex_shader,
                fragment_shader
                )
    {}

    GLuint  id() const { return resource().id(); }
    vertex_shader  get_vertex_shader() const { return resource().get_vertex_shader(); }
    fragment_shader  get_fragment_shader() const { return resource().get_fragment_shader(); }

    bool  ready() const;
    bool  make_current() const;

private:

    void  set_ready() { resource().set_ready(); }
    void  create_gl_binding() { return resource().create_gl_binding(); }
    void  destroy_gl_binding() { return resource().destroy_gl_binding(); }
};


bool  make_current(shaders_binding const&  binding);


}

#endif
