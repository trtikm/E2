#ifndef QTGL_DETAIL_TEXTURE_CACHE_HPP_INCLUDED
#   define QTGL_DETAIL_TEXTURE_CACHE_HPP_INCLUDED

#   include <qtgl/texture.hpp>
#   include <unordered_map>
#   include <memory>
#   include <mutex>

namespace qtgl { namespace detail {


struct texture_cache
{
    static texture_cache&  instance();

    void clear(bool destroy_also_dummy_texture = false);

    void  insert_load_request(texture_properties_ptr const  props);

    bool  insert(texture_ptr const  texture);
    std::weak_ptr<texture const>  find(texture_properties_ptr const  props) const;

    std::weak_ptr<texture const>  get_dummy_texture() const noexcept { return m_dummy_texture; }

private:
    texture_cache();

    texture_cache(texture_cache const&) = delete;
    texture_cache& operator=(texture_cache const&) = delete;

    std::unordered_map<
            texture_properties_ptr,
            texture_ptr,
            size_t(*)(texture_properties_ptr const),
            bool(*)(texture_properties_ptr const,texture_properties_ptr const)
            >  m_cached_textures;
    mutable std::mutex  m_mutex;
    texture_ptr  m_dummy_texture;


//    texture_cache(natural_32_bit const  max_num_textures,
//                  natural_32_bit const  max_summary_size_of_all_textures);
//    struct priority
//    {
//    };
//    std::vector< std::pair<priority,texture_pathname> >  m_heap_of_priorities;
//    natural_32_bit  m_summary_size_of_all_textures;
//    natural_32_bit  m_max_num_textures;
//    natural_32_bit  m_max_summary_size_of_all_textures;

};


}}

#endif
