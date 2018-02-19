#ifndef QTGL_DETAIL_MODELSPACE_HPP_INCLUDED
#   define QTGL_DETAIL_MODELSPACE_HPP_INCLUDED

#   include <utility/async_resource_load.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <boost/filesystem/path.hpp>
#   include <vector>
#   include <memory>

namespace qtgl { namespace detail {


struct  modelspace_data
{
    modelspace_data(boost::filesystem::path const&  path);

    std::vector<angeo::coordinate_system> const&  coord_systems() const { return m_coord_systems; }

private:

    std::vector<angeo::coordinate_system>  m_coord_systems;
};


}}

namespace qtgl {


struct  modelspace : public async_resource_accessor_base<detail::modelspace_data>
{
    using  modelspace_ptr = std::shared_ptr<modelspace const>;

    static modelspace_ptr  create(boost::filesystem::path const&  path)
    { return modelspace_ptr(new modelspace(path)); }

    explicit modelspace(boost::filesystem::path const&  path)
        : async_resource_accessor_base<detail::modelspace_data>(path,1U)
    {}

    std::vector<angeo::coordinate_system> const&  get_coord_systems() const
    { return resource().coord_systems(); }
};


using  modelspace_ptr = modelspace::modelspace_ptr;


void  apply_modelspace_to_frame_of_keyframe_animation(
        modelspace const&  modelspace,
        std::vector<matrix44>&  frame  // the results will be composed with the current data.
        );


}

#endif
