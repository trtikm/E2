#ifndef QTGL_DETAIL_RESOURCE_LOADER_HPP_INCLUDED
#   define QTGL_DETAIL_RESOURCE_LOADER_HPP_INCLUDED

#   include <qtgl/texture.hpp>
#   include <qtgl/shader.hpp>
#   include <boost/filesystem/path.hpp>
#   include <functional>
#   include <deque>
#   include <vector>
#   include <utility>
#   include <mutex>
#   include <atomic>

namespace qtgl { namespace detail {


struct resource_loader
{
    static  resource_loader&  instance();

    void clear();

    using  texture_receiver_fn = std::function<void(texture_image_properties&,texture_properties_ptr)>;
    void  insert(texture_properties_ptr const  props, texture_receiver_fn const&  receiver);

    using  vertex_program_receiver_fn =
            std::function<void(boost::filesystem::path const&,  //!< Shader file path-name.
                               std::shared_ptr<std::vector<std::string> const>, //!< Lines of the shader's code.
                               std::string const&               //!< Error message. Empty string means no error.
                               )>;
    void  insert(boost::filesystem::path const&  shader_file, vertex_program_receiver_fn const&  receiver);

private:
    resource_loader();

    resource_loader(resource_loader const&) = delete;
    resource_loader& operator=(resource_loader const&) = delete;

    bool  fetch(texture_properties_ptr&  output_props, texture_receiver_fn&  output_receiver);
    bool  fetch(boost::filesystem::path&  shader_file, vertex_program_receiver_fn&  output_receiver);

    void  start_worker_if_not_running();
    void  worker();

    std::thread  m_worker_thread;
    std::atomic<bool>  m_worker_finished;
    mutable std::mutex  m_mutex;

    std::deque< std::pair<texture_properties_ptr,texture_receiver_fn> >  m_texture_requests;
    std::deque< std::pair<boost::filesystem::path,vertex_program_receiver_fn> >  m_vertex_program_requests;
};


}}

#endif
