#ifndef GFX_DETAIL_KEYFRAME_HPP_INCLUDED
#   define GFX_DETAIL_KEYFRAME_HPP_INCLUDED

#   include <utility/async_resource_load.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <utility/assumptions.hpp>
#   include <filesystem>
#   include <vector>
#   include <unordered_map>
#   include <algorithm>
#   include <memory>
#   include <utility>
#   include <type_traits>

namespace gfx { namespace detail {


struct  keyframes_data;


struct  keyframe_data
{
    using  translation_map_ptr = std::shared_ptr<std::unordered_map<natural_32_bit, natural_32_bit> const>;

    keyframe_data(async::finalise_load_on_destroy_ptr const  finaliser);
    ~keyframe_data();

    float_32_bit  time_point() const { return m_time_point; }
    std::vector<angeo::coordinate_system> const&  coord_systems() const { return m_coord_systems; }
    translation_map_ptr  from_indices_to_bones() const { return m_from_indices_to_bones; };
    translation_map_ptr  from_bones_to_indices() const { return m_from_bones_to_indices; };

private:

    friend struct  keyframes_data;

    float_32_bit  m_time_point;
    std::vector<angeo::coordinate_system>  m_coord_systems;
    translation_map_ptr  m_from_indices_to_bones;
    translation_map_ptr  m_from_bones_to_indices;
};


}}

namespace gfx {


struct  keyframe : public async::resource_accessor<detail::keyframe_data>
{
    using  translation_map_ptr = detail::keyframe_data::translation_map_ptr;

    keyframe()
        : async::resource_accessor<detail::keyframe_data>()
    {}

    keyframe(
            std::filesystem::path const&  path,
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
        : async::resource_accessor<detail::keyframe_data>(
            {"gfx::keyframe",path.string()},
            1U,
            parent_finaliser
            )
    {}

    void  insert_load_request(
            std::filesystem::path const&  path,
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
    {
        async::resource_accessor<detail::keyframe_data>::insert_load_request(
            { "gfx::keyframe", path.string() },
            1U,
            parent_finaliser
            );
    }

    float_32_bit  get_time_point() const { return resource().time_point(); }

    std::vector<angeo::coordinate_system> const&  get_coord_systems() const
    { return resource().coord_systems(); }

    translation_map_ptr  from_indices_to_bones() const { return resource().from_indices_to_bones(); };
    translation_map_ptr  from_bones_to_indices() const { return resource().from_bones_to_indices(); };
};


}

namespace gfx { namespace detail {


struct  keyframes_data
{
    using  translation_map = std::unordered_map<natural_32_bit, natural_32_bit>;
    using  translation_map_ptr = std::shared_ptr<std::unordered_map<natural_32_bit, natural_32_bit> const>;

    keyframes_data(async::finalise_load_on_destroy_ptr const  finaliser);
    ~keyframes_data();

    std::vector<keyframe> const&  keyframes() const { return m_keyframes; }
    translation_map_ptr  from_indices_to_bones() const { return m_from_indices_to_bones; };
    translation_map_ptr  from_bones_to_indices() const { return m_from_bones_to_indices; };

private:

    std::vector<keyframe>  m_keyframes;
    translation_map_ptr  m_from_indices_to_bones;
    translation_map_ptr  m_from_bones_to_indices;
};


}}

namespace gfx {


struct  keyframes : public async::resource_accessor<detail::keyframes_data>
{
    using  translation_map_ptr = detail::keyframes_data::translation_map_ptr;

    keyframes()
        : async::resource_accessor<detail::keyframes_data>()
    {}

    keyframes(
            std::filesystem::path const&  keyframes_dir,
            async::load_priority_type const  priority = 1U,
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
        : async::resource_accessor<detail::keyframes_data>(
            {"gfx::keyframes", keyframes_dir.string()},
            priority,
            parent_finaliser
            )
    {}

    void  insert_load_request(
            std::filesystem::path const&  path,
            async::load_priority_type const  priority = 1U,
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr)
    {
        async::resource_accessor<detail::keyframes_data>::insert_load_request(
            { "gfx::keyframes", path.string() },
            priority,
            parent_finaliser
            );
    }

    std::vector<keyframe> const&  get_keyframes() const { return resource().keyframes(); }

    float_32_bit  start_time_point() const { return get_keyframes().front().get_time_point(); }
    float_32_bit  end_time_point() const { return get_keyframes().back().get_time_point(); }
    float_32_bit  duration() const { return end_time_point() - start_time_point(); }

    std::size_t  num_keyframes() const { return get_keyframes().size(); }
    keyframe const&  keyframe_at(std::size_t const  index) const { return get_keyframes().at(index); }

    float_32_bit  time_point_at(std::size_t const  index) const { return keyframe_at(index).get_time_point(); }
    std::size_t  num_coord_systems_per_keyframe() const { return get_keyframes().front().get_coord_systems().size(); }
    angeo::coordinate_system const&  coord_system_at(
        std::size_t const  keyframe_index,
        std::size_t const  coord_system_index) const
    {
        return keyframe_at(keyframe_index).get_coord_systems().at(coord_system_index);
    }

    translation_map_ptr  from_indices_to_bones() const { return resource().from_indices_to_bones(); };
    translation_map_ptr  from_bones_to_indices() const { return resource().from_bones_to_indices(); };
};


}

#endif
