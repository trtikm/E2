#ifndef E2_TOOL_NETVIEWER_DBG_FRUSTUM_SECTOR_ENUMERATION_HPP_INCLUDED
#   define E2_TOOL_NETVIEWER_DBG_FRUSTUM_SECTOR_ENUMERATION_HPP_INCLUDED

#   include <netlab/network_layer_props.hpp>
#   include <angeo/tensor_math.hpp>
#   include <qtgl/draw.hpp>
#   include <vector>
#   include <tuple>


struct  dbg_frustum_sector_enumeration
{
    dbg_frustum_sector_enumeration();

    bool  is_enabled() const noexcept { return m_enabled; }
    void  enable() { m_enabled = true; m_invalidated = true; }
    void  disable();

    bool  is_invalidated() const noexcept { return m_invalidated; }
    void  invalidate() { if (is_enabled()) m_invalidated = true; }

    void  enumerate(std::vector< std::pair<vector3,vector3> > const&  clip_planes,
                    std::vector<netlab::network_layer_props> const&  layer_props);

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
