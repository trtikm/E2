#ifndef E2_TOOL_GFXTUNER_EDIT_MODE_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_EDIT_MODE_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>


enum struct SCENE_EDIT_MODE : natural_8_bit
{
    SELECT_SCENE_OBJECT = 0,
    TRANSLATE_SELECTED_NODES = 1,
    ROTATE_SELECTED_NODES = 2,
};


#endif
