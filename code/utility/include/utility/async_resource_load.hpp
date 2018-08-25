#ifndef UTILITY_ASYNC_RESOURCE_LOAD_HPP_INCLUDED
#   define UTILITY_ASYNC_RESOURCE_LOAD_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <utility/assumptions.hpp>
#   include <utility/invariants.hpp>
#   include <utility/timeprof.hpp>
#   include <utility/std_pair_hash.hpp>
#   include <utility/msgstream.hpp>
#   include <utility/log.hpp>
#   include <boost/filesystem/path.hpp>
#   include <unordered_map>
#   include <vector>
#   include <string>
#   include <mutex>
#   include <queue>
#   include <atomic>
#   include <memory>
#   include <exception>


namespace  async {


enum struct LOAD_STATE
{
    IN_PROGRESS,
    FINISHED_SUCCESSFULLY,
    FINISHED_WITH_ERROR,
};


}

namespace async { namespace detail {


struct  key_type
{
    key_type() : key_type(get_default_data_type_name()) {}

    explicit key_type(std::string const& data_type_name) : key_type(data_type_name, generate_unique_id()) {}

    key_type(std::string const& data_type_name, std::string const&  unique_id)
        : m_data_type_name(data_type_name)
        , m_unique_id(unique_id)
    {}

    std::string const&  get_data_type_name() const { return  m_data_type_name; }
    std::string const&  get_unique_id() const { return  m_unique_id; }

    static char const*  get_default_data_type_name() noexcept { return "@async::any"; }
    static char const*  get_prefix_of_generated_fresh_unique_id() noexcept { return "@async::uid#"; }

    static std::string  generate_unique_id();

    static key_type const&  get_invalid_key();

private:
    std::string  m_data_type_name;
    std::string  m_unique_id;
};


inline bool  operator==(key_type const&  left, key_type const&  right)
{
    return left.get_unique_id() == right.get_unique_id() &&
           left.get_data_type_name() == right.get_data_type_name();
}


inline bool  operator<(key_type const&  left, key_type const&  right)
{
    return left.get_data_type_name() < right.get_data_type_name()
           || ( left.get_data_type_name() == right.get_data_type_name()
                && left.get_unique_id() < right.get_unique_id() );
}


inline bool  operator!=(key_type const&  left, key_type const&  right) { return !(left == right); }
inline bool  operator>(key_type const&  left, key_type const&  right) { return  right < left; }
inline bool  operator<=(key_type const&  left, key_type const&  right) { return  left == right || left < right; }
inline bool  operator>=(key_type const&  left, key_type const&  right) { return  left == right || left > right; }


}}

namespace std
{


template<>
struct hash<async::detail::key_type>
{
    size_t operator()(async::detail::key_type const&  key) const
    {
        size_t seed = 0;
        ::hash_combine(seed, key.get_data_type_name());
        ::hash_combine(seed, key.get_unique_id());
        return seed;
    }
};


}


namespace async { namespace detail {


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
using  notification_callback_type = std::function<void()>;


struct  finalise_load_on_destroy
{
    explicit finalise_load_on_destroy(key_type const&  key)
        : m_key(key)
        , m_error_message()
    {}

    void  force_finalisation_as_failure(std::string const&  force_error_message)
    {
        m_error_message = force_error_message;
    }

    ~finalise_load_on_destroy();

private:
    key_type  m_key;
    std::string  m_error_message;
};


using  finalise_load_on_destroy_ptr = std::shared_ptr<finalise_load_on_destroy>;


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

    LOAD_STATE  get_load_state() const { return m_load_state; }

    pointer_to_resource_type  resource_ptr() const
    {
        return m_resource_ptr;
    }

    std::string const&  error_message() const
    {
        return m_error_message;
    }

    template<typename resource_type>
    static void  resource_loader(
            key_type const&  key,
            resource_holder_type&  resource_holder,
            finalise_load_on_destroy_ptr const  load_finaliser
            );

