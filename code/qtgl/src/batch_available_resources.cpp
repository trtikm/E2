#include <qtgl/batch_available_resources.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/msgstream.hpp>
#include <utility/canonical_path.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>
#include <iomanip>

namespace qtgl { namespace detail {


batch_available_resources_data::batch_available_resources_data(
        boost::filesystem::path const&  path,
        async::finalise_load_on_destroy_ptr
        )
{
    TMPROF_BLOCK();

    NOT_IMPLEMENTED_YET();
}


batch_available_resources_data::batch_available_resources_data(
        async::finalise_load_on_destroy_ptr,
        buffers_dictionaty_type const&  buffers_,
        textures_dictionary_type const&  textures_
        )
    : m_buffers(buffers_)
    , m_textures(textures_)
{}


}}
