#ifndef UTILITY_ASYNC_RESOURCE_LOAD_HPP_INCLUDED
#   define UTILITY_ASYNC_RESOURCE_LOAD_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <utility/assumptions.hpp>
#   include <utility/invariants.hpp>
#   include <utility/timeprof.hpp>
#   include <utility/std_pair_hash.hpp>
#   include <utility/msgstream.hpp>
#   include <utility/log.hpp>
#   include <filesystem>
#   include <unordered_map>
#   include <vector>
#   include <string>
#   include <mutex>
#   include <queue>
#   include <thread>
#   include <atomic>
#   include <memory>
#   include <iosfwd>
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
    static std::string  generate_unique_custom_id(std::string  custom_prefix);

    static key_type const&  get_invalid_key();

private:
    static natural_64_bit  generate_next_unique_id();

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


std::ostream&  operator<<(std::ostream&  ostr, key_type const&  key);
std::string  to_string(key_type const&  key);


}}

namespace async { namespace detail {


struct  finalise_load_on_destroy;
using  finalise_load_on_destroy_ptr = std::shared_ptr<finalise_load_on_destroy>;
using  callback_function_type = std::function<void(finalise_load_on_destroy_ptr)>;


struct  finalise_load_on_destroy
{
    static finalise_load_on_destroy_ptr  create(
            key_type const&  key,
            finalise_load_on_destroy_ptr const  parent = nullptr
            );

    static finalise_load_on_destroy_ptr  create(
            callback_function_type const&  callback,
            finalise_load_on_destroy_ptr const  finaliser
            );

    ~finalise_load_on_destroy();

    key_type const&  get_key() const { return m_key; }
    std::string const&  get_error_message() const { return m_error_message; }
    finalise_load_on_destroy_ptr  get_parent() const { return m_parent; }
    callback_function_type const&  get_callback() const { return m_callback; }

    void  force_finalisation_as_failure(std::string const&  force_error_message);

private:

    finalise_load_on_destroy(
            key_type const&  key,
            finalise_load_on_destroy_ptr const  parent,
            callback_function_type const&  callback 
            );

    key_type  m_key;
    std::string  m_error_message;
    finalise_load_on_destroy_ptr  m_parent;
    callback_function_type  m_callback;
};


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


void terminate();
bool is_terminated();


}}

namespace async { namespace detail {


using  resource_load_priority_type = natural_32_bit;

using  resource_load_data_type = std::pair<key_type, std::function<void()> >;
inline  resource_load_data_type  get_invalid_resource_load_data() { return resource_load_data_type{ key_type::get_invalid_key(),{} }; }
inline  key_type const&  get_key_of_resource_load_data(resource_load_data_type const&  data) { return data.first; }
inline  void  perform_load_of_resource_load_data(resource_load_data_type const&  data) { data.second(); }


struct  resource_load_planner  final
{
    static resource_load_planner&  instance();

    ~resource_load_planner();

    void clear();

    void  insert_load_request(resource_load_priority_type const  priority, resource_load_data_type const&  data);

    std::mutex&  mutex() { return m_mutex; }

    resource_load_data_type const&  resource_just_being_loaded() { return m_resource_just_being_loaded; }

private:

    using  queue_value_type = std::pair<resource_load_priority_type, resource_load_data_type>;
    using  queue_storage_type = std::vector<queue_value_type>;
    using  queus_less_than_type = std::function<bool(queue_value_type const&, queue_value_type const&)>;

    struct  queue_type : public std::priority_queue<queue_value_type,queue_storage_type,queus_less_than_type>
    {
        using  super_type = std::priority_queue<queue_value_type,queue_storage_type,queus_less_than_type>;
        queue_type() : super_type([](queue_value_type const& l, queue_value_type const& r) -> bool { return l.first < r.first; }) {}
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
    resource_load_data_type  m_resource_just_being_loaded;
    std::thread  m_worker_thread;
    std::atomic<bool>  m_worker_finished;
};


using  pointer_to_resource_type = void*;
using  ref_counter_type = natural_32_bit;

struct  resource_holder_type  final
{
    resource_holder_type();

    ref_counter_type  ref_count() const
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

    void  set_resource_ptr(pointer_to_resource_type const  ptr)
    {
        m_resource_ptr = ptr;
    }

    std::string const&  error_message() const
    {
        return m_error_message;
    }

    void  finalise_load(std::string const&  force_error_message);

private:

