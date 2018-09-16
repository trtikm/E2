#include <gfxtuner/simulation/gfx_object.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/canonical_path.hpp>
#include <boost/filesystem.hpp>


gfx_animated_object::gfx_animated_object(qtgl::batch const  batch_)
    : m_batch(batch_)
    , m_animation_names()
    , m_animation_index(0U)
    , m_animation_time(0.0f)
    , m_keyframes()
{
    TMPROF_BLOCK();

    ASSUMPTION(get_batch().ready());

    ASSUMPTION(get_batch().get_available_resources().skeletal().size() == 1UL);
    boost::filesystem::path  anim_root_dir =
        canonical_path(get_batch().get_available_resources().skeletal().cbegin()->first);
    for (boost::filesystem::directory_entry& entry : boost::filesystem::directory_iterator(anim_root_dir))
        if (boost::filesystem::is_directory(entry.path()))
            m_animation_names.push_back(entry.path().string());
    ASSUMPTION(!m_animation_names.empty());

    m_keyframes.insert_load_request(m_animation_names.at(m_animation_index));
    ASSUMPTION(!m_keyframes.empty());
}


void  gfx_animated_object::next_round(float_64_bit const  time_to_simulate_in_seconds)
{
    TMPROF_BLOCK();

    if (!get_keyframes().loaded_successfully())
        return;

    m_animation_time = qtgl::update_animation_time(
        get_animation_time(),
        time_to_simulate_in_seconds,
        get_keyframes().start_time_point(),
        get_keyframes().end_time_point()
        );
}


void  gfx_animated_object::get_transformations(std::vector<matrix44>&  output, matrix44 const&  target_space) const
{
    TMPROF_BLOCK();

    output.clear();
    if (!get_keyframes().loaded_successfully())
        output.push_back(target_space);
    else
        qtgl::compute_frame_of_keyframe_animation(
                get_keyframes(),
                target_space,
                get_animation_time(),
                output
                );
}
