#ifndef QTGL_DETAIL_RESOURCE_CACHE_HPP_INCLUDED
#   define QTGL_DETAIL_RESOURCE_CACHE_HPP_INCLUDED

#   include <utility/instance_wrapper.hpp>
#   include <utility/assumptions.hpp>
#   include <utility/invariants.hpp>
#   include <utility/timeprof.hpp>
#   include <utility/msgstream.hpp>
#   include <utility/log.hpp>
#   include <boost/filesystem/path.hpp>
#   include <map>
#   include <vector>
#   include <mutex>
#   include <queue>
#   include <atomic>
#   include <exception>

namespace qtgl { namespace detail {


using  async_load_resuorce_priority_type = natural_32_bit;


struct  resource_load_planner  final
{
    using  pointer_to_resource_type = void*;
    using  loader_type = std::function<void(boost::filesystem::path const&,pointer_to_resource_type)>;

    static resource_load_planner&  instance();

    ~resource_load_planner();

    void clear();

    void  insert_load_request(
            boost::filesystem::path const&  path,
            pointer_to_resource_type const  pointer_to_resource,
            loader_type const&  loader,
            async_load_resuorce_priority_type const  priority
            );

    void  invalidate_load_request(pointer_to_resource_type const  pointer_to_resource);

    std::mutex&  mutex_to_resources() { return m_mutex_to_resources; }

    pointer_to_resource_type  pointer_to_resource_just_being_loaded() { return m_pointer_to_resource_just_being_loaded; }

private:

    using  queue_value_type =
                std::tuple<
                    async_load_resuorce_priority_type,
                    boost::filesystem::path,
                    pointer_to_resource_type,loader_type
                    >;
    using  queue_storage_type = std::vector<queue_value_type>;
    using  queus_less_than_type = std::function<bool(queue_value_type const&, queue_value_type const&)>;

    struct  queue_type : public std::priority_queue<queue_value_type,queue_storage_type,queus_less_than_type>
    {
        using  super_type = std::priority_queue<queue_value_type,queue_storage_type,queus_less_than_type>;
        queue_type()
            : super_type([](queue_value_type const& l, queue_value_type const& r) -> bool { return std::get<0>(l) < std::get<0>(r); })
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
    std::mutex  m_mutex_to_resources;
    std::atomic<pointer_to_resource_type>  m_pointer_to_resource_just_being_loaded;
    std::thread  m_worker_thread;
    std::atomic<bool>  m_worker_finished;
};


template<typename resource_type__>
struct  resource_cache  final
{
    using  resource_type = resource_type__;

    using  key_type = boost::filesystem::path;

    struct  mapped_type  final
    {
        mapped_type()
            : m_ref_count(0ULL)
            , m_wrapped_resource()
            , m_error_message()
        {}

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

        static void  loader(
                mapped_type* const  this_ptr,
                boost::filesystem::path const&  path,
                resource_load_planner::pointer_to_resource_type const  ptr
                )
        {
            ASSUMPTION(this_ptr != nullptr && &this_ptr->m_wrapped_resource == reinterpret_cast<instance_wrapper<resource_type>*>(ptr));
            try
            {
                reinterpret_cast<instance_wrapper<resource_type>*>(ptr)->construct_instance(path);
            }
            catch (std::exception const&  e)
            {
                this_ptr->m_error_message = msgstream() << "ERROR: " << e.what();
            }
            if (!this_ptr->m_error_message.empty())
                LOG(error,this_ptr->m_error_message);
        }

        bool  is_load_finished() const
        {
            std::lock_guard<std::mutex> const  lock(resource_load_planner::instance().mutex_to_resources());
            return (resource_load_planner::pointer_to_resource_type)this != 
                    resource_load_planner::instance().pointer_to_resource_just_being_loaded() &&
                   (m_wrapped_resource.is_constructed() || !m_error_message.empty());
        }

        resource_type const*  resource_ptr() const
        {
            return is_load_finished() && m_error_message.empty() ? m_wrapped_resource.operator->() : nullptr;
        }

        std::string const*  error_message_ptr() const
        {
            return is_load_finished() ? (m_error_message.empty() ? nullptr : &m_error_message) : nullptr;
        }

        instance_wrapper<resource_type>*  wrapped_resource_ptr()
        {
            return &m_wrapped_resource;
        }

    private:

        natural_64_bit  m_ref_count;
        instance_wrapper<resource_type>  m_wrapped_resource;
        std::string  m_error_message;
    };

    using  cache_type = std::map<key_type,mapped_type>;

    struct  resource_handle  final
    {
        explicit resource_handle(typename cache_type::value_type* const  data_ptr)
            : m_data_ptr(data_ptr)
        {
            ASSUMPTION(m_data_ptr != nullptr);
            m_data_ptr->second.inc_ref_count();
        }

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

        ~resource_handle()
        {
            m_data_ptr->second.dec_ref_count();
            if (m_data_ptr->second.ref_count() == 0ULL)
                resource_cache::instance().on_unreferenced_resource(key());
        }

        key_type const&  key() const
        {
            return m_data_ptr->first;
        }

        bool  is_load_finished() const
        {
            return m_data_ptr->second.is_load_finished();
        }

