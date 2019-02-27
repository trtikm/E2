#ifndef QTGL_TEXT_MANAGER_HPP_INCLUDED
#   define QTGL_TEXT_MANAGER_HPP_INCLUDED

#   include <qtgl/batch.hpp>
#   include <qtgl/effects_config.hpp>
#   include <qtgl/shader_data_linkers.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <angeo/tensor_math.hpp>
#   include <unordered_map>
#   include <vector>
#   include <string>
#   include <mutex>

namespace qtgl {


struct text_manager
{
    text_manager();

    void  set_font_directory(std::string const&  font_directory);

    void  set_viewport(
            float_32_bit const  left,
            float_32_bit const  right,
            float_32_bit const  bottom,
            float_32_bit const  top
            );
    void  set_colour(vector4 const&  colour) { m_colour = colour; }
    void  set_size(float_32_bit const  size) { m_size = size; }
    void  set_cursor(vector2 const&  position) { m_cursor = position; }
    void  enable_depth_testing(bool const  state) { m_depth_test_enabled = state; }

    // Returns false in case of failure - when the passed character does not have any representation in the font, or it was not loaded yet.
    bool  insert(char const  character, float_32_bit const  z_coord);

    void  flush();

private:
    struct  character_draw_info
    {
        matrix44  size_and_position_matrix;
        matrix44  from_camera_to_clipspace_matrix;
        vector4  diffuse_colour;
    };

    matrix44 build_from_camera_to_clipspace_matrix() const;
    vector2  move_cursor(char  character);

    std::unordered_map<char, batch>  m_font;
    std::vector<std::pair<matrix44, std::unordered_map<char, vertex_shader_instanced_data_provider> > >  m_text;
    float_32_bit  m_left;
    float_32_bit  m_right;
    float_32_bit  m_bottom;
    float_32_bit  m_top;
    vector2  m_cursor;
    vector4  m_colour;
    float_32_bit  m_size;
    bool  m_depth_test_enabled;
    std::mutex  m_mutex;
};


}

#endif
