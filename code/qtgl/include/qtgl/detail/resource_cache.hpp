#ifndef QTGL_DETAIL_RESOURCE_CACHE_HPP_INCLUDED
#   define QTGL_DETAIL_RESOURCE_CACHE_HPP_INCLUDED

#   include <utility/instance_wrapper.hpp>
#   include <utility/assumptions.hpp>
#   include <utility/invariants.hpp>
#   include <utility/timeprof.hpp>
#   include <utility/msgstream.hpp>
#   include <utility/log.hpp>
#   include <boost/filesystem/path.hpp>
#   include <unordered_map>
#   include <vector>
#   include <mutex>
#   include <queue>
#   include <atomic>
#   include <exception>

namespace qtgl {


enum struct ASYNC_LOAD_STATE
{
    IN_PROGRESS,
    FINISHED_SUCCESSFULLY,
    FINISHED_WITH_ERROR,
};


}

namespace qtgl { namespace detail { namespace async {


using  key_type = std::string;
using  resource_loader_type = std::function<void()>;
using  resource_load_priority_type = natural_32_bit;


struct  resource_load_planner  final
{
    static resource_load_planner&  instance();

    ~resource_load_planner();

    void clear();

    void  insert_load_request(
            key_type const&  key,
            resource_loader_type const&  loader,
            resource_load_priority_type const  priority
            );

    void  cancel_load_request(key_type const&  key);

    std::mutex&  mutex() { return m_mutex; }

    key_type const&  resource_just_being_loaded() { return m_resource_just_being_loaded; }

private:

    using  queue_value_type = std::tuple<resource_load_priority_type, key_type, resource_loader_type>;
    using  queue_storage_type = std::vector<queue_value_type>;
    using  queus_less_than_type = std::function<bool(queue_value_type const&, queue_value_type const&)>;

    struct  queue_type : public std::priority_queue<queue_value_type,queue_storage_type,queus_less_than_type>
    {
        using  super_type = std::priority_queue<queue_value_type,queue_storage_type,queus_less_than_type>;
        queue_type()
            : super_type(
                [](queue_value_type const& l, queue_value_type const& r) -> bool {
                    return std::get<0>(l) < std::get<0>(r);
                    }
                )
        {}
        queue_storage_type::const_iterator  cbegin() { return c.cbegin(); }
        queue_storage_type::const_iterator  cend() { return c.cend(); }
        queue_storage_type::iterator  begin() { return c.begin(); }
        queue_storage_type::iterator  end() { return c.end(); }
        void  clear() { c.clear(); }
    };

    resource_load_planner();

    resource_load_planner(resource_load_planner const&) = delete;
    resource_load_planner& operator=(resource_load_planner const&) = delete;
    resource_load_planner(resource_load_planner&&) = delete;

    void  start_worker_if_not_running();

    void  worker();

    queue_type  m_queue;
    std::mutex  m_mutex;
    key_type  m_resource_just_being_loaded;
    std::thread  m_worker_thread;
    std::atomic<bool>  m_worker_finished;
};


using  pointer_to_resource_type = void*;


struct  resource_holder_type  final
{
    resource_holder_type();

    natural_64_bit  ref_count() const
    {
        return m_ref_count;
    }

    void  inc_ref_count()
    {
        ++m_ref_count;
        ASSUMPTION(m_ref_count != 0ULL);
    }

    void  dec_ref_count()
    {
        ASSUMPTION(m_ref_count != 0ULL);
        --m_ref_count;
    }

    pointer_to_resource_type  resource_ptr() const
    {
        return m_resource_ptr;
    }

    std::string const&  error_message() const
    {
        return m_error_message;
    }

    ASYNC_LOAD_STATE  get_load_state(key_type const&  key) const;

    template<typename resource_type>
    static void  resource_loader(
            key_type const&  path,
            resource_holder_type&  resource_holder
            );

    template<typename resource_type>
    void  destroy_resource();

private:

    natural_64_bit  m_ref_count;
    pointer_to_resource_type  m_resource_ptr;
    std::string  m_error_message;
    mutable ASYNC_LOAD_STATE  m_load_state;
};


template<typename resource_type>
void  resource_holder_type::resource_loader(
        key_type const&  key,
        resource_holder_type&  resource_holder
        )
{
    try
    {
        std::unique_ptr<resource_type> resource_ptr(new resource_type(key));
        resource_holder.m_resource_ptr = resource_ptr.release();
    }
    catch (std::exception const&  e)
    {
        resource_holder.m_error_message = msgstream() << "ERROR: " << e.what();
    }
    if (!resource_holder.m_error_message.empty())
        LOG(error, resource_holder.m_error_message);
}


template<typename resource_type>
void  resource_holder_type::destroy_resource()
{
    ASSUMPTION(m_ref_count == 0ULL);
    delete reinterpret_cast<resource_type*>(resource_ptr());
    m_resource_ptr = nullptr;
}


using  resources_cache_type = std::unordered_map<key_type, resource_holder_type>;


struct  resource_cache  final
{
    static resource_cache&  instance();

