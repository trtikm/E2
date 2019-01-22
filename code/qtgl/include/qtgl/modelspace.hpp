#ifndef QTGL_DETAIL_MODELSPACE_HPP_INCLUDED
#   define QTGL_DETAIL_MODELSPACE_HPP_INCLUDED

#   include <utility/async_resource_load.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <boost/filesystem/path.hpp>
#   include <vector>
#   include <string>
#   include <memory>

namespace qtgl { namespace detail {


struct  modelspace_data
{
    modelspace_data(async::finalise_load_on_destroy_ptr const  finaliser);
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

    modelspace(
            boost::filesystem::path const&  path,
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
        : async::resource_accessor<detail::modelspace_data>(
            {"qtgl::modelspace",path.string()},
            1U,
            parent_finaliser
            )
    {}

    std::vector<angeo::coordinate_system> const&  get_coord_systems() const
    { return resource().coord_systems(); }

    boost::filesystem::path  get_skeleton_path() const
    {
        return boost::filesystem::path(key().get_unique_id()).parent_path();
    }
};


/**
 * The 'pose_file_pathname' is a path-name of a 'pose.txt' file.
 *
 * Each loaded coord. system correspond to one bone, i.e a bone is an index to the vector.
 * The bones are in the same topological order as the related file 'names.txt' and 'parents.txt'
 * in the same directory (however, these two files are not used here; see ai::load_skeleton for more
 * details).
 *
 * The function returns the empty string on succeess and error message otherwise.
 */
std::string  load_modelspace_coordinate_systems(
        boost::filesystem::path const&  pose_file_pathname,
        std::vector<angeo::coordinate_system>&  output_coord_systems
        );


}

#endif
