#ifndef QTGL_SHADER_HPP_INCLUDED
#   define QTGL_SHADER_HPP_INCLUDED

#   include <qtgl/glapi.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <utility/tensor_math.hpp>
#   include <boost/filesystem/path.hpp>
#   include <istream>
#   include <string>
#   include <vector>
#   include <memory>

namespace qtgl {


struct vertex_program_properties
{
    vertex_program_properties(
            boost::filesystem::path const&  shader_file,
            bool const  assumes_buffer_positions,
            bool const  assumes_buffer_colours,
            bool const  assumes_buffer_normals,
            std::vector<natural_8_bit> const&  indices_of_assumed_buffers_of_texture_coordinates,
            bool const  assumes_uniform_alpha_colour,
            bool const  assumes_uniform_transform_matrix_transposed,
            bool const  builds_buffer_positions,
            bool const  builds_buffer_colours,
            bool const  builds_buffer_normals,
            std::vector<natural_8_bit> const&  indices_of_built_buffers_of_texture_coordinates
            );

    vertex_program_properties(
            boost::filesystem::path const&  shader_file,
            std::vector<std::string> const&  lines_of_shader_code
            );

    boost::filesystem::path const&  shader_file() const noexcept { return m_shader_file; }

    bool  assumes_buffer_positions() const noexcept { return m_assumes_buffer_positions; }
    bool  assumes_buffer_colours() const noexcept { return m_assumes_buffer_colours; }
    bool  assumes_buffer_normals() const noexcept { return m_assumes_buffer_normals; }

    bool  assumes_buffer_texture_coordinates(natural_8_bit const  texcoords_buffer_index) const;
    bool  num_assumed_buffers_of_texture_coordinates() const noexcept { return m_indices_of_assumed_buffers_of_texture_coordinates.size(); }
    std::vector<natural_8_bit> const&  indices_of_assumed_buffers_of_texture_coordinates() const noexcept { return m_indices_of_assumed_buffers_of_texture_coordinates; }

    bool  assumes_uniform_alpha_colour() const noexcept { return m_assumes_uniform_alpha_colour; }
    bool  assumes_uniform_transform_matrix_transposed() const noexcept { return m_assumes_uniform_transform_matrix_transposed; }

    bool  builds_buffer_positions() const noexcept { return m_builds_buffer_positions; }
    bool  builds_buffer_colours() const noexcept { return m_builds_buffer_colours; }
    bool  builds_buffer_normals() const noexcept { return m_builds_buffer_normals; }

    bool  builds_buffer_texture_coordinates(natural_8_bit const  texcoords_buffer_index) const;
    bool  num_built_buffers_of_texture_coordinates() const noexcept { return m_indices_of_built_buffers_of_texture_coordinates.size(); }
    std::vector<natural_8_bit> const&  indices_of_built_buffers_of_texture_coordinates() const noexcept { return m_indices_of_built_buffers_of_texture_coordinates; }

private:
    boost::filesystem::path  m_shader_file;
    bool  m_assumes_buffer_positions;
    bool  m_assumes_buffer_colours;
    bool  m_assumes_buffer_normals;
    std::vector<natural_8_bit>  m_indices_of_assumed_buffers_of_texture_coordinates;
    bool  m_assumes_uniform_alpha_colour;
    bool  m_assumes_uniform_transform_matrix_transposed;
    bool  m_builds_buffer_positions;
    bool  m_builds_buffer_colours;
    bool  m_builds_buffer_normals;
    std::vector<natural_8_bit>  m_indices_of_built_buffers_of_texture_coordinates;
};


bool  operator==(vertex_program_properties const&  props0, vertex_program_properties const&  props1);
inline bool  operator!=(vertex_program_properties const&  props0, vertex_program_properties const&  props1) { return !(props0 == props1); }

size_t  hasher_of_vertex_program_properties(vertex_program_properties const&  props);

using  vertex_program_properties_ptr = std::shared_ptr<vertex_program_properties const>;


}

namespace qtgl {


struct vertex_program;
typedef std::shared_ptr<vertex_program const>  vertex_program_ptr;


struct vertex_program
{
    static vertex_program_ptr  create(std::istream&  source_code, std::string&  error_message);
    static vertex_program_ptr  create(boost::filesystem::path const&  shader_source_file, std::string&  error_message);
    static vertex_program_ptr  create(GLuint const  id, vertex_program_properties_ptr const  properties);

    ~vertex_program();

    GLuint  id() const noexcept { return m_id; }
    vertex_program_properties_ptr  properties() const noexcept { return m_properties; }

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


struct fragment_program;
typedef std::shared_ptr<fragment_program const>  fragment_program_ptr;


struct fragment_program
{
    static fragment_program_ptr  create(std::istream&  source_code, std::string&  error_message);
    static fragment_program_ptr  create(boost::filesystem::path const&  shader_source_file, std::string&  error_message);
    static fragment_program_ptr  create(GLuint const  id);

    ~fragment_program();

    GLuint  id() const { return m_id; }

private:
    fragment_program(GLuint const  id) : m_id(id) {}

    fragment_program(fragment_program const&) = delete;
    fragment_program& operator=(fragment_program const&) = delete;

    GLuint  m_id;
};


inline std::string  shader_uniform_colour_alpha() { return "UNIFORM_COLOUR_ALPHA"; }
inline std::string  shader_uniform_transform_matrix_transposed() { return "UNIFORM_TRANSFORM_MATRIX_TRANSPOSED"; }

void  set_uniform_variable(vertex_program_ptr const  shader_program,
                           std::string const&  variable_name,
                           float_32_bit const  value_to_store);
void  set_uniform_variable(vertex_program_ptr const  shader_program,
                           std::string const&  variable_name,
                           matrix44 const&  value_to_store);


struct shaders_binding;
typedef std::shared_ptr<shaders_binding const>  shaders_binding_ptr;


struct shaders_binding
{
    static shaders_binding_ptr  create(vertex_program_ptr const  vertex_program,
                                       fragment_program_ptr const  fragment_program);

    ~shaders_binding();

    GLuint  id() const { return m_id; }

private:
    shaders_binding(vertex_program_ptr const  vertex_program,
                    fragment_program_ptr const  fragment_program);

    shaders_binding(shaders_binding const&) = delete;
    shaders_binding& operator=(shaders_binding const&) = delete;

    GLuint  m_id;
};


void make_current(shaders_binding_ptr const  shaders_binding);


}

#endif
