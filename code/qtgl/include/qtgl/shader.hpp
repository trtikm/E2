#ifndef QTGL_SHADER_HPP_INCLUDED
#   define QTGL_SHADER_HPP_INCLUDED

#   include <qtgl/shader_data_bindings.hpp>
#   include <qtgl/glapi.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <angeo/tensor_math.hpp>
#   include <boost/filesystem/path.hpp>
#   include <unordered_set>
#   include <istream>
#   include <string>
#   include <vector>
#   include <memory>

namespace qtgl {


struct vertex_program_properties;
using  vertex_program_properties_ptr = std::shared_ptr<vertex_program_properties const>;


struct vertex_program_properties
{
    static vertex_program_properties_ptr  create(
            std::unordered_set<vertex_shader_input_buffer_binding_location> const&  input_buffer_bindings,
            std::unordered_set<vertex_shader_output_buffer_binding_location> const&  output_buffer_bindings,
            std::unordered_set<vertex_shader_uniform_symbolic_name> const&  symbolic_names_of_used_uniforms
            );

    static vertex_program_properties_ptr  create(std::vector<std::string> const&  lines_of_shader_code);

    vertex_program_properties(
            std::unordered_set<vertex_shader_input_buffer_binding_location> const&  input_buffer_bindings,
            std::unordered_set<vertex_shader_output_buffer_binding_location> const&  output_buffer_bindings,
            std::unordered_set<vertex_shader_uniform_symbolic_name> const&  symbolic_names_of_used_uniforms
            );

    vertex_program_properties(std::vector<std::string> const&  lines_of_shader_code);

    std::unordered_set<vertex_shader_input_buffer_binding_location> const& input_buffer_bindings() const noexcept
    { return m_input_buffer_bindings; }
    std::unordered_set<vertex_shader_output_buffer_binding_location> const&  output_buffer_bindings() const noexcept
    { return m_output_buffer_bindings; }

    std::unordered_set<vertex_shader_uniform_symbolic_name> const&  symbolic_names_of_used_uniforms() const noexcept
    { return m_symbolic_names_of_used_uniforms; }

private:
    std::unordered_set<vertex_shader_input_buffer_binding_location>  m_input_buffer_bindings;
    std::unordered_set<vertex_shader_output_buffer_binding_location>  m_output_buffer_bindings;
    std::unordered_set<vertex_shader_uniform_symbolic_name>  m_symbolic_names_of_used_uniforms;
};


bool  operator==(vertex_program_properties const&  props0, vertex_program_properties const&  props1);
inline bool  operator!=(vertex_program_properties const&  props0, vertex_program_properties const&  props1)
{ return !(props0 == props1); }

size_t  hasher_of_vertex_program_properties(vertex_program_properties const&  props);


}

namespace qtgl {


using  uniform_variable_accessor_type = std::pair<GLuint,vertex_program_properties_ptr>;


}

namespace qtgl {

struct vertex_program;
typedef std::shared_ptr<vertex_program const>  vertex_program_ptr;


struct vertex_program
{
    static vertex_program_ptr  create(std::istream&  source_code, std::string&  error_message);
    static vertex_program_ptr  create(std::istream&  source_code,
                                      vertex_program_properties const&  properties,
                                      std::string&  error_message);
    static vertex_program_ptr  create(boost::filesystem::path const&  shader_source_file, std::string&  error_message);
    static vertex_program_ptr  create(std::vector<std::string> const& source_code_lines, std::string&  error_message);
    static vertex_program_ptr  create(std::vector<std::string> const& source_code_lines,
                                      vertex_program_properties const&  properties,
                                      std::string&  error_message);
    static vertex_program_ptr  create(GLuint const  id, vertex_program_properties_ptr const  properties);

    ~vertex_program();

    GLuint  id() const noexcept { return m_id; }
    vertex_program_properties_ptr  properties() const noexcept { return m_properties; }

    uniform_variable_accessor_type  uniform_variable_accessor() const noexcept { return std::make_pair(id(),properties()); }

private:
    vertex_program(GLuint const  id, vertex_program_properties_ptr const  properties)
        : m_id(id)
        , m_properties(properties)
    {}

