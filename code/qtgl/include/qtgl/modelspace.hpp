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
    modelspace_data(boost::filesystem::path const&  path, async::finalise_load_on_destroy_ptr);
    ~modelspace_data();

    std::vector<angeo::coordinate_system> const&  coord_systems() const { return m_coord_systems; }

private:

    std::vector<angeo::coordinate_system>  m_coord_systems;
};


}}

namespace qtgl {


struct  modelspace : public async::resource_accessor<detail::modelspace_data>
{
    modelspace()
        : async::resource_accessor<detail::modelspace_data>()
    {}

    explicit modelspace(boost::filesystem::path const&  path)
        : async::resource_accessor<detail::modelspace_data>(path.string(),1U)
    {}

    std::vector<angeo::coordinate_system> const&  get_coord_systems() const
    { return resource().coord_systems(); }
};


void  apply_modelspace_to_frame_of_keyframe_animation(
        modelspace const&  modelspace,
        std::vector<matrix44>&  frame  // the results will be composed with the current data.
        );


}

#endif
