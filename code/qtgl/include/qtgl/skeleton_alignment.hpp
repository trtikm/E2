#ifndef QTGL_DETAIL_SKELETON_ALIGNMENT_HPP_INCLUDED
#   define QTGL_DETAIL_SKELETON_ALIGNMENT_HPP_INCLUDED

#   include <qtgl/modelspace.hpp>
#   include <utility/async_resource_load.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <boost/filesystem/path.hpp>

namespace qtgl { namespace detail {


struct  skeleton_alignment_data
{
    skeleton_alignment_data(async::key_type const&  key, async::finalise_load_on_destroy_ptr);
    ~skeleton_alignment_data();

    angeo::coordinate_system const&  get_skeleton_alignment() const { return m_skeleton_alignment; }
    boost::filesystem::path const&  get_skeleton_alignment_path() const { return m_skeleton_alignment_path; }

private:

    angeo::coordinate_system  m_skeleton_alignment;
    boost::filesystem::path  m_skeleton_alignment_path;
};


}}

namespace qtgl {


struct  skeleton_alignment : public async::resource_accessor<detail::skeleton_alignment_data>
{
    skeleton_alignment()
        : async::resource_accessor<detail::skeleton_alignment_data>()
    {}

    explicit skeleton_alignment(boost::filesystem::path const&  path)
        : async::resource_accessor<detail::skeleton_alignment_data>(
            {"qtgl::skeleton_alignment",path.string()},
            1U
            )
    {}

    angeo::coordinate_system const&  get_skeleton_alignment() const { return resource().get_skeleton_alignment(); }
    boost::filesystem::path const&  get_skeleton_alignment_path() const { return resource().get_skeleton_alignment_path(); }
};


}

#endif
