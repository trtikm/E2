#include <gfx/gui/window.hpp>
#include <utility/assumptions.hpp>
#include <utility/development.hpp>

namespace gfx { namespace gui {


window::window(
        std::shared_ptr<viewport> const  vp,
        bool const  resize_with_viewport_,
        vector2 const&  lo,
        vector2 const&  hi
        )
    : m_viewport(vp)
    , m_lo(lo)
    , m_hi(hi)
    , m_resize_with_viewport(resize_with_viewport_)

{
    ASSUMPTION(m_viewport != nullptr);
    if (m_resize_with_viewport)
        resize_to_viewport();
    ASSUMPTION(m_lo(0) < m_hi(0) && m_lo(1) < m_hi(1));
    ASSUMPTION(m_viewport->is_point_inside(m_lo) && m_viewport->is_point_inside(m_hi));
}


void  window::resize_to_viewport()
{
    m_lo = { m_viewport->left, m_viewport->bottom, };
    m_hi = { m_viewport->right, m_viewport->top };
}


void  window::update(float_32_bit const  round_seconds)
{
    if (m_resize_with_viewport)
        resize_to_viewport();
}


}}
