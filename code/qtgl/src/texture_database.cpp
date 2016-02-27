#include <qtgl/texture_database.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <mutex>

namespace qtgl { namespace detail {


struct textures_binding_database_record
{
    textures_binding_database_record(textures_binding_ptr const  textures_binding,
                                     texture_files_binding const&  files_binding)
        : m_ref_count(1ULL)
        , m_textures_binding(textures_binding)
        , m_files_binding(files_binding)
    {
        ASSUMPTION(m_textures_binding.operator bool());
    }

    natural_64_bit  ref_count() const { return m_ref_count; }
    textures_binding_ptr  textures_binding() const { return m_textures_binding; }
    texture_files_binding const&  files_binding() const { return m_files_binding; }

    void  decrement_ref_count()
    {
        ASSUMPTION(ref_count() > 0ULL);
        --m_ref_count;
    }

private:
    natural_64_bit  m_ref_count;
    textures_binding_ptr  m_textures_binding;
    texture_files_binding  m_files_binding;
};

struct texture_database
{
    static texture_database&  instance();

    textures_binding_handle_ptr  create(texture_files_binding const&  files_binding);

    void  make_current(textures_binding_handle_ptr const  handle);

private:

    friend struct  qtgl::textures_binding_handle;

    texture_database();

    void  release(detail::textures_bindings_list::iterator const  it);

    std::unordered_map<std::string,std::vector<detail::textures_bindings_list::iterator> >  m_texture_usages;
    detail::textures_bindings_list  m_textures_bindings;
    std::mutex  m_mutex;
};


}}

namespace qtgl {


textures_binding_handle::textures_binding_handle(detail::textures_bindings_list::iterator  data_access)
    : m_data_access(data_access)
{}

textures_binding_handle::~textures_binding_handle()
{
    detail::texture_database::instance().release(m_data_access);
}

textures_binding_handle_ptr  create_textures_binding(texture_files_binding const&  files_binding)
{
    return detail::texture_database::instance().create(files_binding);
}

void  make_current(textures_binding_handle_ptr const  texture_binding_handle)
{
    detail::texture_database::instance().make_current(texture_binding_handle);
}


}

namespace qtgl { namespace detail {


texture_database&  texture_database::instance()
{
    static texture_database  database;
    return database;
}

texture_database::texture_database()
    : m_texture_usages()
    , m_textures_bindings()
    , m_mutex()
{}

textures_binding_handle_ptr  texture_database::create(texture_files_binding const&  files_binding)
{
    std::lock_guard<std::mutex> const  lock(m_mutex);

//    std::vector< std::pair<natural_64_bit,texture_ptr> >  new_textures;
//    std::vector< std::pair<natural_64_bit,std::vector<detail::textures_bindings_list::iterator>*> >  bindings_table;
//    for (natural_64_bit  i = 0ULL; i < files_binding.size(); ++i)
//    {
//        auto const  usage_iter = m_texture_usages.find(files_binding.at(i).second.string());
//        if (usage_iter == m_texture_usages.end())
//        {
//            new_textures.push_back({i,create_texture(files_binding.at(i).second)});
//        }
//        else
//            bindings_table.push_back({i,&usage_iter->second});
//    }
//    if (!new_textures.empty())
//    {

//        return;
//    }

    return textures_binding_handle_ptr{};
}

void  texture_database::release(detail::textures_bindings_list::iterator const  it)
{
    std::lock_guard<std::mutex> const  lock(m_mutex);

    it->decrement_ref_count();
    if (it->ref_count() != 0ULL)
        return;

    for (auto const& file_binding : it->files_binding())
    {
        auto const  usage_iter = m_texture_usages.find(file_binding.second.string());
        INVARIANT(usage_iter != m_texture_usages.end());
        auto const  it_pos_iter = std::find(usage_iter->second.begin(),usage_iter->second.end(),it);
        INVARIANT(it_pos_iter != usage_iter->second.end());
        usage_iter->second.erase(it_pos_iter);
        if (usage_iter->second.empty())
            m_texture_usages.erase(usage_iter);
    }

    m_textures_bindings.erase(it);
}

void  texture_database::make_current(textures_binding_handle_ptr const  handle)
{
    ASSUMPTION(handle.operator bool());
    qtgl::make_current(handle->m_data_access->textures_binding());
}


}}