    std::atomic<ref_counter_type>  m_ref_count;
    pointer_to_resource_type  m_resource_ptr;
    std::string  m_error_message;
    std::atomic<LOAD_STATE>  m_load_state;
};


using  resources_holder_unique_ptr = std::unique_ptr<resource_holder_type>;
using  resources_cache_type = std::unordered_map<key_type, resources_holder_unique_ptr>;
using  finalisers_cache_type = std::unordered_map<key_type, std::vector<finalise_load_on_destroy_ptr> >;


struct  resource_cache  final
{
    static resource_cache&  instance();

    template<typename resource_type>
    void  insert_load_request(
            key_type const&  key,
            resource_load_priority_type const  priority,
            finalise_load_on_destroy_ptr const  parent_finaliser,
            resources_cache_type::value_type*&  output
            );

    template<typename resource_type>
    void  load_resource(finalise_load_on_destroy_ptr const  load_finaliser);

    template<typename resource_type, typename... arg_types>
    void  insert_resource(
            key_type const&  key,
            finalise_load_on_destroy_ptr const  parent_finaliser,
            resources_cache_type::value_type*&  output,
            arg_types... args_for_constructor_of_the_resource
            );

    resources_cache_type::value_type*  find_resource(key_type const&  key);

    LOAD_STATE  finalise_load(key_type const  key, std::string const&  error_message);

    template<typename resource_type>
    void  erase_resource(key_type const&  key);

    resources_cache_type const&  get_cache() const { return m_cache; }
    std::mutex&  mutex() { return m_mutex; }

    void  take_finalisers(key_type const  key, std::vector<finalise_load_on_destroy_ptr>&  output);

private:

    resource_cache() = default;

    resource_cache(resource_cache const&) = delete;
    resource_cache& operator=(resource_cache const&) = delete;
    resource_cache(resource_cache&&) = delete;

    ~resource_cache();

    static key_type  generate_fresh_key();

    static natural_64_bit  s_fresh_key_id;

