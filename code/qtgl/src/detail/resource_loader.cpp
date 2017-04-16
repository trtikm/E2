#include <qtgl/detail/resource_loader.hpp>
#include <qtgl/detail/texture_cache.hpp>
#include <qtgl/detail/buffer_cache.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <tuple>
#include <algorithm>
#include <functional>

namespace qtgl { namespace detail { namespace {


template <typename T>
struct identity { typedef T type; };

template<typename container_type, typename key_type>
bool  contains(container_type const&  C, key_type const&  K,
               typename identity<std::function<bool(key_type const&, key_type const&)> >::type const&  equal_to
                        = std::equal_to<key_type>())
{
    for (auto  it = C.cbegin(); it != C.cend(); ++it)
        if (equal_to(it->first,K))
            return true;
    return false;
}


}}}

namespace qtgl { namespace detail {


resource_loader&  resource_loader::instance()
{
    static resource_loader  rl;
    return rl;
}

resource_loader::resource_loader()
    : m_worker_thread()
    , m_worker_finished(true)
    , m_mutex()
    , m_texture_props_requests()
    , m_texture_requests()
    , m_vertex_program_requests()
    , m_fragment_program_requests()
    , m_buffer_requests()
    , m_batch_requests()
{}

void  resource_loader::start_worker_if_not_running()
{
    TMPROF_BLOCK();

    if (m_worker_finished)
    {
        m_worker_finished = false;
        if (m_worker_thread.joinable())
            m_worker_thread.join();
        m_worker_thread = std::thread(&resource_loader::worker,this);
    }
}

void  resource_loader::clear()
{
    TMPROF_BLOCK();
    {
        std::lock_guard<std::mutex> const  lock(m_mutex);
        m_texture_props_requests.clear();
        m_texture_requests.clear();
        m_vertex_program_requests.clear();
        m_fragment_program_requests.clear();
        m_buffer_requests.clear();
        m_batch_requests.clear();
    }
    if (m_worker_thread.joinable())
        m_worker_thread.join();
}

void  resource_loader::insert_texture_request(boost::filesystem::path const&  texture_file, texture_props_receiver_fn const&  receiver)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    if (qtgl::detail::contains(m_texture_props_requests, texture_file))
        return;
    m_texture_props_requests.push_back({texture_file,receiver});
    start_worker_if_not_running();
}

void  resource_loader::insert_texture_request(texture_properties_ptr const  props, texture_receiver_fn const&  receiver)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    if (qtgl::detail::contains(m_texture_requests, props,
                               [](texture_properties_ptr const  a, texture_properties_ptr const  b) {
                                    return *a == *b;
                                    }))
        return;
    m_texture_requests.push_back({props,receiver});
    start_worker_if_not_running();
}

void  resource_loader::insert_vertex_program_request(boost::filesystem::path const&  shader_file,
                                                     vertex_program_receiver_fn const&  receiver)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    if (qtgl::detail::contains(m_vertex_program_requests,shader_file))
        return;
    m_vertex_program_requests.push_back({shader_file,receiver});
    start_worker_if_not_running();
}

void  resource_loader::insert_fragment_program_request(boost::filesystem::path const&  shader_file,
                                                       fragment_program_receiver_fn const&  receiver)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    if (qtgl::detail::contains(m_fragment_program_requests, shader_file))
        return;
    m_fragment_program_requests.push_back({shader_file,receiver});
    start_worker_if_not_running();
}

void  resource_loader::insert_buffer_request(boost::filesystem::path const&  buffer_file, buffer_receiver_fn const&  receiver)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    if (qtgl::detail::contains(m_buffer_requests, buffer_file))
        return;
    m_buffer_requests.push_back({buffer_file,receiver});
    start_worker_if_not_running();
}

void  resource_loader::insert_batch_request(boost::filesystem::path const&  batch_file, batch_receiver_fn const&  receiver)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    if (qtgl::detail::contains(m_batch_requests, batch_file))
        return;
    m_batch_requests.push_back({batch_file,receiver});
    start_worker_if_not_running();
}

bool  resource_loader::fetch_texture_request(boost::filesystem::path&  texture_file, texture_props_receiver_fn&  output_receiver)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    if (m_texture_props_requests.empty())
        return false;
    std::tie(texture_file,output_receiver) = m_texture_props_requests.front();
    m_texture_props_requests.pop_front();
    return true;
}

bool  resource_loader::fetch_texture_request(texture_properties_ptr&  output_props, texture_receiver_fn&  output_receiver)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    if (m_texture_requests.empty())
        return false;
    std::tie(output_props,output_receiver) = m_texture_requests.front();
    m_texture_requests.pop_front();
    return true;
}

