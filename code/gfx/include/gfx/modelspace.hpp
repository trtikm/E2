#ifndef GFX_DETAIL_MODELSPACE_HPP_INCLUDED
#   define GFX_DETAIL_MODELSPACE_HPP_INCLUDED

#   include <utility/async_resource_load.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <boost/filesystem/path.hpp>
#   include <vector>
#   include <string>
#   include <memory>

namespace gfx { namespace detail {


struct  modelspace_data
{
    modelspace_data(async::finalise_load_on_destroy_ptr const  finaliser);
    ~modelspace_data();

    std::vector<angeo::coordinate_system> const&  coord_systems() const { return m_coord_systems; }

private:

    std::vector<angeo::coordinate_system>  m_coord_systems;
};


}}

namespace gfx {


struct  modelspace : public async::resource_accessor<detail::modelspace_data>
{
    modelspace()
        : async::resource_accessor<detail::modelspace_data>()
    {}

    modelspace(
            boost::filesystem::path const&  path,
            async::load_priority_type const  priority,
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr,
            std::string const&  data_type_name = "gfx::modelspace"
            )
        : async::resource_accessor<detail::modelspace_data>(
            { data_type_name, path.string() },
            priority,
            parent_finaliser
            )
    {}

    std::vector<angeo::coordinate_system> const&  get_coord_systems() const
    { return resource().coord_systems(); }

    std::size_t size() const { return get_coord_systems().size(); }

    angeo::coordinate_system const&  at(natural_32_bit const  index) const { return get_coord_systems().at(index); }

    boost::filesystem::path  get_skeleton_path() const
    {
        return boost::filesystem::path(key().get_unique_id()).parent_path();
    }
};


}

#endif