    resources_cache_type  m_cache;
    finalisers_cache_type  m_finalisers;
    std::mutex  m_mutex;
};


template<typename resource_type>
void  resource_cache::insert_load_request(
        key_type const&  key,
        resource_load_priority_type const  priority,
        finalise_load_on_destroy_ptr const  parent_finaliser,
        resources_cache_type::value_type*&  output
        )
{
    TMPROF_BLOCK();

    if (is_terminated())
    {
        if (parent_finaliser != nullptr)
            parent_finaliser->force_finalisation_as_failure(msgstream() <<
                "ERROR: Resource loading was terminated. So, terminating also load of the resource " << parent_finaliser->get_key() << "."
                );
        return;
    }

    {
        std::lock_guard<std::mutex> const  lock(mutex());

        resources_cache_type::value_type* const  resource_ptr = find_resource(key);
        if (resource_ptr != nullptr)
        {
            output = resource_ptr;
            output->second->inc_ref_count();
            if (resource_ptr->second->get_load_state() == LOAD_STATE::IN_PROGRESS)
                m_finalisers[key].push_back(finalise_load_on_destroy::create(key, parent_finaliser));
            return;
        }

        auto const  iter_and_bool = m_cache.insert({
                key,
                resources_holder_unique_ptr(new resource_holder_type)
                });
        INVARIANT(iter_and_bool.second);

        output = &*iter_and_bool.first;
        output->second->inc_ref_count();
    }

    resource_load_planner::instance().insert_load_request(
            priority,
            {key, std::bind(&resource_cache::load_resource<resource_type>, this, finalise_load_on_destroy::create(key, parent_finaliser))}
            );
}


template<typename resource_type>
void  resource_cache::load_resource(finalise_load_on_destroy_ptr const  load_finaliser)
{
    TMPROF_BLOCK();

    ASSUMPTION(load_finaliser->get_key() != key_type::get_invalid_key());

    if (is_terminated())
    {
        load_finaliser->force_finalisation_as_failure(msgstream() <<
            "ERROR: Resource loading was terminated. So, terminating also load of the resource " << load_finaliser->get_key() << "."
            );
        return;
    }

    {
        std::lock_guard<std::mutex> const  lock(mutex());
        resources_cache_type::value_type* const  resource_ptr = find_resource(load_finaliser->get_key());
        if (resource_ptr == nullptr)
        {
            load_finaliser->force_finalisation_as_failure(msgstream() <<
                "ERROR: The resource " << load_finaliser->get_key() << " was destroyed even before the load was started."
                );
            return;
        }
    }

    try
    {
        std::unique_ptr<resource_type>  data_ptr(new resource_type(load_finaliser));

        std::lock_guard<std::mutex> const  lock(mutex());
        resources_cache_type::value_type* const  resource_ptr = find_resource(load_finaliser->get_key());
        if (resource_ptr == nullptr)
        {
            load_finaliser->force_finalisation_as_failure(msgstream() <<
                "ERROR: The resource " << load_finaliser->get_key() << " was destroyed during the load of its data."
                );
            return;
        }
        resource_ptr->second->set_resource_ptr(data_ptr.release());
    }
    catch (std::exception const&  e)
    {
        load_finaliser->force_finalisation_as_failure(msgstream() << "ERROR: " << e.what());
    }
}


template<typename resource_type, typename... arg_types>
void  resource_cache::insert_resource(
        key_type const&  key,
        finalise_load_on_destroy_ptr const  parent_finaliser,
        resources_cache_type::value_type*&  output,
        arg_types... args_for_constructor_of_the_resource
        )
{
    TMPROF_BLOCK();

    if (is_terminated())
    {
        if (parent_finaliser != nullptr)
            parent_finaliser->force_finalisation_as_failure(msgstream() <<
                "ERROR: Resource loading was terminated. So, terminating also load of the resource " << parent_finaliser->get_key() << "."
                );
        return;
    }

    ASSUMPTION(key != key_type::get_invalid_key());

    output = nullptr;

    {
        std::lock_guard<std::mutex> const  lock(mutex());

        resources_cache_type::value_type* const  resource_ptr = find_resource(key);
        if (resource_ptr != nullptr)
        {
            output = resource_ptr;
            output->second->inc_ref_count();
            return;
        }

        auto const  iter_and_bool = m_cache.insert({
                key,
                resources_holder_unique_ptr(new resource_holder_type)
                });
        INVARIANT(iter_and_bool.second);

        iter_and_bool.first->second->inc_ref_count();
    }

    finalise_load_on_destroy_ptr const  load_finaliser = finalise_load_on_destroy::create(key, parent_finaliser);
    try
    {
        std::unique_ptr<resource_type> data_ptr(
            new resource_type(load_finaliser, args_for_constructor_of_the_resource...)
            );
        std::lock_guard<std::mutex> const  lock(mutex());
        resources_cache_type::value_type* const  resource_ptr = find_resource(key);
        if (resource_ptr == nullptr)
        {
            load_finaliser->force_finalisation_as_failure(msgstream() <<
                "ERROR: The resource " << key << " was destroyed during the construction of its data."
                );
            return;
        }
        if (resource_ptr->second->resource_ptr() != nullptr)
        {
            load_finaliser->force_finalisation_as_failure(msgstream() <<
                "ERROR: The resource " << key << " was assigned data before their load was finished."
                );
            return;
        }
        resource_ptr->second->set_resource_ptr(data_ptr.release());
        output = resource_ptr;
    }
    catch (std::exception const&  e)
    {
        load_finaliser->force_finalisation_as_failure(msgstream() << "ERROR: " << e.what());
    }
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

        ASSUMPTION(it->second->ref_count() == 0UL);
        destroy_resource_ptr = it->second->resource_ptr();

        m_cache.erase(it);
    }
    if (destroy_resource_ptr != nullptr)
        delete reinterpret_cast<resource_type*>(destroy_resource_ptr);
}


}}

namespace async {


using  key_type = detail::key_type;
using  load_priority_type = detail::resource_load_priority_type;
using  finalise_load_on_destroy = detail::finalise_load_on_destroy;
using  finalise_load_on_destroy_ptr = detail::finalise_load_on_destroy_ptr;
using  ref_counter_type = detail::ref_counter_type;


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
            finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
        : m_data_ptr(nullptr)
    {
        insert_load_request(key, priority, parent_finaliser);
    }

    template<typename... arg_types>
    explicit resource_accessor(
            key_type const&  key,
            finalise_load_on_destroy_ptr const  parent_finaliser,
            arg_types... args_for_constructor_of_the_resource
            )
        : m_data_ptr(nullptr)
    {
        insert_resource(key, parent_finaliser, args_for_constructor_of_the_resource...);
    }

    resource_accessor(resource_accessor<resource_type> const&  other)
        : m_data_ptr(other.m_data_ptr)
    {
        if (m_data_ptr != nullptr)
            m_data_ptr->second->inc_ref_count();
    }

    resource_accessor(resource_accessor<resource_type>&&  other)
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

    resource_accessor<resource_type>& operator=(resource_accessor<resource_type>&&  other)
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

    resource_type const&  resource_const() const
    {
        ASSUMPTION(is_load_finished());
        return *reinterpret_cast<resource_type const*>(m_data_ptr->second->resource_ptr());
    }