    template<typename resource_type>
    resources_cache_type::value_type*  insert_load_request(
            key_type const&  key,
            resource_load_priority_type const  priority
            );

    void  erase_resource(key_type const&  key);

private:

    resource_cache() = default;

    resource_cache(resource_cache const&) = delete;
    resource_cache& operator=(resource_cache const&) = delete;
    resource_cache(resource_cache&&) = delete;

    ~resource_cache() = default;

    resources_cache_type  m_cache;
};


template<typename resource_type>
resources_cache_type::value_type*  resource_cache::insert_load_request(
        key_type const&  key,
        resource_load_priority_type const  priority)
{
    TMPROF_BLOCK();

    resources_cache_type::iterator const  it = m_cache.find(key);
    if (it != m_cache.end())
        return &*it;

    auto const  iter_and_bool = m_cache.insert({ key,resource_holder_type() });
    INVARIANT(iter_and_bool.second);

    resource_load_planner::instance().insert_load_request(
            key,
            std::bind(
                &resource_holder_type::resource_loader<resource_type>,
                key,
                std::ref(iter_and_bool.first->second)
                ),
            priority
            );

    return &*iter_and_bool.first;
}


struct  resource_handle  final
{
    explicit resource_handle(resources_cache_type::value_type* const  data_ptr);

    template<typename resource_type>
    void  destroy();

    resource_handle(resource_handle const&  other)
        : m_data_ptr(other.m_data_ptr)
    {
        m_data_ptr->second.inc_ref_count();
    }

    resource_handle& operator=(resource_handle const&  other)
    {
        m_data_ptr = other.m_data_ptr;
        m_data_ptr->second.inc_ref_count();
        return *this;
    }

    key_type const&  key() const
    {
        return m_data_ptr->first;
    }

    ASYNC_LOAD_STATE  get_load_state() const
    {
        return m_data_ptr->second.get_load_state(m_data_ptr->first);
    }

    bool  is_load_finished() const
    {
        return get_load_state() != ASYNC_LOAD_STATE::IN_PROGRESS;
    }

    pointer_to_resource_type  resource_ptr() const
    {
        ASSUMPTION(is_load_finished());
        return m_data_ptr->second.resource_ptr();
    }

    std::string const&  error_message() const
    {
        ASSUMPTION(is_load_finished());
        return m_data_ptr->second.error_message();
    }

private:

    resources_cache_type::value_type*  m_data_ptr;
};


template<typename resource_type>
void  resource_handle::destroy()
{
    m_data_ptr->second.dec_ref_count();
    if (m_data_ptr->second.ref_count() == 0ULL)
    {
        resource_load_planner::instance().cancel_load_request(key());
        m_data_ptr->second.destroy_resource<resource_type>();
        resource_cache::instance().erase_resource(key());
        m_data_ptr = nullptr;
    }
}


}}}

namespace qtgl {


template<typename resource_type__>
struct  async_resource_accessor
{
    using  key_type = detail::async::key_type;
    using  resource_type = resource_type__;
    using  resource_load_priority_type = detail::async::resource_load_priority_type;

    explicit async_resource_accessor(
            boost::filesystem::path const&  path,
            resource_load_priority_type const  priority
            )
        : m_handle(
            detail::async::resource_cache::instance().insert_load_request<resource_type>(
                    path.string(),
                    priority
                    )
            )
    {}

    ~async_resource_accessor()
    {
        m_handle.destroy<resource_type>();
    }

    boost::filesystem::path  path() const { return boost::filesystem::path(key()); }

    bool  loaded_successfully() const { return get_load_state() == ASYNC_LOAD_STATE::FINISHED_SUCCESSFULLY; }
    bool  load_failed() const { return get_load_state() == ASYNC_LOAD_STATE::FINISHED_WITH_ERROR; }

    key_type const&  key() const { return m_handle.key(); }

    ASYNC_LOAD_STATE  get_load_state() const { return m_handle.get_load_state(); }

    resource_type&  resource() const { return *reinterpret_cast<resource_type*>(m_handle.resource_ptr()); }

    std::string const&  load_fail_message() const { return m_handle.error_message(); }

private:

    detail::async::resource_handle  m_handle;
};


template<typename resource_type__>
struct  async_resource_accessor_base : public async_resource_accessor<resource_type__>
{
    explicit async_resource_accessor_base(
            boost::filesystem::path const&  path,
            resource_load_priority_type const  priority
            )
        : async_resource_accessor<resource_type__>(path, priority)
    {}

protected:
    using  async_resource_accessor<resource_type>::key;
    using  async_resource_accessor<resource_type>::resource;
};


}

#endif
