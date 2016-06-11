#include <qtgl/batch.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/msgstream.hpp>
#include <boost/filesystem.hpp>
#include <boost/functional/hash.hpp>
#include <limits>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <iterator>
#include <unordered_set>

namespace qtgl {


std::unordered_set<vertex_shader_uniform_symbolic_name>  batch::s_empty_uniforms;

batch::batch(boost::filesystem::path const&  path)
    : m_path(path)
    , m_buffers_binding()
    , m_shaders_binding()
    , m_textures_binding()
{
    ASSUMPTION(!path.empty());
}

batch::batch(boost::filesystem::path const&  path,
             buffers_binding_ptr const  buffers_binding,
             shaders_binding_ptr const  shaders_binding,
             textures_binding_ptr const  textures_binding
             )
    : m_path(path)
    , m_buffers_binding(buffers_binding)
    , m_shaders_binding(shaders_binding)
    , m_textures_binding(textures_binding)
{
    ASSUMPTION(m_buffers_binding.operator bool());
    ASSUMPTION(m_shaders_binding.operator bool());
    ASSUMPTION(m_textures_binding.operator bool());
}

std::unordered_set<vertex_shader_uniform_symbolic_name> const&  batch::symbolic_names_of_used_uniforms() const
{
    return shaders_binding().operator bool() && shaders_binding()->vertex_program_props().operator bool() ?
                shaders_binding()->vertex_program_props()->symbolic_names_of_used_uniforms() :
                s_empty_uniforms ;
}

std::shared_ptr<batch const>  load_batch_file(boost::filesystem::path const&  batch_file, std::string&  error_message)
{
    NOT_IMPLEMENTED_YET();
}

bool  make_current(batch const&  binding, batch::uniform_initialiser const& initialiser)
{
    NOT_IMPLEMENTED_YET();
}

bool  make_current(batch const&  binding,
                   matrix44 const& transform_matrix_transposed,
                   batch::uniform_initialiser const& initialiser)
{
    NOT_IMPLEMENTED_YET();
}


}