        resource_type const*  resource_ptr() const
        {
            return m_data_ptr->second.resource_ptr();
        }

        std::string const*  error_message_ptr() const
        {
            return m_data_ptr->second.error_message_ptr();
        }

    private:

        typename cache_type::value_type*  m_data_ptr;
    };

    static resource_cache&  instance();

    void clear();

    resource_handle  insert_load_request(key_type const&  key, async_load_resuorce_priority_type const  priority);

private:

    void  on_unreferenced_resource(key_type const&  key);

    resource_cache() = default;

    resource_cache(resource_cache const&) = delete;
    resource_cache& operator=(resource_cache const&) = delete;
    resource_cache(resource_cache&&) = delete;

    ~resource_cache() = default;

    cache_type  m_cache;
};


template<typename resource_type__>
resource_cache<resource_type__>&  resource_cache<resource_type__>::instance()
{
    static resource_cache  cache;
    return cache;
}


template<typename resource_type__>
void resource_cache<resource_type__>::clear()
{
    TMPROF_BLOCK();

    while (!m_cache.empty())
    {
        resource_load_planner::pointer_to_resource_type const  ptr = m_cache.begin()->second.wrapped_resource_ptr();
        resource_load_planner::instance().invalidate_load_request(ptr);
        std::lock_guard<std::mutex> const  lock(resource_load_planner::instance().mutex_to_resources());
        if (ptr != resource_load_planner::instance().pointer_to_resource_just_being_loaded())
            m_cache.erase(m_cache.begin());
    }
}


template<typename resource_type__>
typename resource_cache<resource_type__>::resource_handle  resource_cache<resource_type__>::insert_load_request(
    key_type const&  key,
    async_load_resuorce_priority_type const  priority)
{
    TMPROF_BLOCK();

    cache_type::iterator const  it = m_cache.find(key);
    if (it != m_cache.end())
        return resource_handle(&*it);

    auto const  iter_bool = m_cache.insert({ key,mapped_type{} });
    INVARIANT(iter_bool.second);

    cache_type::value_type* const  value_ptr = &*iter_bool.first;
    instance_wrapper<resource_type>* const  wrapped_resource_ptr = value_ptr->second.wrapped_resource_ptr();

    resource_load_planner::instance().insert_load_request(
            key,
            reinterpret_cast<resource_load_planner::pointer_to_resource_type>(wrapped_resource_ptr),
            std::bind(mapped_type::loader,&value_ptr->second,std::placeholders::_1,std::placeholders::_2),
            priority
            );

    return resource_handle(value_ptr);
}


template<typename resource_type__>
void  resource_cache<resource_type__>::on_unreferenced_resource(key_type const&  key)
{
    auto const  it = m_cache.find(key);
    INVARIANT(it != m_cache.end());
    std::lock_guard<std::mutex> const  lock(resource_load_planner::instance().mutex_to_resources());
    if (it->second.wrapped_resource_ptr() != resource_load_planner::instance().pointer_to_resource_just_being_loaded())
        m_cache.erase(it);
}


enum struct ASYNC_LOAD_STATE
{
    IN_PROGRESS,
    FINISHED_SUCCESSFULLY,
    FINISHED_WITH_ERROR,
};


template<typename resource_type__>
struct  async_resource_accessor
{
    using  resource_type = resource_type__;
    using  resource_cache_type = resource_cache<resource_type>;
    using  key_type = typename resource_cache_type::key_type;

    explicit async_resource_accessor(key_type const&  key, async_load_resuorce_priority_type const  priority)
        : m_handle(resource_cache_type::instance().insert_load_request(key, priority))
    {}

    bool  loaded_successfully() const { return get_load_state() == ASYNC_LOAD_STATE::FINISHED_SUCCESSFULLY; }
    bool  load_failed() const { return get_load_state() == ASYNC_LOAD_STATE::FINISHED_WITH_ERROR; }

    key_type const&  key() const { return m_handle.key(); }

    ASYNC_LOAD_STATE  get_load_state() const
    {
        return !m_handle.is_load_finished() ? ASYNC_LOAD_STATE::IN_PROGRESS :
            resource_ptr() != nullptr ? ASYNC_LOAD_STATE::FINISHED_SUCCESSFULLY :
            ASYNC_LOAD_STATE::FINISHED_WITH_ERROR;
    }

    resource_type const*  resource_ptr() const { return m_handle.resource_ptr(); }

    std::string const*  error_message_ptr() const { return m_handle.error_message_ptr(); }

private:

    typename resource_cache_type::resource_handle  m_handle;
};


template<typename resource_type__>
struct  async_resource_accessor_base : public async_resource_accessor<resource_type__>
{
    explicit async_resource_accessor_base(key_type const&  key, async_load_resuorce_priority_type const  priority)
        : async_resource_accessor(key, priority)
    {}

    boost::filesystem::path  path() const { return boost::filesystem::path(key()); }
    std::string const&  get_load_fail_message() const { return *error_message_ptr(); }

protected:
    using  async_resource_accessor<resource_type>::key;
    using  async_resource_accessor<resource_type>::resource_ptr;
    using  async_resource_accessor<resource_type>::error_message_ptr;
};


}}

#endif