    template<typename resource_type, typename... arg_types>
    static void  resource_constructor(
            resource_holder_type&  resource_holder,
            finalise_load_on_destroy_ptr const  load_finaliser,
            arg_types... args_for_constructor_of_the_resource
            );

    void  finalise_load(std::string const&  force_error_message);

    template<typename resource_type>
    static void  destroy_resource(pointer_to_resource_type const  resource_ptr);

private:

    std::atomic<natural_64_bit>  m_ref_count;
    pointer_to_resource_type  m_resource_ptr;
    std::string  m_error_message;
    std::atomic<LOAD_STATE>  m_load_state;
};


template<typename resource_type>
void  resource_holder_type::resource_loader(
        key_type const&  key,
        resource_holder_type&  resource_holder,
        finalise_load_on_destroy_ptr const  load_finaliser
        )
{
    TMPROF_BLOCK();

    try
    {
        std::unique_ptr<resource_type> resource_ptr(new resource_type(key, load_finaliser));
        resource_holder.m_resource_ptr = resource_ptr.release();
    }
    catch (std::exception const&  e)
    {
        resource_holder.m_error_message = msgstream() << "ERROR: " << e.what();
    }
    if (!resource_holder.m_error_message.empty())
        LOG(error, resource_holder.m_error_message);
}


template<typename resource_type, typename... arg_types>
void  resource_holder_type::resource_constructor(
        resource_holder_type&  resource_holder,
        finalise_load_on_destroy_ptr const  load_finaliser,
        arg_types... args_for_constructor_of_the_resource)
{
    TMPROF_BLOCK();

    try
    {
        std::unique_ptr<resource_type> resource_ptr(
                new resource_type(load_finaliser, args_for_constructor_of_the_resource...)
                );
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
void  resource_holder_type::destroy_resource(pointer_to_resource_type const  resource_ptr)
{
    TMPROF_BLOCK();

    delete reinterpret_cast<resource_type*>(resource_ptr);
}

using  resources_holder_unique_ptr = std::unique_ptr<resource_holder_type>;
using  resources_cache_type = std::unordered_map<key_type, resources_holder_unique_ptr>;


struct  resource_cache  final
{
    static resource_cache&  instance();

    template<typename resource_type>
    void  insert_load_request(
            key_type const&  key,
            resource_load_priority_type const  priority,
            notification_callback_type const&  parent_notification_callback,
            resources_cache_type::value_type*&  output
            );

    template<typename resource_type, typename... arg_types>
    void  insert_resource(
            key_type const&  key,
            notification_callback_type const&  parent_notification_callback,
            resources_cache_type::value_type*&  output,
            arg_types... args_for_constructor_of_the_resource
            );

    resources_cache_type::value_type*  find_resource(key_type const&  key);

    void  finalise_load(key_type const&  key, std::string const&  force_error_message);

    void  process_notification_callbacks(key_type const&  key);

    template<typename resource_type>
    void  erase_resource(key_type const&  key);

    std::mutex&  mutex() { return m_mutex; }

private:

    resource_cache() = default;

    resource_cache(resource_cache const&) = delete;
    resource_cache& operator=(resource_cache const&) = delete;
    resource_cache(resource_cache&&) = delete;

    ~resource_cache();

    static key_type  generate_fresh_key();

    static natural_64_bit  s_fresh_key_id;

    resources_cache_type  m_cache;
    std::unordered_map<key_type, std::vector<notification_callback_type> >  m_notification_callbacks;
    std::mutex  m_mutex;
};


template<typename resource_type>
void  resource_cache::insert_load_request(
        key_type const&  key,
        resource_load_priority_type const  priority,
        notification_callback_type const&  parent_notification_callback,
        resources_cache_type::value_type*&  output
        )
{
    TMPROF_BLOCK();

    {
        std::lock_guard<std::mutex> const  lock(mutex());
        resources_cache_type::value_type* const  resource_ptr = find_resource(key);
        if (resource_ptr != nullptr)
        {
            if (resource_ptr->second->get_load_state() == LOAD_STATE::IN_PROGRESS &&
                    parent_notification_callback.operator bool())
                m_notification_callbacks[key].push_back(parent_notification_callback);
            output = resource_ptr;
            output->second->inc_ref_count();
            return;
        }

        auto const  iter_and_bool = m_cache.insert({
                key,
                resources_holder_unique_ptr(new resource_holder_type)
                });
        INVARIANT(iter_and_bool.second);

        if (parent_notification_callback.operator bool())
            m_notification_callbacks[key].push_back(parent_notification_callback);

        output = &*iter_and_bool.first;
        output->second->inc_ref_count();
    }

    resource_load_planner::instance().insert_load_request(
        key,
        std::bind(
            &resource_holder_type::resource_loader<resource_type>,
            key,
            std::ref(*output->second),
            finalise_load_on_destroy_ptr(new finalise_load_on_destroy(key))
            ),
        priority
        );
}


template<typename resource_type, typename... arg_types>
void  resource_cache::insert_resource(
        key_type const&  key,
        notification_callback_type const&  parent_notification_callback,
        resources_cache_type::value_type*&  output,
        arg_types... args_for_constructor_of_the_resource
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(key != key_type::get_invalid_key());

    {
        std::lock_guard<std::mutex> const  lock(mutex());

        resources_cache_type::value_type* const  resource_ptr = find_resource(key);
        if (resource_ptr != nullptr)
        {
            if (resource_ptr->second->get_load_state() == LOAD_STATE::IN_PROGRESS &&
                    parent_notification_callback.operator bool())
                m_notification_callbacks[key].push_back(parent_notification_callback);
            output = resource_ptr;
            output->second->inc_ref_count();
            return;
        }

        auto const  iter_and_bool = m_cache.insert({
                key,
                resources_holder_unique_ptr(new resource_holder_type)
                });
        INVARIANT(iter_and_bool.second);

        if (parent_notification_callback.operator bool())
            m_notification_callbacks[key].push_back(parent_notification_callback);

        output = &*iter_and_bool.first;
        output->second->inc_ref_count();
    }

    resource_holder_type::resource_constructor<resource_type>(
        *output->second,
        finalise_load_on_destroy_ptr(new finalise_load_on_destroy(key)),
        args_for_constructor_of_the_resource...
        );
}


template<typename resource_type>
void  resource_cache::erase_resource(key_type const&  key)
{
    TMPROF_BLOCK();

    pointer_to_resource_type  destroy_resource_ptr = nullptr;
    {
        std::lock_guard<std::mutex> const  lock(mutex());
        auto const  it = m_cache.find(key);
        if (it == m_cache.end())
            return;

        resource_load_planner::instance().cancel_load_request(key);

        ASSUMPTION(it->second->ref_count() == 0UL);
        destroy_resource_ptr = it->second->resource_ptr();

        m_notification_callbacks.erase(key);
        m_cache.erase(it);
    }
    if (destroy_resource_ptr != nullptr)
        resource_holder_type::destroy_resource<resource_type>(destroy_resource_ptr);
}


}}

namespace async {


using  key_type = detail::key_type;
using  load_priority_type = detail::resource_load_priority_type;
using  notification_callback_type = detail::notification_callback_type;
using  finalise_load_on_destroy_ptr = detail::finalise_load_on_destroy_ptr;


template<typename resource_type__>
struct  resource_accessor
{
    using  resource_type = resource_type__;