    vertex_program(vertex_program const&) = delete;
    vertex_program& operator=(vertex_program const&) = delete;

    GLuint  m_id;
    vertex_program_properties_ptr  m_properties;
};


template<typename value_type>
bool  set_uniform_variable(uniform_variable_accessor_type const&  accessor,
                           vertex_shader_uniform_symbolic_name const  symbolic_name,
                           value_type const&  value_to_store)
{
    return set_uniform_variable(accessor,uniform_name(symbolic_name),value_to_store);
}

bool  set_uniform_variable(uniform_variable_accessor_type const&  accessor,
                           std::string const&  variable_name,
                           float_32_bit const  value_to_store);
bool  set_uniform_variable(uniform_variable_accessor_type const&  accessor,
                           std::string const&  variable_name,
                           vector4 const&  value_to_store);
bool  set_uniform_variable(uniform_variable_accessor_type const&  accessor,
                           std::string const&  variable_name,
                           matrix44 const&  value_to_store);


std::string  load_vertex_program_file(boost::filesystem::path const&  shader_file,
                                      std::vector<std::string>& output_lines);

void  insert_vertex_program_load_request(boost::filesystem::path const&  shader_file);
bool  insert_vertex_program_load_request(vertex_program_properties_ptr const  props);
inline bool  insert_vertex_program_load_request(vertex_program_properties const&  props)
{ return insert_vertex_program_load_request(std::make_shared<vertex_program_properties>(props)); }

std::weak_ptr<vertex_program const>  find_vertex_program(boost::filesystem::path const&  shader_file);
std::weak_ptr<vertex_program const>  find_vertex_program(vertex_program_properties_ptr const  props);

inline std::weak_ptr<vertex_program const>  find_vertex_program(vertex_program_properties const&  props)
{ return find_vertex_program(std::make_shared<vertex_program_properties>(props)); }

bool  associate_vertex_program_properties_with_shader_file(
        vertex_program_properties_ptr const  props, boost::filesystem::path const&  shader_file
        );
boost::filesystem::path  find_vertex_program_file(vertex_program_properties_ptr const  props);
inline boost::filesystem::path  find_vertex_program_file(vertex_program_properties const&  props)
{ return find_vertex_program_file(std::make_shared<vertex_program_properties>(props)); }


void  get_properties_of_cached_vertex_programs(
        std::vector< std::pair<boost::filesystem::path,vertex_program_properties_ptr> >&  output
        );
void  get_properties_of_failed_vertex_programs(
        std::vector< std::pair<boost::filesystem::path,std::string> >&  output
        );


}

namespace qtgl {


struct fragment_program_properties;
using  fragment_program_properties_ptr = std::shared_ptr<fragment_program_properties const>;


struct fragment_program_properties
{
    static fragment_program_properties_ptr  create(
            std::unordered_set<fragment_shader_input_buffer_binding_location> const&  input_buffer_bindings,
            std::unordered_set<fragment_shader_output_buffer_binding_location> const&  output_buffer_bindings,
            std::unordered_set<fragment_shader_texture_sampler_binding> const&  texture_sampler_bindings
            );

    static fragment_program_properties_ptr  create(std::vector<std::string> const&  lines_of_shader_code);

    fragment_program_properties(
            std::unordered_set<fragment_shader_input_buffer_binding_location> const&  input_buffer_bindings,
            std::unordered_set<fragment_shader_output_buffer_binding_location> const&  output_buffer_bindings,
            std::unordered_set<fragment_shader_texture_sampler_binding> const&  texture_sampler_bindings
            );

    fragment_program_properties(std::vector<std::string> const&  lines_of_shader_code);

    std::unordered_set<fragment_shader_input_buffer_binding_location> const& input_buffer_bindings() const noexcept
    { return m_input_buffer_bindings; }
    std::unordered_set<fragment_shader_output_buffer_binding_location> const&  output_buffer_bindings() const noexcept
    { return m_output_buffer_bindings; }

