#ifndef QTGL_DETAIL_KEYFRAME_HPP_INCLUDED
#   define QTGL_DETAIL_KEYFRAME_HPP_INCLUDED

#   include <utility/async_resource_load.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <utility/assumptions.hpp>
#   include <boost/filesystem/path.hpp>
#   include <vector>
#   include <algorithm>
#   include <utility>

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
    keyframes_data(async::finalise_load_on_destroy_ptr const  finaliser);
    ~keyframes_data();

    std::vector<keyframe> const&  keyframes() const { return m_keyframes; }

private:

    std::vector<keyframe>  m_keyframes;
};


}}

namespace qtgl {


struct  keyframes : public async::resource_accessor<detail::keyframes_data>
{
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


std::pair<std::size_t, std::size_t>  find_indices_of_keyframes_to_interpolate_for_time(
        keyframes const&  keyframes,
        float_32_bit const  time_point
        );


float_32_bit  compute_interpolation_parameter(
        float_32_bit const  time_point,
        float_32_bit const  keyframe_start_time_point,
        float_32_bit const  keyframe_end_time_point
        );


void  compute_coord_system_of_frame_of_keyframe_animation(
        keyframes const&  keyframes,
        std::pair<std::size_t, std::size_t> const  indices_of_keyframe_to_interpolate,
        std::size_t const  index_of_coord_system_in_keyframes,
        float_32_bit const  interpolation_param, // in range <0,1>
        angeo::coordinate_system&  output
        );


void  compute_frame_of_keyframe_animation(
        keyframes const&  keyframes,
        std::pair<std::size_t, std::size_t> const  indices_of_keyframe_to_interpolate,
        float_32_bit const  interpolation_param, // in range <0,1>
        std::vector<angeo::coordinate_system>&  output
        );


void  compute_frame_of_keyframe_animation(
        keyframes const&  keyframes,
        std::pair<std::size_t, std::size_t> const  indices_of_keyframe_to_interpolate,
        float_32_bit const  interpolation_param, // in range <0,1>
        std::vector<matrix44>&  output
        );


void  compute_frame_of_keyframe_animation(
        keyframes const&  keyframes,
        float_32_bit const  time_point,
        std::vector<angeo::coordinate_system>&  output
        );


void  compute_frame_of_keyframe_animation(
        keyframes const&  keyframes,
        float_32_bit const  time_point,
        std::vector<matrix44>&  output  // the results will be composed with the current data.
        );


void  compute_frame_of_keyframe_animation(
        keyframes const&  keyframes,
        matrix44 const&  target_space, // typically, the target space is a camera space, i.e. MATRIX_FROM_MODEL_TO_CAMERA
        float_32_bit const  time_point,
        std::vector<matrix44>&  output // old content won't be used and it will be owerwritten by the computed data.
        );


float_32_bit  update_animation_time(
        float_32_bit  current_animation_time_point,
        float_32_bit const  time_delta,
        float_32_bit const  keyframes_start_time,
        float_32_bit const  keyframes_end_time
        );


}

#endif
