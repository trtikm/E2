#ifndef PARALAB_TC0_HPP_INCLUDED
#   define PARALAB_TC0_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <utility/bits_reference.hpp>

namespace paralab {


struct tc0
{
    tc0();
    tc0(bits_reference const&  data);

    void  update();

    void operator>>(bits_reference&  output) const;

private:

};


}

#endif
