#ifndef QTGL_GUI_UTILS_HPP_INCLUDED
#   define QTGL_GUI_UTILS_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <QSplitter>
#   include <QString>
#   include <string>

namespace qtgl {


void  set_splitter_sizes(QSplitter&  splitter,  float_32_bit const  ratio_of_child_widgets);
float_32_bit  get_splitter_sizes_ratio(QSplitter const&  splitter);

std::string  to_string(QString const& s);

}

#endif
