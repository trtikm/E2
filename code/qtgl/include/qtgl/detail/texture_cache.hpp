#ifndef QTGL_DETAIL_TEXTURE_CACHE_HPP_INCLUDED
#   define QTGL_DETAIL_TEXTURE_CACHE_HPP_INCLUDED

#   include <qtgl/texture.hpp>
#   include <unordered_map>
#   include <vector>
#   include <memory>
#   include <mutex>

namespace qtgl { namespace detail {


struct texture_cache
{
    static texture_cache&  instance();

    void clear(bool destroy_also_dummy_texture = false);

    void  insert_load_request(
            boost::filesystem::path const&  texture_file  //!< This is path to a TEXTURE file,
                                                          //!< so it is NOT a path to an IMAGE file!!
            );
    void  insert_load_request(texture_properties_ptr const  props);

    bool  insert(texture_ptr const  texture);

    texture_properties_ptr  find(
            boost::filesystem::path const&  texture_file  //!< This is path to a TEXTURE file,
                                                          //!< so it is NOT a path to an IMAGE file!!
            );
    std::weak_ptr<texture const>  find(texture_properties_ptr const  props);

    std::weak_ptr<texture const>  get_dummy_texture() const noexcept { return m_dummy_texture; }

    void  process_pending_textures();

    void  cached(std::vector< std::pair<boost::filesystem::path,texture_properties_ptr> >&  output);
    void  failed(std::vector< std::pair<boost::filesystem::path,std::string> >&  output);

private:
    texture_cache();

    texture_cache(texture_cache const&) = delete;
    texture_cache& operator=(texture_cache const&) = delete;

    void  receiver_props(boost::filesystem::path const&  texture_file,
                         texture_properties_ptr const  texture_props,
                         std::string const&  error_message
                         );
    void  receiver(texture_image_properties& image_props, texture_properties_ptr const  texture_props);

    std::unordered_map<
            boost::filesystem::path,
            texture_properties_ptr,
            size_t(*)(boost::filesystem::path const&)
            >  m_cached_properties;
    std::unordered_map<
            texture_properties_ptr,
            texture_ptr,
            size_t(*)(texture_properties_ptr const),
            bool(*)(texture_properties_ptr const,texture_properties_ptr const)
            >  m_cached_textures;
    std::vector< std::pair<boost::filesystem::path,texture_properties_ptr> >  m_pending_properties;
    std::vector< std::pair<texture_image_properties,texture_properties_ptr> >  m_pending_textures;
    std::unordered_map<
            boost::filesystem::path,
            std::string,
            size_t(*)(boost::filesystem::path const&)
            >  m_failed_properties;
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
