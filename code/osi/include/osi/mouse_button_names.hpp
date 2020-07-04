#ifndef OSI_MOUSE_BUTTON_NAMES_HPP_INCLUDED
#   define OSI_MOUSE_BUTTON_NAMES_HPP_INCLUDED

#   include <string>

namespace osi {


using  mouse_button_name = std::string;

inline mouse_button_name  LEFT_MOUSE_BUTTON() { return "LMOUSE"; }
inline mouse_button_name  RIGHT_MOUSE_BUTTON() { return "RMOUSE"; }
inline mouse_button_name  MIDDLE_MOUSE_BUTTON() { return "MMOUSE"; }


}

#endif
