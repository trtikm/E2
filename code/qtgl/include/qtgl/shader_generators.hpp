#ifndef QTGL_SHADER_GENERATORS_HPP_INCLUDED
#   define QTGL_SHADER_GENERATORS_HPP_INCLUDED

#   include <qtgl/shader.hpp>
#   include <qtgl/glapi.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <boost/filesystem/path.hpp>

namespace qtgl { namespace vertex_program_generators { namespace transform_3D_vertices {


boost::filesystem::path  imaginary_image_path() noexcept;
vertex_program_properties const&  properties();
vertex_program_ptr  create();


}}}

#endif
