#ifndef GFX_DETAIL_KEYFRAME_CACHE_HPP_INCLUDED
#   define GFX_DETAIL_KEYFRAME_CACHE_HPP_INCLUDED

#   include <gfx/detail/resource_cache.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <vector>

namespace gfx { namespace detail {


struct  keyframe_data
{
    keyframe_data(std::filesystem::path const&  path);

    float_32_bit  time_point() const { return m_time_point; }
    std::vector<angeo::coordinate_system> const&  coord_systems() const { return m_coord_systems; }

private:

    float_32_bit  m_time_point;
    std::vector<angeo::coordinate_system>  m_coord_systems;
};


using  keyframe_cache = resource_cache<keyframe_data>;


}}

namespace gfx {


struct  keyframe
{
    using  data_ptr = detail::keyframe_data;

    keyframe(std::filesystem::path const&  path)
        : m_handle(detail::keyframe_cache::instance().insert_load_request(path,1U))
    {}

    std::filesystem::path const&  path() const { return m_handle.key(); }
    bool  is_load_finished() const { return m_handle.is_load_finished(); }

    data_ptr const*  data() const { return m_handle.resource_ptr(); }

private:

    detail::keyframe_cache::resource_handle  m_handle;
};


}

#endif
