#ifndef QTGL_DETAIL_RESOURCE_LOADER_HPP_INCLUDED
#   define QTGL_DETAIL_RESOURCE_LOADER_HPP_INCLUDED

#   include <qtgl/texture.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <boost/filesystem/path.hpp>
#   include <functional>
#   include <mutex>

namespace qtgl { namespace detail {


//enum struct resource_type : natural_8_bit
//{
//    TEXTURE
//};

struct resource_loader
{
    static  resource_loader&  instance();

    void clear();

    void  insert(texture_properties_ptr const  props, std::function<void(texture_ptr)> const&  receiver);

private:
    resource_loader();

    resource_loader(resource_loader const&) = delete;
    resource_loader& operator=(resource_loader const&) = delete;

    mutable std::mutex  m_mutex;
};


}}

#endif
