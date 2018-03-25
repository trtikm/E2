#ifndef QTGL_SHADER_HPP_INCLUDED
#   define QTGL_SHADER_HPP_INCLUDED

#   include <qtgl/shader_data_bindings.hpp>
#   include <qtgl/glapi.hpp>
#   include <angeo/tensor_math.hpp>
#   include <utility/async_resource_load.hpp>
#   include <boost/filesystem/path.hpp>
#   include <unordered_set>
#   include <istream>
#   include <string>
#   include <vector>
#   include <memory>

namespace qtgl { namespace detail {


struct  vertex_shader_data
{
    vertex_shader_data(boost::filesystem::path const&  path, async::finalise_load_on_destroy_ptr);

    vertex_shader_data(async::finalise_load_on_destroy_ptr, std::vector<std::string> const&  lines_of_shader_code)
    {
        initialise(lines_of_shader_code);
    }

    vertex_shader_data(
            async::finalise_load_on_destroy_ptr,
            GLuint const  id, 
            std::unordered_set<vertex_shader_input_buffer_binding_location> const&  input_buffer_bindings,
            std::unordered_set<vertex_shader_output_buffer_binding_location> const&  output_buffer_bindings,
            std::unordered_set<vertex_shader_uniform_symbolic_name> const&  symbolic_names_of_used_uniforms,
            std::vector<std::string> const&  lines_of_shader_code
            )
    {
        initialise(id, input_buffer_bindings, output_buffer_bindings, symbolic_names_of_used_uniforms, lines_of_shader_code);
    }

    ~vertex_shader_data();

    GLuint  id() const { return m_id; }
    std::unordered_set<vertex_shader_input_buffer_binding_location> const&  get_input_buffer_bindings() const
    { return m_input_buffer_bindings; }
    std::unordered_set<vertex_shader_output_buffer_binding_location> const&  get_output_buffer_bindings() const
    { return m_output_buffer_bindings; }
    std::unordered_set<vertex_shader_uniform_symbolic_name> const&  get_symbolic_names_of_used_uniforms() const
    { return m_symbolic_names_of_used_uniforms; }
    std::vector<std::string> const&  get_lines_of_shader_code() const
    { return m_lines_of_shader_code; }

    bool  set_uniform_variable(std::string const&  variable_name, natural_32_bit const  value_to_store);
    bool  set_uniform_variable(std::string const&  variable_name, float_32_bit const  value_to_store);
    bool  set_uniform_variable(std::string const&  variable_name, vector4 const&  value_to_store);
    bool  set_uniform_variable(std::string const&  variable_name, matrix44 const&  value_to_store);
    bool  set_uniform_variable(std::string const&  variable_name, std::vector<matrix44> const&  value_to_store);

    std::string  create_gl_shader();
    void  destroy_gl_shader();

private:
    void  initialise(std::vector<std::string> const&  lines_of_shader_code);
    void  initialise(
            GLuint const  id, 
            std::unordered_set<vertex_shader_input_buffer_binding_location> const&  input_buffer_bindings,
            std::unordered_set<vertex_shader_output_buffer_binding_location> const&  output_buffer_bindings,
            std::unordered_set<vertex_shader_uniform_symbolic_name> const&  symbolic_names_of_used_uniforms,
            std::vector<std::string> const&  lines_of_shader_code
            );

    GLuint  m_id;
    std::unordered_set<vertex_shader_input_buffer_binding_location>  m_input_buffer_bindings;
    std::unordered_set<vertex_shader_output_buffer_binding_location>  m_output_buffer_bindings;
    std::unordered_set<vertex_shader_uniform_symbolic_name>  m_symbolic_names_of_used_uniforms;
    std::vector<std::string>  m_lines_of_shader_code;
};


}}

namespace qtgl {


struct  vertex_shader : public async::resource_accessor<detail::vertex_shader_data>
{
    vertex_shader()
        : async::resource_accessor<detail::vertex_shader_data>()
    {}

    explicit vertex_shader(boost::filesystem::path const&  path)
        : async::resource_accessor<detail::vertex_shader_data>(path.string(), 1U)
    {}

    vertex_shader(std::vector<std::string> const&  lines_of_shader_code, boost::filesystem::path const&  path = "")
        : async::resource_accessor<detail::vertex_shader_data>(
                path.string(),
                async::notification_callback_type(),
                lines_of_shader_code
                )
    {}

