#include <qtgl/keyframe.hpp>
#include <utility/read_line.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/canonical_path.hpp>
#include <utility/msgstream.hpp>
#include <boost/filesystem.hpp>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>

namespace qtgl { namespace detail {


keyframe_data::keyframe_data(async::key_type const&  key, async::finalise_load_on_destroy_ptr)
    : m_time_point(0.0)
    , m_coord_systems()
{
    TMPROF_BLOCK();

    boost::filesystem::path const  pathname = key.get_unique_id();

    if (!boost::filesystem::exists(pathname))
        throw std::runtime_error(msgstream() << "The passed file '" << pathname << "' does not exist.");

    if (boost::filesystem::file_size(pathname) < 4ULL)
        throw std::runtime_error(msgstream() << "The passed file '" << pathname << "' is not a qtgl file (wrong size).");

    std::ifstream  istr(pathname.string(),std::ios_base::binary);
    if (!istr.good())
        throw std::runtime_error(msgstream() << "Cannot open the keyframe file '" << pathname << "'.");

    {
        std::string  line;
        if (!read_line(istr,line))
            throw std::runtime_error(msgstream() << "Cannot read time point in the file '" << pathname << "'.");
        std::istringstream istr(line);
        istr >> m_time_point;
        if (m_time_point < 0.0f)
            throw std::runtime_error(msgstream() << "The time point in the file '" << pathname << "' is negative.");
    }

    natural_32_bit  num_coord_systems;
    {
        std::string  line;
        if (!read_line(istr,line))
            throw std::runtime_error(msgstream() << "Cannot read time point in the file '" << pathname << "'.");
        std::istringstream istr(line);
        istr >> num_coord_systems;
        if (num_coord_systems == 0U)
            throw std::runtime_error(msgstream() << "The the file '" << pathname << "' does not contain any coodinate system.");
    }

    for (natural_32_bit  i = 0U; i != num_coord_systems; ++i)
    {
        vector3  position;
        {
            for (natural_32_bit  j = 0U; j != 3U; ++j)
            {
                std::string  line;
                if (!read_line(istr,line))
                    throw std::runtime_error(msgstream() << "Cannot read coordinate #" << j
                                                            << " of position of coodinate system #" << i
                                                            << " in the file '" << pathname << "'.");
                std::istringstream istr(line);
                istr >> position(j);
            }
        }
        quaternion  orientation;
        {
            float_32_bit  coords[4];
            float_32_bit  sum = 0.0f;
            for (natural_32_bit  j = 0U; j != 4U; ++j)
            {
                std::string  line;
                if (!read_line(istr,line))
                    throw std::runtime_error(msgstream() << "Cannot read coordinate #" << j
                                                            << " of orientation of coodinate system #" << i
                                                            << " in the file '" << pathname << "'.");
                std::istringstream istr(line);
                istr >> coords[j];
                sum += coords[j] * coords[j];
            }
            if (std::fabsf(1.0f - sum) > 1e-2f)
                throw std::runtime_error(msgstream() << "The orientation of coodinate system #" << i
                                                        << " in the file '" << pathname << "' is not normalised.");
            orientation = quaternion(coords[0],coords[1],coords[2],coords[3]);
            orientation.normalize();
        }
        m_coord_systems.push_back({position,orientation});
    }
}


keyframe_data::~keyframe_data()
{
    TMPROF_BLOCK();
}


}}

