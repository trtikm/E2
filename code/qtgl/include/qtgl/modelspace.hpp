#ifndef QTGL_DETAIL_MODELSPACE_HPP_INCLUDED
#   define QTGL_DETAIL_MODELSPACE_HPP_INCLUDED

#   include <qtgl/detail/modelspace_cache.hpp>

namespace qtgl {


struct  modelspace
{
    using  data_ptr = detail::modelspace_data;

    modelspace(boost::filesystem::path const&  path)
        : m_handle(detail::modelspace_cache::instance().insert_load_request(path,1U))
    {}

    boost::filesystem::path const&  path() const { return m_handle.key(); }
    bool  is_load_finished() const { return m_handle.is_load_finished(); }

    data_ptr const*  data() const { return m_handle.resource_ptr(); }

private:

    detail::modelspace_cache::resource_handle  m_handle;
};


}

#endif
