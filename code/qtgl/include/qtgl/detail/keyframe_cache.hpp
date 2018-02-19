#ifndef QTGL_DETAIL_KEYFRAME_CACHE_HPP_INCLUDED
#   define QTGL_DETAIL_KEYFRAME_CACHE_HPP_INCLUDED

#   include <qtgl/detail/resource_cache.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <vector>

namespace qtgl { namespace detail {


struct  keyframe_data
{
    keyframe_data(boost::filesystem::path const&  pathname);

    float_32_bit  time_point() const { return m_time_point; }
    std::vector<angeo::coordinate_system> const&  coord_systems() const { return m_coord_systems; }

private:

    float_32_bit  m_time_point;
    std::vector<angeo::coordinate_system>  m_coord_systems;
};


}}

#endif
