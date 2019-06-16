#ifndef QTGL_DETAIL_KEYFRAME_HPP_INCLUDED
#   define QTGL_DETAIL_KEYFRAME_HPP_INCLUDED

#   include <utility/async_resource_load.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <utility/assumptions.hpp>
#   include <boost/filesystem/path.hpp>
#   include <vector>
#   include <algorithm>
#   include <utility>
#   include <type_traits>

namespace qtgl { namespace detail {


struct  keyframe_data
{
    keyframe_data(async::finalise_load_on_destroy_ptr const  finaliser);
    ~keyframe_data();

    float_32_bit  time_point() const { return m_time_point; }
    std::vector<angeo::coordinate_system> const&  coord_systems() const { return m_coord_systems; }

private:

    float_32_bit  m_time_point;
    std::vector<angeo::coordinate_system>  m_coord_systems;
};


}}

namespace qtgl {


struct  keyframe : public async::resource_accessor<detail::keyframe_data>
{
    keyframe()
        : async::resource_accessor<detail::keyframe_data>()
    {}

    keyframe(
            boost::filesystem::path const&  path,
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
        : async::resource_accessor<detail::keyframe_data>(
            {"qtgl::keyframe",path.string()},
            1U,
            parent_finaliser
            )
    {}

    void  insert_load_request(
            boost::filesystem::path const&  path,
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
    {
        async::resource_accessor<detail::keyframe_data>::insert_load_request(
            { "qtgl::keyframe", path.string() },
            1U,
            parent_finaliser
            );
    }

    float_32_bit  get_time_point() const { return resource().time_point(); }

    std::vector<angeo::coordinate_system> const&  get_coord_systems() const
    { return resource().coord_systems(); }
};


}

namespace qtgl { namespace detail {


struct  keyframes_data
{
    struct  meta_data
    {
        std::vector<angeo::coordinate_system>  m_reference_frames;

        template<typename T>
        struct  record
        {
            using  element_type = T;

            static_assert(std::is_floating_point<element_type>::value || std::is_integral<element_type>::value,
                          "Only numeric types are allowed for the template parameter.");

            std::string  keyword;
            std::vector<element_type>  arguments;

            bool  is_valid() const { return !keyword.empty(); }

            bool  operator==(record const&  other) const;
            bool  operator!=(record const&  other) const { return !(*this == other); }
        };

        template<typename T>
        struct  records
        {
            using  element_type = T;
            std::vector<record<element_type> >  m_records;

            bool  is_valid() const { return !m_records.empty(); }

            bool  operator==(records const&  other) const;
            bool  operator!=(records const&  other) const { return !(*this == other); }
        };

        std::vector<records<float_32_bit> >  m_constraints;
        std::vector<records<float_32_bit> >  m_motion_actions;
        std::vector<records<float_32_bit> >  m_motion_colliders;
        std::vector<records<float_32_bit> >  m_mass_distributions;
        std::vector<records<natural_32_bit> >  m_keyframe_equivalences;
    };

    keyframes_data(async::finalise_load_on_destroy_ptr const  finaliser);
    ~keyframes_data();

    std::vector<keyframe> const&  keyframes() const { return m_keyframes; }
    meta_data const&  get_meta_data() const { return m_meta_data; }

private:

    std::vector<keyframe>  m_keyframes;
    meta_data  m_meta_data;
};


template<>
bool  keyframes_data::meta_data::record<float_32_bit>::operator==(record const&  other) const;


template<typename T>
bool  keyframes_data::meta_data::record<T>::operator==(record<T> const&  other) const
{
    if (keyword != other.keyword || arguments.size() != other.arguments.size())
        return false;
    for (natural_32_bit i = 0U; i != arguments.size(); ++i)
        if (arguments.at(i) != other.arguments.at(i))
            return false;
    return true;
}


template<typename T>
bool  keyframes_data::meta_data::records<T>::operator==(records<T> const&  other) const
{
    if (m_records.size() != other.m_records.size())
        return false;
    for (natural_32_bit i = 0U; i != m_records.size(); ++i)
        if (m_records.at(i) != other.m_records.at(i))
            return false;
    return true;
}


}}

namespace qtgl {


struct  keyframes : public async::resource_accessor<detail::keyframes_data>
{
    using  meta_data = detail::keyframes_data::meta_data;

    keyframes()
        : async::resource_accessor<detail::keyframes_data>()
    {}

    keyframes(
            boost::filesystem::path const&  keyframes_dir,
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
        : async::resource_accessor<detail::keyframes_data>(
            {"qtgl::keyframes", keyframes_dir.string()},
            1U,
            parent_finaliser
            )
    {}

    void  insert_load_request(
            boost::filesystem::path const&  path,
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr)
    {
        async::resource_accessor<detail::keyframes_data>::insert_load_request(
            { "qtgl::keyframes", path.string() },
            1U,
            parent_finaliser
            );
    }

    std::vector<keyframe> const&  get_keyframes() const { return resource().keyframes(); }
    meta_data const&  get_meta_data() const { return resource().get_meta_data(); }

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
};


}

#endif
