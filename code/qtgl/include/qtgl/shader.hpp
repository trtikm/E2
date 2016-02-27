#ifndef QTGL_SHADER_HPP_INCLUDED
#   define QTGL_SHADER_HPP_INCLUDED

#   include <qtgl/glapi.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <utility/tensor_math.hpp>
#   include <boost/filesystem/path.hpp>
#   include <istream>
#   include <string>
#   include <memory>

namespace qtgl {


struct vertex_program;
typedef std::shared_ptr<vertex_program const>  vertex_program_ptr;


struct vertex_program
{
    static vertex_program_ptr  create(std::istream&  source_code, std::string&  error_message);
    static vertex_program_ptr  create(boost::filesystem::path const&  shader_source_file, std::string&  error_message);
    static vertex_program_ptr  create(GLuint const  id);

    ~vertex_program();

    GLuint  id() const { return m_id; }

private:
    vertex_program(GLuint const  id) : m_id(id) {}

    vertex_program(vertex_program const&) = delete;
    vertex_program& operator=(vertex_program const&) = delete;

    GLuint  m_id;
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
