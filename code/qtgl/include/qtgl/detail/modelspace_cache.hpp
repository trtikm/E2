#ifndef QTGL_DETAIL_MODELSPACE_CACHE_HPP_INCLUDED
#   define QTGL_DETAIL_MODELSPACE_CACHE_HPP_INCLUDED

#   include <qtgl/detail/resource_cache.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <vector>

namespace qtgl { namespace detail {


struct  modelspace_data
{
    modelspace_data(boost::filesystem::path const&  path);

    std::vector<angeo::coordinate_system> const&  coord_systems() const { return m_coord_systems; }

private:

    std::vector<angeo::coordinate_system>  m_coord_systems;
};


using  modelspace_cache = resource_cache<modelspace_data>;


}}

#endif
