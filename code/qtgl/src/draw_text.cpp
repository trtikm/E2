#include <qtgl/draw_text.hpp>
#include <qtgl/text_manager.hpp>
#include <qtgl/window.hpp>

namespace qtgl { namespace detail {


extern  window*  current_window();
text_manager&  get_text_manager() { return current_window()->get_text_manager(); }



}}

namespace qtgl {


void  set_font_directory(std::string const&  font_directory)
{
    detail::get_text_manager().set_font_directory(font_directory);
}


void  set_text_viewport(
        float_32_bit const  left,
        float_32_bit const  right,
        float_32_bit const  bottom,
        float_32_bit const  top
        )
{
    detail::get_text_manager().set_viewport(left, right, bottom, top);
}


void  set_text_colour(vector4 const&  colour)
{
    detail::get_text_manager().set_colour(colour);
}


void  set_text_size(float_32_bit const  size_in_meters)
{
    detail::get_text_manager().set_size(size_in_meters);
}


void  set_text_cursor(vector2 const&  position)
{
    detail::get_text_manager().set_cursor(position);
}


void  enable_depth_testing_for_text(bool const  state)
{
    detail::get_text_manager().enable_depth_testing(state);
}


void  print_character(char const character, float_32_bit const  z_coord)
{
    detail::get_text_manager().insert(character, z_coord);
}


void  print_text(std::string const&  text, float_32_bit const  z_coord)
{
    for (char  character : text)
        print_character(character, z_coord);
}


}