    std::unordered_set<fragment_shader_texture_sampler_binding> const&  texture_sampler_bindings() const noexcept
    { return m_texture_sampler_bindings; }

private:
    std::unordered_set<fragment_shader_input_buffer_binding_location>  m_input_buffer_bindings;
    std::unordered_set<fragment_shader_output_buffer_binding_location>  m_output_buffer_bindings;
    std::unordered_set<fragment_shader_texture_sampler_binding>  m_texture_sampler_bindings;
};


bool  operator==(fragment_program_properties const&  props0, fragment_program_properties const&  props1);
inline bool  operator!=(fragment_program_properties const&  props0, fragment_program_properties const&  props1)
{ return !(props0 == props1); }

size_t  hasher_of_fragment_program_properties(fragment_program_properties const&  props);


}

namespace qtgl {


struct fragment_program;
typedef std::shared_ptr<fragment_program const>  fragment_program_ptr;


struct fragment_program
{
    static fragment_program_ptr  create(std::istream&  source_code, std::string&  error_message);
    static fragment_program_ptr  create(std::istream&  source_code,
                                        fragment_program_properties const&  properties,
                                        std::string&  error_message);
    static fragment_program_ptr  create(boost::filesystem::path const&  shader_source_file, std::string&  error_message);
    static fragment_program_ptr  create(std::vector<std::string> const& source_code_lines, std::string&  error_message);
    static fragment_program_ptr  create(std::vector<std::string> const& source_code_lines,
                                        fragment_program_properties const&  properties,
                                        std::string&  error_message);
    static fragment_program_ptr  create(GLuint const  id, fragment_program_properties_ptr const  properties);

    ~fragment_program();

    GLuint  id() const { return m_id; }
    fragment_program_properties_ptr  properties() const noexcept { return m_properties; }

private:
    fragment_program(GLuint const  id, fragment_program_properties_ptr const  properties)
        : m_id(id)
        , m_properties(properties)
    {}

    fragment_program(fragment_program const&) = delete;
    fragment_program& operator=(fragment_program const&) = delete;

    GLuint  m_id;
    fragment_program_properties_ptr  m_properties;
};


std::string  load_fragment_program_file(boost::filesystem::path const&  shader_source_file,
                                        std::vector<std::string>& output_lines);

void  insert_fragment_program_load_request(boost::filesystem::path const&  shader_file);
bool  insert_fragment_program_load_request(fragment_program_properties_ptr const  props);
inline bool  insert_fragment_program_load_request(fragment_program_properties const&  props)
{ return insert_fragment_program_load_request(std::make_shared<fragment_program_properties>(props)); }

std::weak_ptr<fragment_program const>  find_fragment_program(boost::filesystem::path const&  shader_file);
std::weak_ptr<fragment_program const>  find_fragment_program(fragment_program_properties_ptr const  props);

inline std::weak_ptr<fragment_program const>  find_fragment_program(fragment_program_properties const&  props)
{ return find_fragment_program(std::make_shared<fragment_program_properties>(props)); }

bool  associate_fragment_program_properties_with_shader_file(
        fragment_program_properties_ptr const  props, boost::filesystem::path const&  shader_file
        );
boost::filesystem::path  find_fragment_program_file(fragment_program_properties_ptr const  props);
inline boost::filesystem::path  find_fragment_program_file(fragment_program_properties const&  props)
{ return find_fragment_program_file(std::make_shared<fragment_program_properties>(props)); }


void  get_properties_of_cached_fragment_programs(
        std::vector< std::pair<boost::filesystem::path,fragment_program_properties_ptr> >&  output
        );
void  get_properties_of_failed_fragment_programs(
        std::vector< std::pair<boost::filesystem::path,std::string> >&  output
        );


}

namespace qtgl {


struct shaders_binding;
typedef std::shared_ptr<shaders_binding const>  shaders_binding_ptr;


struct shaders_binding
{
    static shaders_binding_ptr  create(boost::filesystem::path const&  vertex_shader_file,
                                       boost::filesystem::path const&  fragment_shader_file);

    static shaders_binding_ptr  create(boost::filesystem::path const&  vertex_shader_file,
                                       fragment_program_properties_ptr const  fragment_program_props);