    vertex_shader(
            GLuint const  id, 
            std::unordered_set<vertex_shader_input_buffer_binding_location> const&  input_buffer_bindings,
            std::unordered_set<vertex_shader_output_buffer_binding_location> const&  output_buffer_bindings,
            std::unordered_set<vertex_shader_uniform_symbolic_name> const&  symbolic_names_of_used_uniforms,
            std::vector<std::string> const&  lines_of_shader_code,
            boost::filesystem::path const&  path = ""
            )
        : async::resource_accessor<detail::vertex_shader_data>(
                path.string(),
                async::notification_callback_type(),
                id,
                input_buffer_bindings,
                output_buffer_bindings,
                symbolic_names_of_used_uniforms,
                lines_of_shader_code
                )
    {}

    GLuint  id() const { return resource().id(); }

    std::unordered_set<vertex_shader_input_buffer_binding_location> const&  get_input_buffer_bindings() const
    { return resource().get_input_buffer_bindings(); }
    
    std::unordered_set<vertex_shader_output_buffer_binding_location> const&  get_output_buffer_bindings() const
    { return resource().get_output_buffer_bindings(); }
    
    std::unordered_set<vertex_shader_uniform_symbolic_name> const&  get_symbolic_names_of_used_uniforms() const
    { return resource().get_symbolic_names_of_used_uniforms(); }
    
    std::vector<std::string> const&  get_lines_of_shader_code() const
    { return resource().get_lines_of_shader_code(); }

    template<typename value_type>
    bool  set_uniform_variable(vertex_shader_uniform_symbolic_name const  symbolic_name, value_type const&  value_to_store)
    {
        return set_uniform_variable(uniform_name(symbolic_name), value_to_store);
    }

    bool  set_uniform_variable(std::string const&  variable_name, natural_32_bit const  value_to_store)
    { return resource().set_uniform_variable(variable_name, value_to_store); }
    
    bool  set_uniform_variable(std::string const&  variable_name, float_32_bit const  value_to_store)
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
    fragment_shader_data(boost::filesystem::path const&  path, async::finalise_load_on_destroy_ptr);

    fragment_shader_data(async::finalise_load_on_destroy_ptr, std::vector<std::string> const&  lines_of_shader_code)
    {
        initialise(lines_of_shader_code);
    }

    fragment_shader_data(
            async::finalise_load_on_destroy_ptr,
            GLuint const  id, 
            std::unordered_set<fragment_shader_input_buffer_binding_location> const&  input_buffer_bindings,
            std::unordered_set<fragment_shader_output_buffer_binding_location> const&  output_buffer_bindings,
            std::unordered_set<fragment_shader_uniform_symbolic_name> const&  symbolic_names_of_used_uniforms,
            std::vector<std::string> const&  lines_of_shader_code
            )
    {
        initialise(id, input_buffer_bindings, output_buffer_bindings, symbolic_names_of_used_uniforms, lines_of_shader_code);
    }

    ~fragment_shader_data();

    GLuint  id() const { return m_id; }
    std::unordered_set<fragment_shader_input_buffer_binding_location> const&  get_input_buffer_bindings() const
    { return m_input_buffer_bindings; }
    std::unordered_set<fragment_shader_output_buffer_binding_location> const&  get_output_buffer_bindings() const
    { return m_output_buffer_bindings; }
    std::unordered_set<fragment_shader_uniform_symbolic_name> const&  get_symbolic_names_of_used_uniforms() const
    { return m_symbolic_names_of_used_uniforms; }
    std::vector<std::string> const&  get_lines_of_shader_code() const
    { return m_lines_of_shader_code; }

    bool  set_uniform_variable(std::string const&  variable_name, natural_32_bit const  value_to_store);
    bool  set_uniform_variable(std::string const&  variable_name, float_32_bit const  value_to_store);
    bool  set_uniform_variable(std::string const&  variable_name, vector4 const&  value_to_store);
    bool  set_uniform_variable(std::string const&  variable_name, matrix44 const&  value_to_store);
    bool  set_uniform_variable(std::string const&  variable_name, std::vector<matrix44> const&  value_to_store);

