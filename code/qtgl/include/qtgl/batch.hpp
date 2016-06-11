#ifndef QTGL_BATCH_HPP_INCLUDED
#   define QTGL_BATCH_HPP_INCLUDED

#   include <qtgl/buffer.hpp>
#   include <qtgl/shader.hpp>
#   include <qtgl/texture.hpp>
#   include <utility/tensor_math.hpp>
#   include <boost/filesystem/path.hpp>
#   include <unordered_set>
#   include <functional>

namespace qtgl {


struct batch
{
    using  uniform_initialiser = std::function<bool(uniform_variable_accessor_type const&,
                                                    vertex_shader_uniform_symbolic_name)>;

    batch(boost::filesystem::path const&  path);

    boost::filesystem::path const&  path() const noexcept { return m_path; }

    buffers_binding_ptr  buffers_binding() const noexcept { return m_buffers_binding; }
    shaders_binding_ptr  shaders_binding() const noexcept { return m_shaders_binding; }
    textures_binding_ptr  textures_binding() const noexcept { return m_textures_binding; }

    std::unordered_set<vertex_shader_uniform_symbolic_name> const&  symbolic_names_of_used_uniforms() const;

private:

    boost::filesystem::path  m_path;
    buffers_binding_ptr  m_buffers_binding;
    shaders_binding_ptr  m_shaders_binding;
    textures_binding_ptr  m_textures_binding;

    static std::unordered_set<vertex_shader_uniform_symbolic_name>  s_empty_uniforms;
};


bool  make_current(batch const&  binding,
                   batch::uniform_initialiser const& initialiser
                        = [](uniform_variable_accessor_type const&,vertex_shader_uniform_symbolic_name) { return true; }
                   );


bool  make_current(batch const&  binding,
                   matrix44 const& transform_matrix_transposed,
                   batch::uniform_initialiser const& initialiser
                        = [](uniform_variable_accessor_type const&,vertex_shader_uniform_symbolic_name) { return true; }
                   );


}

#endif
