#ifndef E2_TOOL_NETVIEWER_DBG_DRAW_MOVEMENT_AREAS_HPP_INCLUDED
#   define E2_TOOL_NETVIEWER_DBG_DRAW_MOVEMENT_AREAS_HPP_INCLUDED

#   include <netlab/network.hpp>
#   include <angeo/tensor_math.hpp>
#   include <qtgl/draw.hpp>
#   include <vector>
#   include <tuple>


struct  dbg_draw_movement_areas
{
    dbg_draw_movement_areas();

    bool  is_enabled() const noexcept { return m_enabled; }
    void  enable() { m_enabled = true; m_invalidated = true; }
    void  disable();

    bool  is_invalidated() const noexcept { return m_invalidated; }
    void  invalidate() { if (is_enabled()) m_invalidated = true; }

    void  collect_visible_areas(std::vector< std::pair<vector3, vector3> > const&  clip_planes,
                                std::shared_ptr<netlab::network const> const  network);

    void  render(
            matrix44 const&  matrix_from_world_to_camera,
            matrix44 const&  matrix_from_camera_to_clipspace,
            qtgl::draw_state&  draw_state
            ) const;

private:
    bool  m_enabled;
    bool  m_invalidated;
    std::vector< std::pair<vector3,qtgl::batch> >  m_batches;
};


#endif