    resource_type const&  resource() const
    {
        return resource_const();
    }

    resource_type&  resource()
    {
        ASSUMPTION(is_load_finished());
        return *reinterpret_cast<resource_type*>(m_data_ptr->second->resource_ptr());
    }

    std::string const&  error_message() const
    {
        ASSUMPTION(is_load_finished());
        return m_data_ptr->second->error_message();
    }

    bool  wait_till_load_is_finished() const
    {
        while (!is_load_finished())
        {
            std::this_thread::yield();
        }
        return loaded_successfully();
    }

protected:

    void  insert_load_request(
            key_type const&  key,
            load_priority_type const  priority,
            finalise_load_on_destroy_ptr const  parent_finaliser
            )
    {
        ASSUMPTION(m_data_ptr == nullptr);
        detail::resource_cache::instance().insert_load_request<resource_type>(
                            key,
                            priority,
                            parent_finaliser,
                            m_data_ptr
                            );
    }

    template<typename... arg_types>
    void  insert_resource(
            key_type const&  key,
            finalise_load_on_destroy_ptr const  parent_finaliser,
            arg_types... args_for_constructor_of_the_resource
            )
    {
        ASSUMPTION(m_data_ptr == nullptr);
        detail::resource_cache::instance().insert_resource<resource_type>(
                            key,
                            parent_finaliser,
                            m_data_ptr,
                            args_for_constructor_of_the_resource...
                            );
    }

private:

    detail::resources_cache_type::value_type*  m_data_ptr;
};


template<typename resource_type__>
resource_accessor<resource_type__>  insert_load_request(
        key_type const&  key,
        load_priority_type const  priority,
        finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
        )
{
    resource_accessor<resource_type__>  accessor;
    accessor.insert_load_request(key, priority, parent_finaliser);
    return accessor;
}


template<typename resource_type__, typename... arg_types>
resource_accessor<resource_type__>  insert_resource(
        key_type const&  key,
        finalise_load_on_destroy_ptr const  parent_finaliser,
        arg_types... args_for_constructor_of_the_resource
        )
{
    resource_accessor<resource_type__>  accessor;
    accessor.insert_resource(key, parent_finaliser, args_for_constructor_of_the_resource...);
    return accessor;
}


}

namespace async {


key_type  get_key_of_resource_just_being_loaded();

inline std::string  generate_unique_custom_id(std::string const& custom_prefix)
{ return detail::key_type::generate_unique_custom_id(custom_prefix); }

inline void  clear() { detail::resource_load_planner::instance().clear(); }
inline void  terminate() { detail::terminate(); }
inline bool  is_terminated() { return detail::is_terminated(); }


}

namespace async {


struct cached_resource_info
{
    cached_resource_info(
            LOAD_STATE const  load_state,
            ref_counter_type const  ref_count,
            std::string const&  error_message
            )
        : m_load_state(load_state)
        , m_ref_count(ref_count)
        , m_error_message(error_message)
    {}

    LOAD_STATE  get_load_state() const { return m_load_state; }
    ref_counter_type  get_ref_count() const { return m_ref_count; }

    /// Returns a non-empty string only if 'get_load_state() == LOAD_STATE::FINISHED_WITH_ERROR'
    std::string const&  get_error_message() const { return m_error_message; }

private:
    LOAD_STATE  m_load_state;
    ref_counter_type  m_ref_count;
    std::string  m_error_message;
};


inline bool  operator==(cached_resource_info const&  left, cached_resource_info const&  right)
{
    return  left.get_load_state() == right.get_load_state() &&
            left.get_ref_count() == right.get_ref_count()   &&
            left.get_error_message() == right.get_error_message()   ;
}


inline bool  operator!=(cached_resource_info const&  left, cached_resource_info const&  right)
{
    return  !(left == right);
}


using  statistics_of_cached_resources = std::unordered_map<key_type, cached_resource_info>;


/**
 * Returns an invalid key, i.e. a key which does not represent any resource.
 */
inline key_type const&  get_invalid_key() { return key_type::get_invalid_key(); }


/**
 * The returned key identifies the just being loaded resource (by its 'key_type'). If the
 * key is invalid (i.e. equals to the key returned from 'get_invalid_key()' function), then
 * there is no resource being loaded. Otherwise, the corresponding resource is just being
 * loaded and there is a 'cached_resource_info' record in the filled-in 'output_statistics'
 * map for that key.
 */
key_type  get_statistics_of_cached_resources(statistics_of_cached_resources&  output_statistics);


}

#endif
