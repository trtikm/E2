#ifndef QTGL_TEXTURE_DATABASE_HPP_INCLUDED
#   define QTGL_TEXTURE_DATABASE_HPP_INCLUDED

#   include <qtgl/texture.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <boost/filesystem/path.hpp>
#   include <boost/noncopyable.hpp>
#   include <vector>
#   include <list>
#   include <utility>
#   include <memory>

namespace qtgl {


typedef  std::vector< std::pair<texture_binding_location,boost::filesystem::path> >  texture_files_binding;


}

namespace qtgl { namespace detail {


struct textures_binding_database_record;
typedef std::list<textures_binding_database_record> textures_bindings_list;
struct texture_database;


}}

namespace qtgl {


struct textures_binding_handle : private boost::noncopyable
{
    textures_binding_handle(detail::textures_bindings_list::iterator  data_access);
    ~textures_binding_handle();

    bool  operator==(textures_binding_handle const&  other) const
    {
        return m_data_access == other.m_data_access;
    }

private:
    friend struct  detail::texture_database;
    detail::textures_bindings_list::iterator  m_data_access;
};


typedef std::shared_ptr<textures_binding_handle>  textures_binding_handle_ptr;

textures_binding_handle_ptr  create_textures_binding(texture_files_binding const&  files_binding);

void  make_current(textures_binding_handle_ptr const  texture_binding_handle);


}

#endif