    static shaders_binding_ptr  create(vertex_program_properties_ptr const  vertex_program_props,
                                       boost::filesystem::path const&  fragment_shader_file);

    static shaders_binding_ptr  create(vertex_program_properties_ptr const  vertex_program_props,
                                       fragment_program_properties_ptr const  fragment_program_props);

    static shaders_binding_ptr  create(vertex_program_ptr const  vertex_program,
                                       fragment_program_ptr const  fragment_program);

    boost::filesystem::path const&  vertex_shader_file() const noexcept { return m_vertex_shader_file; }
    boost::filesystem::path const&  fragment_shader_file() const noexcept { return m_fragment_shader_file; }
    vertex_program_properties_ptr  vertex_program_props() const noexcept { return m_vertex_program_props; }
    fragment_program_properties_ptr  fragment_program_props() const noexcept { return m_fragment_program_props; }
    vertex_program_ptr  vertex_program() const noexcept { return m_vertex_program; }
    fragment_program_ptr  fragment_program() const noexcept { return m_fragment_program; }

    GLuint  id() const noexcept { return m_binding_data->id(); }
    GLuint  vertex_program_id() const noexcept { return m_binding_data->vertex_program_id(); }
    GLuint  fragment_program_id() const noexcept { return m_binding_data->fragment_program_id(); }
    vertex_program_properties_ptr  binding_vertex_program_props() const noexcept { return m_binding_data->vertex_program_props(); }
    fragment_program_properties_ptr  binding_fragment_program_props() const noexcept { return m_binding_data->fragment_program_props(); }

    uniform_variable_accessor_type  uniform_variable_accessor() const noexcept
    { return std::make_pair(vertex_program_id(), m_binding_data->vertex_program_props()); }

    bool  make_current() const;

private:
    shaders_binding(boost::filesystem::path const&  vertex_shader_file,
                    boost::filesystem::path const&  fragment_shader_file,
                    vertex_program_properties_ptr const  vertex_program_props,
                    fragment_program_properties_ptr const  fragment_program_props);
    shaders_binding(vertex_program_ptr const  vertex_program,
                    fragment_program_ptr const  fragment_program);

    shaders_binding(shaders_binding const&) = delete;
    shaders_binding& operator=(shaders_binding const&) = delete;

    struct  binding_data_type
    {
        binding_data_type()
            : m_id(0U)
            , m_vertex_program_id(0U)
            , m_fragment_program_id(0U)
            , m_vertex_program_props()
            , m_fragment_program_props()
        {}

        ~binding_data_type()
        {
            destroy_ID();
        }

        GLuint  id() const noexcept { return m_id; }
        GLuint  vertex_program_id() const noexcept { return m_vertex_program_id; }
        GLuint  fragment_program_id() const noexcept { return m_fragment_program_id; }

        vertex_program_properties_ptr  vertex_program_props() const noexcept { return m_vertex_program_props; }
        fragment_program_properties_ptr  fragment_program_props() const noexcept { return m_fragment_program_props; }

        bool  reset(vertex_program_ptr const  vertex_program, fragment_program_ptr const  fragment_program);

    private:
        void  destroy_ID();

        GLuint  m_id;
        GLuint  m_vertex_program_id;
        GLuint  m_fragment_program_id;
        vertex_program_properties_ptr  m_vertex_program_props;
        fragment_program_properties_ptr  m_fragment_program_props;
    };

    using  binding_data_ptr = std::unique_ptr<binding_data_type>;

    boost::filesystem::path  m_vertex_shader_file;
    boost::filesystem::path  m_fragment_shader_file;
    vertex_program_properties_ptr  m_vertex_program_props;
    fragment_program_properties_ptr  m_fragment_program_props;
    vertex_program_ptr  m_vertex_program;
    fragment_program_ptr  m_fragment_program;
    binding_data_ptr  m_binding_data;
};


void  insert_load_request(shaders_binding const&  binding);

inline bool make_current(shaders_binding const&  binding)
{ return binding.make_current(); }


}

#endif