    resource_accessor()
        : m_data_ptr(nullptr)
    {}

    explicit resource_accessor(
            key_type const&  key,
            load_priority_type const  priority,
            notification_callback_type const& notification_callback = notification_callback_type())
        : m_data_ptr(nullptr)
    {
        insert_load_request(key, priority, notification_callback);
    }

    template<typename... arg_types>
    explicit resource_accessor(
            key_type const&  key,
            notification_callback_type const& notification_callback,
            arg_types... args_for_constructor_of_the_resource
            )
        : m_data_ptr(nullptr)
    {
        insert_resource(key, notification_callback, args_for_constructor_of_the_resource...);
    }

    resource_accessor(resource_accessor<resource_type> const&  other)
        : m_data_ptr(other.m_data_ptr)
    {
        if (m_data_ptr != nullptr)
            m_data_ptr->second->inc_ref_count();
    }

    resource_accessor<resource_type>& operator=(resource_accessor<resource_type> const&  other)
    {
        if (this == &other)
            return *this;
        release();
        m_data_ptr = other.m_data_ptr;
        if (m_data_ptr != nullptr)
            m_data_ptr->second->inc_ref_count();
        return *this;
    }

    virtual bool  operator==(resource_accessor<resource_type> const&  other) const
    {
        return m_data_ptr == other.m_data_ptr;
    }