namespace qtgl { namespace detail {


keyframes_data::keyframes_data(
        async::key_type const&  key,
        async::finalise_load_on_destroy_ptr  finaliser)
    : m_keyframes()
{
    TMPROF_BLOCK();

    boost::filesystem::path const  keyframes_dir = key.get_unique_id();

    if (!boost::filesystem::is_directory(keyframes_dir))
        throw std::runtime_error("Cannot access the directory of keyframes: " + keyframes_dir.string());

    std::shared_ptr< std::vector<boost::filesystem::path> >  keyframe_pathnames(
            new std::vector<boost::filesystem::path>()
            );
    for (boost::filesystem::directory_entry const&  entry : boost::filesystem::directory_iterator(keyframes_dir))
    {
        std::string const  filename = entry.path().filename().string();
        std::string const  extension = entry.path().filename().extension().string();
        if (filename.find("keyframe") == 0UL && extension == ".txt")
            keyframe_pathnames->push_back(canonical_path(entry.path()));
    }

    if (keyframe_pathnames->empty())
        throw std::runtime_error("There is no keyframe file in the directory: " + keyframes_dir.string());

    m_keyframes.resize(keyframe_pathnames->size());

    struct local
    {
        static void  on_keyframe_loaded(
                std::shared_ptr< std::vector<boost::filesystem::path> > const  keyframe_pathnames,
                natural_64_bit  index,
                async::finalise_load_on_destroy_ptr  finaliser,
                std::vector<keyframe>&  keyframes
                )
        {
            ASSUMPTION(index < keyframes.size());
            ASSUMPTION(keyframes.at(index).is_load_finished());

            if (keyframes.at(index).get_load_state() != async::LOAD_STATE::FINISHED_SUCCESSFULLY)
            {
                // Oh no! We failed to load the keyframe at index 'index'!
                // So, let's also fail the load of the whole 'keyframe_data' resource.

                finaliser->force_finalisation_as_failure(
                        "Load of keyframe '" + keyframe_pathnames->at(index).string() + "' has FAILED!"
                        );
                return;
            }

            // Let's load the next keyframe (if any remains)

            ++index;
            if (index == keyframes.size())
            {
                // All keyframes are loaded! So, let's check for their consystency and sort them by time.

                if (![&keyframes]() -> bool {
                        auto const  num_coord_systems_per_keyframe = keyframes.front().get_coord_systems().size();
                        for (std::size_t i = 0ULL; i != keyframes.size(); ++i)
                            if (keyframes.at(i).get_coord_systems().size() != num_coord_systems_per_keyframe)
                                return false;
                        return true;
                        }())
                {
                    finaliser->force_finalisation_as_failure(
                        "Loaded of keyframes have different counts of coordinate systems (inconsystent animation)."
                        );
                    return;
                }

                std::sort(
                    keyframes.begin(),
                    keyframes.end(),
                    [](keyframe const& left, keyframe const& right) -> bool {
                        return left.get_time_point() < right.get_time_point();
                        }
                    );

                return;
            }
    
            ASSUMPTION(keyframe_pathnames->size() == keyframes.size());

            // Let's load the next keyframe.

            keyframes.at(index).insert_load_request(
                    keyframe_pathnames->at(index).string(),
                    std::bind(&on_keyframe_loaded, keyframe_pathnames, index, finaliser, std::ref(keyframes))
                    );
        }
    };

    // We load individual keyframes one by one, starting from the first one in the array (at index 0).
    // NOTE: All remaining keyframes (at remaining indices) are loaded in function 'local::on_keyframe_loaded'.

    m_keyframes.at(0UL).insert_load_request(
            keyframe_pathnames->at(0UL).string(),
            std::bind(&local::on_keyframe_loaded, keyframe_pathnames, 0UL, finaliser, std::ref(m_keyframes))
            );
}


keyframes_data::~keyframes_data()
{
    TMPROF_BLOCK();
}


}}

