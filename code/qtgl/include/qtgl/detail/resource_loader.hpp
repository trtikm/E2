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

    bool  fetch_vertex_program_request(boost::filesystem::path&  shader_file,
                                       vertex_program_receiver_fn&  output_receiver);
    bool  fetch_fragment_program_request(boost::filesystem::path&  shader_file,
                                         fragment_program_receiver_fn&  output_receiver);
    bool  fetch_batch_request(boost::filesystem::path&  batch_file, batch_receiver_fn&  output_receiver);

    void  start_worker_if_not_running();
    void  worker();

    std::thread  m_worker_thread;
    std::atomic<bool>  m_worker_finished;
    mutable std::mutex  m_mutex;

    std::deque< std::pair<boost::filesystem::path,vertex_program_receiver_fn> >  m_vertex_program_requests;
    std::deque< std::pair<boost::filesystem::path,fragment_program_receiver_fn> >  m_fragment_program_requests;
    std::deque< std::pair<boost::filesystem::path,batch_receiver_fn> >  m_batch_requests;
};


}}

#endif
