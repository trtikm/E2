#ifndef GFX_DETAIL_SKELETON_ALIGNMENT_HPP_INCLUDED
#   define GFX_DETAIL_SKELETON_ALIGNMENT_HPP_INCLUDED

#   include <gfx/modelspace.hpp>
#   include <utility/async_resource_load.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <filesystem>

namespace gfx { namespace detail {


struct  skeleton_alignment_data
{
    skeleton_alignment_data(async::finalise_load_on_destroy_ptr const  finaliser);
    ~skeleton_alignment_data();

    angeo::coordinate_system const&  get_skeleton_alignment() const { return m_skeleton_alignment; }
    std::filesystem::path const&  get_skeleton_alignment_path() const { return m_skeleton_alignment_path; }

private:

    angeo::coordinate_system  m_skeleton_alignment;
    std::filesystem::path  m_skeleton_alignment_path;
};


}}

namespace gfx {


struct  skeleton_alignment : public async::resource_accessor<detail::skeleton_alignment_data>
{
    skeleton_alignment()
        : async::resource_accessor<detail::skeleton_alignment_data>()
    {}

    explicit skeleton_alignment(
            std::filesystem::path const&  path,
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
        : async::resource_accessor<detail::skeleton_alignment_data>(
            {"gfx::skeleton_alignment",path.string()},
            1U,
            parent_finaliser
            )
    {}

    angeo::coordinate_system const&  get_skeleton_alignment() const { return resource().get_skeleton_alignment(); }
    std::filesystem::path const&  get_skeleton_alignment_path() const { return resource().get_skeleton_alignment_path(); }
};


}

#endif
