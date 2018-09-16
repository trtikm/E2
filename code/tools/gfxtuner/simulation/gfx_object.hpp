#ifndef E2_TOOL_GFXTUNER_GFX_OBJECT_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_GFX_OBJECT_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <qtgl/batch.hpp>
#   include <qtgl/keyframe.hpp>
#   include <vector>
#   include <string>


struct gfx_animated_object
{
    explicit gfx_animated_object(qtgl::batch const  batch_);

    qtgl::batch  get_batch() const { return m_batch; }
    std::string const&  get_animation_name() const { return m_animation_names.at(m_animation_index); }
    float_32_bit  get_animation_time() const { return m_animation_time; }
    qtgl::keyframes  get_keyframes() const { return m_keyframes; }

    void  next_round(float_64_bit const  time_to_simulate_in_seconds);
    void  get_transformations(std::vector<matrix44>&  output, matrix44 const&  target_space) const;

private:

    qtgl::batch  m_batch;
    std::vector<std::string>  m_animation_names;
    natural_32_bit  m_animation_index;
    float_32_bit  m_animation_time;
    qtgl::keyframes  m_keyframes;
};


#endif
