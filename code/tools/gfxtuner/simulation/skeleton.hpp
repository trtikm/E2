#ifndef E2_TOOL_GFXTUNER_SKELETON_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_SKELETON_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <scene/scene_node_id.hpp>
#   include <utility/async_resource_load.hpp>
#   include <boost/filesystem/path.hpp>
#   include <vector>
#   include <string>

namespace detail {


struct  skeleton_data
{
    skeleton_data(async::finalise_load_on_destroy_ptr const  finaliser);
    ~skeleton_data();

    std::vector<scn::scene_node_id> const&  get_relative_node_ids() const { return  m_relative_node_ids; }

private:

    std::vector<scn::scene_node_id>  m_relative_node_ids;
};


}


struct  skeleton : public async::resource_accessor<detail::skeleton_data>
{
    skeleton()
        : async::resource_accessor<detail::skeleton_data>()
    {}

    skeleton(
            boost::filesystem::path const&  skeleton_directory,
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
        : async::resource_accessor<detail::skeleton_data>(
            {"gfxtuner::skeleton", skeleton_directory.string()},
            1U,
            parent_finaliser
            )
    {}

    boost::filesystem::path  get_skeleton_directory() const { return key().get_unique_id(); }
    std::vector<scn::scene_node_id> const&  get_relative_node_ids() const { return resource().get_relative_node_ids(); }

};


#endif
