#ifndef E2_TOOL_NETVIEWER_DBG_RAYCAST_SECTOR_ENUMERATION_HPP_INCLUDED
#   define E2_TOOL_NETVIEWER_DBG_RAYCAST_SECTOR_ENUMERATION_HPP_INCLUDED

#   include <netlab/network_layer_props.hpp>
#   include <angeo/tensor_math.hpp>
#   include <qtgl/draw.hpp>
#   include <vector>
#   include <tuple>


struct  dbg_raycast_sector_enumeration
{
    dbg_raycast_sector_enumeration();

    bool  is_enabled() const noexcept { return m_enabled; }
    void  enable() { m_enabled = true; }
    void  disable();

    void  enumerate(vector3 const&  line_begin, vector3 const&  line_end,
                    std::vector<netlab::network_layer_props> const&  layer_props);

    void  render(
            matrix44 const&  matrix_from_world_to_camera,
            matrix44 const&  matrix_from_camera_to_clipspace,
            qtgl::draw_state&  draw_state
            ) const;

private:
    bool  m_enabled;
    qtgl::batch  m_batch_line;
    std::vector< std::pair<vector3,qtgl::batch> >  m_batches;
};


#endif