    virtual ~resource_accessor()
    {
        release();
    }

    void  release()
    {
        if (m_data_ptr != nullptr)
        {
            m_data_ptr->second->dec_ref_count();
            if (m_data_ptr->second->ref_count() == 0ULL)
                detail::resource_cache::instance().erase_resource<resource_type>(key());
            m_data_ptr = nullptr;
        }
    }

    bool  empty() const { return m_data_ptr == nullptr; }

    LOAD_STATE  get_load_state() const
    {
        return empty() ? LOAD_STATE::IN_PROGRESS : m_data_ptr->second->get_load_state();
    }

    bool  is_load_finished() const
    {
        return get_load_state() != LOAD_STATE::IN_PROGRESS;
    }

    bool  loaded_successfully() const
    {
        return get_load_state() == LOAD_STATE::FINISHED_SUCCESSFULLY;
    }

    bool  load_failed() const
    {
        return get_load_state() == LOAD_STATE::FINISHED_WITH_ERROR;
    }

    key_type const&  key() const
    {
        ASSUMPTION(m_data_ptr != nullptr);
        return m_data_ptr->first;
    }

    resource_type&  resource() const
    {
        ASSUMPTION(is_load_finished());
        return *reinterpret_cast<resource_type*>(m_data_ptr->second->resource_ptr());
    }

    std::string const&  error_message() const
    {
        ASSUMPTION(is_load_finished());
        return m_data_ptr->second->error_message();
    }

protected:

    void  insert_load_request(
            key_type const&  key,
            load_priority_type const  priority,
            notification_callback_type const& notification_callback = notification_callback_type()
            )
    {
        ASSUMPTION(m_data_ptr == nullptr);
        detail::resource_cache::instance().insert_load_request<resource_type>(
                            key,
                            priority,
                            notification_callback,
                            m_data_ptr
                            );
        if (m_data_ptr->second->get_load_state() != LOAD_STATE::IN_PROGRESS && notification_callback.operator bool())
            notification_callback();
    }

    template<typename... arg_types>
    void  insert_resource(
            key_type const&  key,
            notification_callback_type const& notification_callback,
            arg_types... args_for_constructor_of_the_resource
            )
    {
        ASSUMPTION(m_data_ptr == nullptr);
        detail::resource_cache::instance().insert_resource<resource_type>(
                            key,
                            notification_callback,
                            m_data_ptr,
                            args_for_constructor_of_the_resource...
                            );
        if (m_data_ptr->second->get_load_state() != LOAD_STATE::IN_PROGRESS && notification_callback.operator bool())
            notification_callback();
    }

private:

    detail::resources_cache_type::value_type*  m_data_ptr;
};


template<typename resource_type__>
resource_accessor<resource_type__>  insert_load_request(
        key_type const&  key,
        load_priority_type const  priority,
        notification_callback_type const& notification_callback = notification_callback_type()
        )
{
    resource_accessor<resource_type__>  accessor;
    accessor.insert_load_request(key, priority, notification_callback);
    return accessor;
}


template<typename resource_type__, typename... arg_types>
resource_accessor<resource_type__>  insert_resource(
        key_type const&  key,
        arg_types... args_for_constructor_of_the_resource,
        notification_callback_type const& notification_callback = notification_callback_type()
        )
{
    resource_accessor<resource_type__>  accessor;
    accessor.insert_resource(key, args_for_constructor_of_the_resource..., notification_callback);
    return accessor;
}


}

#endif
