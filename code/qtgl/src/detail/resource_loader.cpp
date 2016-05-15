#include <qtgl/detail/resource_loader.hpp>
#include <qtgl/detail/texture_cache.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <tuple>

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
    , m_texture_requests()
    , m_vertex_program_requests()
    , m_fragment_program_requests()
{}

void  resource_loader::start_worker_if_not_running()
{
    TMPROF_BLOCK();

    if (m_worker_finished)
    {
        m_worker_finished = false;
        m_worker_thread = std::thread(&resource_loader::worker,this);
    }
}

void  resource_loader::clear()
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    m_texture_requests.clear();
    m_vertex_program_requests.clear();
    m_fragment_program_requests.clear();
    if (m_worker_thread.joinable())
        m_worker_thread.join();
}

void  resource_loader::insert_texture_request(texture_properties_ptr const  props, texture_receiver_fn const&  receiver)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    m_texture_requests.push_back({props,receiver});
    start_worker_if_not_running();
}

void  resource_loader::insert_vertex_program_request(boost::filesystem::path const&  shader_file,
                                                     vertex_program_receiver_fn const&  receiver)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    m_vertex_program_requests.push_back({shader_file,receiver});
    start_worker_if_not_running();
}

void  resource_loader::insert_fragment_program_request(boost::filesystem::path const&  shader_file,
                                                       fragment_program_receiver_fn const&  receiver)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> const  lock(m_mutex);
    m_fragment_program_requests.push_back({shader_file,receiver});
    start_worker_if_not_running();
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

void  resource_loader::worker()
{
    TMPROF_BLOCK();

    while (true)
    {
        bool  done = true;

        // Loading vertex programs
        for (int i = 0; i < 10; ++i)
        {
            boost::filesystem::path  shader_file;
            vertex_program_receiver_fn  receiver;
            if (!fetch_vertex_program_request(shader_file,receiver))
                break;
            std::shared_ptr<std::vector<std::string> >  lines = std::make_shared< std::vector<std::string> >();
            std::string const  error_message = load_vertex_program_file(shader_file,*lines);
            receiver(shader_file,lines,error_message);
            done = false;
        }

        // Loading fragment programs
        for (int i = 0; i < 10; ++i)
        {
            boost::filesystem::path  shader_file;
            fragment_program_receiver_fn  receiver;
            if (!fetch_fragment_program_request(shader_file,receiver))
                break;
            std::shared_ptr<std::vector<std::string> >  lines = std::make_shared< std::vector<std::string> >();
            std::string const  error_message = load_fragment_program_file(shader_file,*lines);
            receiver(shader_file,lines,error_message);
            done = false;
        }

        // Loading textures
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
