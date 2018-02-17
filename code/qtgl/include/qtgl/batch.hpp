#ifndef QTGL_BATCH_HPP_INCLUDED
#   define QTGL_BATCH_HPP_INCLUDED

#   include <qtgl/draw_state.hpp>
#   include <qtgl/buffer.hpp>
#   include <qtgl/shader.hpp>
#   include <qtgl/texture.hpp>
#   include <qtgl/modelspace.hpp>
#   include <boost/filesystem/path.hpp>
#   include <unordered_set>
#   include <memory>
#   include <utility>

namespace qtgl {


struct batch
{
    static std::shared_ptr<batch const>  create(boost::filesystem::path const&  path);

    static std::shared_ptr<batch const>  create(
            boost::filesystem::path const&  path,
            buffers_binding_ptr const  buffers_binding,
            shaders_binding_ptr const  shaders_binding,
            textures_binding_ptr const  textures_binding,
            draw_state_ptr const  draw_state,
            modelspace_ptr const modelspace
            );

    batch(boost::filesystem::path const&  path);

    batch(boost::filesystem::path const&  path,
          buffers_binding_ptr const  buffers_binding,
          shaders_binding_ptr const  shaders_binding,
          textures_binding_ptr const  textures_binding,
          draw_state_ptr const  draw_state,
          modelspace_ptr const modelspace
          );

    boost::filesystem::path const&  path() const noexcept { return m_path; }

    buffers_binding_ptr  buffers_binding() const;
    shaders_binding_ptr  shaders_binding() const;
    textures_binding_ptr  textures_binding() const;
    draw_state_ptr  draw_state() const;
    modelspace_ptr  get_modelspace() const;

    std::unordered_set<vertex_shader_uniform_symbolic_name> const&  symbolic_names_of_used_uniforms() const;
    natural_32_bit  num_matrices_per_vertex() const;

private:

    boost::filesystem::path  m_path;
    mutable buffers_binding_ptr  m_buffers_binding;
    mutable shaders_binding_ptr  m_shaders_binding;
    mutable textures_binding_ptr  m_textures_binding;
    mutable draw_state_ptr  m_draw_state;
    mutable modelspace_ptr  m_modelspace;

    static std::unordered_set<vertex_shader_uniform_symbolic_name>  s_empty_uniforms;
};


using  batch_ptr = std::shared_ptr<batch const>;

void  insert_load_request(batch const&  batch_ref);

batch_ptr  load_batch_file(boost::filesystem::path const&  batch_file, std::string&  error_message);


bool  make_current(batch const&  binding);
bool  make_current(batch const&  binding, draw_state const&  previous_state);
bool  make_current(batch const&  binding, draw_state_ptr const  previous_state);


std::pair<bool, //!< Is inside the cache
          bool  //!< Is inside failed
    >  get_batch_chache_state(boost::filesystem::path const&  batch_file);

void  get_cached_batches(std::vector< std::pair<boost::filesystem::path,batch_ptr> >&  output);
void  get_failed_batches(std::vector< std::pair<boost::filesystem::path,std::string> >&  output);


}

#endif
