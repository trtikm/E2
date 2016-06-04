#include <qtgl/gui_utils.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>

namespace qtgl {


void  set_splitter_sizes(QSplitter&  splitter,  float_32_bit const  ratio_of_child_widgets)
{
    QList<int>  sizes = splitter.sizes();
    int const  list_length = sizes.size();
    ASSUMPTION(list_length == 2);
    float_32_bit const  total_size = (float_32_bit)(sizes[0] + sizes[1]);
    sizes[0] = (int)(ratio_of_child_widgets * total_size);
    sizes[1] = (int)((1.0f - ratio_of_child_widgets) * total_size);
    sizes[0] += total_size - (sizes[0] + sizes[1]);
    splitter.setSizes(sizes);
}

float_32_bit  get_splitter_sizes_ratio(QSplitter const&  splitter)
{
    QList<int>  sizes = splitter.sizes();
    int const  list_length = sizes.size();
    ASSUMPTION(list_length == 2);
    float_32_bit const  total_size = (float_32_bit)(sizes[0] + sizes[1]);
    ASSUMPTION(total_size > 0.0f);
    return sizes[0] / total_size;
}


std::string  to_string(QString const& s)
{
    std::string  result;
    for (int  i = 0; i < s.size(); ++i)
        result.push_back(s.at(i).toLatin1());
    return result;
}


}