bool  resource_loader::fetch_vertex_program_request(boost::filesystem::path&  shader_file,
                                                    vertex_program_receiver_fn&  output_receiver)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    if (m_vertex_program_requests.empty())
        return false;
    std::tie(shader_file,output_receiver) = m_vertex_program_requests.front();
    m_vertex_program_requests.pop_front();
    return true;
}

bool  resource_loader::fetch_fragment_program_request(boost::filesystem::path&  shader_file,
                                                      fragment_program_receiver_fn&  output_receiver)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    if (m_fragment_program_requests.empty())
        return false;
    std::tie(shader_file,output_receiver) = m_fragment_program_requests.front();
    m_fragment_program_requests.pop_front();
    return true;
}

bool  resource_loader::fetch_buffer_request(boost::filesystem::path&  buffer_file, buffer_receiver_fn&  output_receiver)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    if (m_buffer_requests.empty())
        return false;
    std::tie(buffer_file,output_receiver) = m_buffer_requests.front();
    m_buffer_requests.pop_front();
    return true;
}

bool  resource_loader::fetch_batch_request(boost::filesystem::path&  batch_file, batch_receiver_fn&  output_receiver)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    if (m_batch_requests.empty())
        return false;
    std::tie(batch_file,output_receiver) = m_batch_requests.front();
    m_batch_requests.pop_front();
    return true;
}

void  resource_loader::worker()
{
    TMPROF_BLOCK();

    while (true)
    {
        bool  done = true;

        // Loading batches
        for (int i = 0; i < 1; ++i)
        {
            boost::filesystem::path  batch_file;
            batch_receiver_fn  receiver;
            if (fetch_batch_request(batch_file,receiver))
            {
                std::string  error_message;
                std::shared_ptr<batch const> const  props = load_batch_file(batch_file,error_message);
                if (props == nullptr || !error_message.empty())
                    LOG(error,"Load of batch file '" << batch_file << "' has failed. " << error_message);
                receiver(batch_file,props,error_message);
                done = false;
            }
        }

        // Loading buffers
        for (int i = 0; i < 1; ++i)
        {
            boost::filesystem::path  buffer_file;
            buffer_receiver_fn  receiver;
            if (fetch_buffer_request(buffer_file,receiver))
            {
                std::shared_ptr< std::vector<natural_8_bit> > const  data = std::make_shared< std::vector<natural_8_bit> >();
                std::string  error_message;
                buffer_properties_ptr const  props = load_buffer_file(buffer_file,*data,error_message);
                if (props == nullptr || !error_message.empty())
                    LOG(error,"Load of buffer file '" << buffer_file << "' has failed. " << error_message);
                receiver(props,data,buffer_file,error_message);
                done = false;
            }
        }

        // Loading vertex programs
        for (int i = 0; i < 1; ++i)
        {
            boost::filesystem::path  shader_file;
            vertex_program_receiver_fn  receiver;
            if (!fetch_vertex_program_request(shader_file,receiver))
                break;
            std::shared_ptr<std::vector<std::string> >  lines = std::make_shared< std::vector<std::string> >();
            std::string const  error_message = load_vertex_program_file(shader_file,*lines);
            if (!error_message.empty())
                LOG(error,"Load of vertex shader file '" << shader_file << "' has failed. " << error_message);
            receiver(shader_file,lines,error_message);
            done = false;
        }

        // Loading fragment programs
        for (int i = 0; i < 1; ++i)
        {
            boost::filesystem::path  shader_file;
            fragment_program_receiver_fn  receiver;
            if (!fetch_fragment_program_request(shader_file,receiver))
                break;
            std::shared_ptr<std::vector<std::string> >  lines = std::make_shared< std::vector<std::string> >();
            std::string const  error_message = load_fragment_program_file(shader_file,*lines);
            if (!error_message.empty())
                LOG(error,"Load of fragment shader file '" << shader_file << "' has failed. " << error_message);
            receiver(shader_file,lines,error_message);
            done = false;
        }

        // Loading texture files
        for (int i = 0; i < 1; ++i)
        {
            boost::filesystem::path  texture_file;
            texture_props_receiver_fn  receiver;
            if (fetch_texture_request(texture_file,receiver))
            {
                std::string  error_message;
                texture_properties_ptr const  props = load_texture_file(texture_file,error_message);
                if (props == nullptr || !error_message.empty())
                    LOG(error,"Load of texture file '" << texture_file << "' has failed. " << error_message);
                receiver(texture_file,props,error_message);
                done = false;
            }
        }

        // Loading textures
        for (int i = 0; i < 1; ++i)
        {
            texture_properties_ptr  props;
            texture_receiver_fn  receiver;
            if (fetch_texture_request(props,receiver))
            {
                receiver(load_texture_image_file(props->image_file()),props);
                done = false;
            }
        }

        if (done)
            break;
    }

    m_worker_finished = true;
}


}}
