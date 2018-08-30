#ifndef QTGL_SPATIAL_BOUNDARY_HPP_INCLUDED
#   define QTGL_SPATIAL_BOUNDARY_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>

namespace qtgl {


struct  spatial_boundary
{
    spatial_boundary(
            float_32_bit const  radius,
            vector3 const&  lo_corner,
            vector3 const&  hi_corner
            )
        : m_radius(radius)
        , m_lo_corner(lo_corner)
        , m_hi_corner(hi_corner)
    {}

    float_32_bit const  radius() const { return m_radius; }
    vector3 const&  lo_corner() const { return m_lo_corner; }
    vector3 const&  hi_corner() const { return m_hi_corner; }

private:
    float_32_bit  m_radius;
    vector3  m_lo_corner;
    vector3  m_hi_corner;
};


}

#endif