    std::string  create_gl_shader();
    void  destroy_gl_shader();

private:
    void  initialise(std::vector<std::string> const&  lines_of_shader_code);
    void  initialise(
            GLuint const  id, 
            std::unordered_set<fragment_shader_input_buffer_binding_location> const&  input_buffer_bindings,
            std::unordered_set<fragment_shader_output_buffer_binding_location> const&  output_buffer_bindings,
            std::unordered_set<fragment_shader_uniform_symbolic_name> const&  symbolic_names_of_used_uniforms,
            std::vector<std::string> const&  lines_of_shader_code
            );

    GLuint  m_id;
    std::unordered_set<fragment_shader_input_buffer_binding_location>  m_input_buffer_bindings;
    std::unordered_set<fragment_shader_output_buffer_binding_location>  m_output_buffer_bindings;
    std::unordered_set<fragment_shader_uniform_symbolic_name>  m_symbolic_names_of_used_uniforms;
    std::vector<std::string>  m_lines_of_shader_code;
};


}}

namespace qtgl {


struct  fragment_shader : public async::resource_accessor<detail::fragment_shader_data>
{
    fragment_shader()
        : async::resource_accessor<detail::fragment_shader_data>()
    {}

    explicit fragment_shader(boost::filesystem::path const&  path)
        : async::resource_accessor<detail::fragment_shader_data>(path.string(), 1U)
    {}

    fragment_shader(std::vector<std::string> const&  lines_of_shader_code, boost::filesystem::path const&  path = "")
        : async::resource_accessor<detail::fragment_shader_data>(
                path.string(),
                async::notification_callback_type(),
                lines_of_shader_code
                )
    {}

    fragment_shader(
            GLuint const  id, 
            std::unordered_set<fragment_shader_input_buffer_binding_location> const&  input_buffer_bindings,
            std::unordered_set<fragment_shader_output_buffer_binding_location> const&  output_buffer_bindings,
            std::unordered_set<fragment_shader_uniform_symbolic_name> const&  symbolic_names_of_used_uniforms,
            std::vector<std::string> const&  lines_of_shader_code,
            boost::filesystem::path const&  path = ""
            )
        : async::resource_accessor<detail::fragment_shader_data>(
                path.string(),
                async::notification_callback_type(),
                id,
                input_buffer_bindings,
                output_buffer_bindings,
                symbolic_names_of_used_uniforms,
                lines_of_shader_code
                )
    {}

    GLuint  id() const { return resource().id(); }
    std::unordered_set<fragment_shader_input_buffer_binding_location> const&  get_input_buffer_bindings() const
    { return resource().get_input_buffer_bindings(); }
    std::unordered_set<fragment_shader_output_buffer_binding_location> const&  get_output_buffer_bindings() const
    { return resource().get_output_buffer_bindings(); }
    std::unordered_set<fragment_shader_uniform_symbolic_name> const&  get_symbolic_names_of_used_uniforms() const
    { return resource().get_symbolic_names_of_used_uniforms(); }
    std::vector<std::string> const&  get_lines_of_shader_code() const
    { return resource().get_lines_of_shader_code(); }

    template<typename value_type>
    bool  set_uniform_variable(fragment_shader_uniform_symbolic_name const  symbolic_name, value_type const&  value_to_store)
    {
        return set_uniform_variable(uniform_name(symbolic_name), value_to_store);
    }

    bool  set_uniform_variable(std::string const&  variable_name, natural_32_bit const  value_to_store)
    { return resource().set_uniform_variable(variable_name, value_to_store); }
    
    bool  set_uniform_variable(std::string const&  variable_name, float_32_bit const  value_to_store)
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
            async::finalise_load_on_destroy_ptr  finaliser,
            boost::filesystem::path const&  vertex_shader_path,
            boost::filesystem::path const&  fragment_shader_path
            );

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

    void  on_shaders_load_finished(
            async::finalise_load_on_destroy_ptr  finaliser,
            boost::filesystem::path const&  vertex_shader_path,
            boost::filesystem::path const&  fragment_shader_path,
            std::shared_ptr<bool>  visited
            );

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
            boost::filesystem::path const&  vertex_shader_path,
            boost::filesystem::path const&  fragment_shader_path,
            async::key_type const&  key = ""
            )
        : async::resource_accessor<detail::shaders_binding_data>(
            key,
            async::notification_callback_type(),
            vertex_shader_path,
            fragment_shader_path
            )
    {}

    shaders_binding(
            GLuint const  id,
            vertex_shader const  vertex_shader,
            fragment_shader const  fragment_shader,
            async::key_type const&  key = ""
            )
        : async::resource_accessor<detail::shaders_binding_data>(
                key,
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
