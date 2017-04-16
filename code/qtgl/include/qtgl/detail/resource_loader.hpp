#ifndef QTGL_DETAIL_RESOURCE_LOADER_HPP_INCLUDED
#   define QTGL_DETAIL_RESOURCE_LOADER_HPP_INCLUDED

#   include <qtgl/texture.hpp>
#   include <qtgl/shader.hpp>
#   include <qtgl/buffer.hpp>
#   include <qtgl/batch.hpp>
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

    using  texture_props_receiver_fn = std::function<void(boost::filesystem::path const&,   //!< Pathname of a texture file
                                                          texture_properties_ptr,   //!< The loaded data from the texture file
                                                          std::string const&    //!< Error message. Empty string means no error.
                                                          )>;
    void  insert_texture_request(boost::filesystem::path const&  texture_file, texture_props_receiver_fn const&  receiver);

    using  texture_receiver_fn = std::function<void(texture_image_properties&,texture_properties_ptr)>;
    void  insert_texture_request(texture_properties_ptr const  props, texture_receiver_fn const&  receiver);

    using  vertex_program_receiver_fn =
            std::function<void(boost::filesystem::path const&,  //!< Shader file path-name.
                               std::shared_ptr<std::vector<std::string> const>, //!< Lines of the shader's code.
                               std::string const&               //!< Error message. Empty string means no error.
                               )>;
    void  insert_vertex_program_request(boost::filesystem::path const&  shader_file,
                                          vertex_program_receiver_fn const&  receiver);

    using  fragment_program_receiver_fn =
            std::function<void(boost::filesystem::path const&,  //!< Shader file path-name.
                               std::shared_ptr<std::vector<std::string> const>, //!< Lines of the shader's code.
                               std::string const&               //!< Error message. Empty string means no error.
                               )>;
    void  insert_fragment_program_request(boost::filesystem::path const&  shader_file,
                                          fragment_program_receiver_fn const&  receiver);


    using  buffer_receiver_fn =
            std::function<void(buffer_properties_ptr,   //!< Properties of the buffer
                               std::shared_ptr<std::vector<natural_8_bit> const>,   //!< The loaded data of the buffer
                               boost::filesystem::path const&,
                               std::string const&               //!< Error message. Empty string means no error.
                               )>;
    void  insert_buffer_request(boost::filesystem::path const&  buffer_file, buffer_receiver_fn const&  receiver);

    using  batch_receiver_fn =
            std::function<void(boost::filesystem::path const&,  //!< Batch file path-name.
                               std::shared_ptr<batch const>,    //!< The loaded data of the batch
                               std::string const&               //!< Error message. Empty string means no error.
                               )>;
    void  insert_batch_request(boost::filesystem::path const&  batch_file, batch_receiver_fn const&  receiver);

private:
    resource_loader();

    resource_loader(resource_loader const&) = delete;
    resource_loader& operator=(resource_loader const&) = delete;

    bool  fetch_texture_request(boost::filesystem::path&  textute_file, texture_props_receiver_fn&  output_receiver);
    bool  fetch_texture_request(texture_properties_ptr&  output_props, texture_receiver_fn&  output_receiver);
    bool  fetch_vertex_program_request(boost::filesystem::path&  shader_file,
                                       vertex_program_receiver_fn&  output_receiver);
    bool  fetch_fragment_program_request(boost::filesystem::path&  shader_file,
                                         fragment_program_receiver_fn&  output_receiver);
    bool  fetch_buffer_request(boost::filesystem::path&  shader_file, buffer_receiver_fn&  output_receiver);
    bool  fetch_batch_request(boost::filesystem::path&  batch_file, batch_receiver_fn&  output_receiver);

    void  start_worker_if_not_running();
    void  worker();

    std::thread  m_worker_thread;
    std::atomic<bool>  m_worker_finished;
    mutable std::mutex  m_mutex;

    std::deque< std::pair<boost::filesystem::path,texture_props_receiver_fn> >  m_texture_props_requests;
    std::deque< std::pair<texture_properties_ptr,texture_receiver_fn> >  m_texture_requests;
    std::deque< std::pair<boost::filesystem::path,vertex_program_receiver_fn> >  m_vertex_program_requests;
    std::deque< std::pair<boost::filesystem::path,fragment_program_receiver_fn> >  m_fragment_program_requests;
    std::deque< std::pair<boost::filesystem::path,buffer_receiver_fn> >  m_buffer_requests;
    std::deque< std::pair<boost::filesystem::path,batch_receiver_fn> >  m_batch_requests;
};


}}

#endif
