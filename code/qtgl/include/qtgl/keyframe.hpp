#ifndef QTGL_DETAIL_KEYFRAME_HPP_INCLUDED
#   define QTGL_DETAIL_KEYFRAME_HPP_INCLUDED

#   include <qtgl/detail/keyframe_cache.hpp>

namespace qtgl {


struct  keyframe
{
    using  data_ptr = detail::keyframe_data;

    explicit keyframe(boost::filesystem::path const&  path)
        : m_handle(detail::keyframe_cache::instance().insert_load_request(path,1U))
    {}

    boost::filesystem::path const&  path() const { return m_handle.key(); }
    bool  is_load_finished() const { return m_handle.is_load_finished(); }

    data_ptr const*  data() const { return m_handle.resource_ptr(); }
    std::string const*  error_message() const  { return m_handle.error_message(); }

private:

    detail::keyframe_cache::resource_handle  m_handle;
};


}

#endif
