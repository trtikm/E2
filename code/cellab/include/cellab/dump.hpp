#ifndef CELLAB_DUMP_HPP_INCLUDED
#   define CELLAB_DUMP_HPP_INCLUDED

#   include <cellab/static_state_of_neural_tissue.hpp>
#   include <memory>
#   include <iosfwd>

namespace cellab {


std::ostream&  dump_in_html(
        std::ostream&  ostr,
        std::shared_ptr<static_state_of_neural_tissue const> const  static_tissue_ptr
        );


}

#endif