namespace qtgl {


std::pair<std::size_t, std::size_t>  find_indices_of_keyframes_to_interpolate_for_time(
        keyframes const&  keyframes,
        float_32_bit const  time_point
        )
{
    ASSUMPTION(keyframes.start_time_point() <= time_point && time_point <= keyframes.end_time_point());
    std::size_t  keyframe_index = 0ULL;
    while (keyframe_index + 1ULL < keyframes.num_keyframes() &&
           time_point >= keyframes.time_point_at(keyframe_index + 1ULL))
        ++keyframe_index;
    std::size_t const  keyframe_succ_index = keyframe_index + (keyframe_index + 1ULL < keyframes.num_keyframes() ? 1ULL : 0ULL);
    INVARIANT(keyframe_succ_index < keyframes.num_keyframes());
    INVARIANT(time_point >= keyframes.time_point_at(keyframe_index));
    INVARIANT(keyframe_index == keyframe_succ_index || time_point < keyframes.time_point_at(keyframe_succ_index));
    return{ keyframe_index , keyframe_succ_index };
}


float_32_bit  compute_interpolation_parameter(
        float_32_bit const  time_point,
        float_32_bit const  keyframe_start_time_point,
        float_32_bit const  keyframe_end_time_point
        )
{
    ASSUMPTION(keyframe_start_time_point <= time_point && time_point <= keyframe_end_time_point);
    float_32_bit  interpolation_param;
    {
        float_32_bit const  dt = keyframe_end_time_point - keyframe_start_time_point;
        if (dt < 0.0001f)
            interpolation_param = 0.0f;
        else
            interpolation_param = (time_point - keyframe_start_time_point) / dt;
        interpolation_param = std::max(0.0f, std::min(interpolation_param, 1.0f));
    }
    return interpolation_param;
}


void  compute_coord_system_of_frame_of_keyframe_animation(
        keyframes const&  keyframes,
        std::pair<std::size_t, std::size_t> const  indices_of_keyframe_to_interpolate,
        std::size_t const  index_of_coord_system_in_keyframes,
        float_32_bit const  interpolation_param, // in range <0,1>
        angeo::coordinate_system&  output
        )
{
    ASSUMPTION(indices_of_keyframe_to_interpolate.first < keyframes.num_keyframes());
    ASSUMPTION(indices_of_keyframe_to_interpolate.second < keyframes.num_keyframes());
    ASSUMPTION(index_of_coord_system_in_keyframes < keyframes.num_coord_systems_per_keyframe());
    ASSUMPTION(0.0f <= interpolation_param && interpolation_param <= 1.0f);
    angeo::interpolate_spherical(
        keyframes.coord_system_at(indices_of_keyframe_to_interpolate.first, index_of_coord_system_in_keyframes),
        keyframes.coord_system_at(indices_of_keyframe_to_interpolate.second, index_of_coord_system_in_keyframes),
        interpolation_param,
        output
        );
}


void  compute_frame_of_keyframe_animation(
        keyframes const&  keyframes,
        std::pair<std::size_t, std::size_t> const  indices_of_keyframe_to_interpolate,
        float_32_bit const  interpolation_param, // in range <0,1>
        std::vector<angeo::coordinate_system>&  output
        )
{
    output.resize(keyframes.num_coord_systems_per_keyframe());
    for (std::size_t  i = 0UL, n = keyframes.num_coord_systems_per_keyframe(); i != n; ++i)
        compute_coord_system_of_frame_of_keyframe_animation(
                keyframes,
                indices_of_keyframe_to_interpolate,
                i,
                interpolation_param,
                output.at(i)
                );
}


void  compute_frame_of_keyframe_animation(
        keyframes const&  keyframes,
        std::pair<std::size_t, std::size_t> const  indices_of_keyframe_to_interpolate,
        float_32_bit const  interpolation_param, // in range <0,1>
        std::vector<matrix44>&  output
        )
{
    output.resize(keyframes.num_coord_systems_per_keyframe());
    for (std::size_t  i = 0UL, n = keyframes.num_coord_systems_per_keyframe(); i != n; ++i)
    {
        matrix44 M;
        {
            angeo::coordinate_system  S;
            compute_coord_system_of_frame_of_keyframe_animation(
                    keyframes,
                    indices_of_keyframe_to_interpolate,
                    i,
                    interpolation_param,
                    S
                    );
            angeo::from_base_matrix(S, M);
        }
        output.at(i) *= M;
    }
}


template<typename T>
static void  _compute_frame_of_keyframe_animation(
    keyframes const&  keyframes,
    float_32_bit const  time_point,
    std::vector<T>&  output
    )
{
    std::pair<std::size_t, std::size_t> const  indices_of_keyframe_to_interpolate =
        find_indices_of_keyframes_to_interpolate_for_time(keyframes, time_point);
    float_32_bit const  interpolation_param = qtgl::compute_interpolation_parameter(
        time_point,
        keyframes.time_point_at(indices_of_keyframe_to_interpolate.first),
        keyframes.time_point_at(indices_of_keyframe_to_interpolate.second)
        );
    compute_frame_of_keyframe_animation(
        keyframes,
        indices_of_keyframe_to_interpolate,
        interpolation_param,
        output
        );
}



void  compute_frame_of_keyframe_animation(
        keyframes const&  keyframes,
        float_32_bit const  time_point,
        std::vector<angeo::coordinate_system>&  output
        )
{
    _compute_frame_of_keyframe_animation<angeo::coordinate_system>(keyframes, time_point, output);
}


void  compute_frame_of_keyframe_animation(
        keyframes const&  keyframes,
        float_32_bit const  time_point,
        std::vector<matrix44>&  output
        )
{
    _compute_frame_of_keyframe_animation<matrix44>(keyframes, time_point, output);
}


void  compute_frame_of_keyframe_animation(
        keyframes const&  keyframes,
        matrix44 const&  target_space, // typically, the target space is a camera space, i.e. MATRIX_FROM_MODEL_TO_CAMERA
        float_32_bit const  time_point,
        std::vector<matrix44>&  output // old content won't be used and it will be owerwritten by the computed data.
        )
{
    std::vector<matrix44>  transform_matrices(keyframes.num_coord_systems_per_keyframe(), target_space);
    compute_frame_of_keyframe_animation(keyframes, time_point, transform_matrices);
    using std::swap;
    swap(transform_matrices, output);
}


float_32_bit  update_animation_time(
        float_32_bit  current_animation_time_point,
        float_32_bit const  time_delta,
        float_32_bit const  keyframes_start_time,
        float_32_bit const  keyframes_end_time
        )
{
    ASSUMPTION(keyframes_start_time <= keyframes_end_time);

    if (keyframes_end_time - keyframes_start_time < 0.0001f)
        return keyframes_start_time;

    current_animation_time_point += time_delta;

    if (current_animation_time_point < keyframes_start_time)
    {
        float_32_bit const  anim_duration = keyframes_start_time - keyframes_end_time;
        float_32_bit const  distance = current_animation_time_point - keyframes_end_time;
        float_32_bit const  param = distance / anim_duration;
        current_animation_time_point = (param - std::floor(param)) * distance;
    }
    else if (current_animation_time_point > keyframes_end_time)
    {
        float_32_bit const  anim_duration = keyframes_end_time - keyframes_start_time;
        float_32_bit const  distance = current_animation_time_point - keyframes_start_time;
        float_32_bit const  param = distance / anim_duration;
        current_animation_time_point = (param - std::floor(param)) * distance;
    }
    return std::max(keyframes_start_time, std::min(current_animation_time_point, keyframes_end_time));
}


}
