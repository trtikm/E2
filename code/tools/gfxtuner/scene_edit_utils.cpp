#include <gfxtuner/scene_edit_utils.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>


void  scene_nodes_translation_data::update(bool const  x_down, bool const  y_down, bool const  z_down, vector3 const&  camera_origin)
{
    bool const  change = m_x_down != x_down || m_y_down != y_down || m_z_down != z_down;
    if (!change)
        return;
    m_x_down = x_down;
    m_y_down = y_down;
    m_z_down = z_down;
    invalidate_plain_point();
    choose_normal_and_reduction(camera_origin);
}

void  scene_nodes_translation_data::choose_normal_and_reduction(vector3 const&  camera_origin)
{
    if (m_x_down && !m_y_down && !m_z_down)
    {
        m_normal = vector3_unit_z();
        m_reduction = vector3_unit_x();
    }
    else if (!m_x_down && m_y_down && !m_z_down)
    {
        m_normal = vector3_unit_z();
        m_reduction = vector3_unit_y();
    }
    else if (!m_x_down && !m_y_down && m_z_down)
    {
        m_normal = std::fabsf(dot_product(vector3_unit_x(), camera_origin)) > std::fabsf(dot_product(vector3_unit_y(), camera_origin)) ?
                        vector3_unit_x() :
                        vector3_unit_y();
        m_reduction = vector3_unit_z();
    }
    else if (m_x_down && !m_y_down && m_z_down)
    {
        m_normal = vector3_unit_y();
        m_reduction = vector3_unit_y();
    }
    else if (!m_x_down && m_y_down && m_z_down)
    {
        m_normal = vector3_unit_x();
        m_reduction = vector3_unit_x();
    }
    else
    {
        m_normal = vector3_unit_z();
        m_reduction = vector3_unit_z();
    }
}

vector3  scene_nodes_translation_data::reduce_shift_vector(vector3 const&  shift)
{
    vector3 const  component = dot_product(shift, m_reduction) * m_reduction;
    return (m_x_down ? 1U : 0U) + (m_y_down ? 1U : 0U) + (m_z_down ? 1U : 0U) == 1 ? component : shift - component;
}

vector3  scene_nodes_translation_data::get_shift(vector3 const&  new_plane_point)
{
    if (!is_plain_point_valid())
    {
        set_plain_point(new_plane_point);
        return vector3_zero();
    }
    vector3  shift = new_plane_point - m_plain_point;
    set_plain_point(new_plane_point);
    return reduce_shift_vector(shift);
}


void  scene_edit_data::set_mode(SCENE_EDIT_MODE const  mode)
{
    if (mode != m_mode)
        invalidate_data();
    m_mode = mode;
}

void  scene_edit_data::initialise_translation_data(scene_nodes_translation_data const&  data)
{
    ASSUMPTION(get_mode() == SCENE_EDIT_MODE::TRANSLATE_SELECTED_NODES);
    m_nodes_translation_data = data;
    m_data_invalidated = false;
}

scene_nodes_translation_data const&  scene_edit_data::get_translation_data() const
{
    ASSUMPTION(get_mode() == SCENE_EDIT_MODE::TRANSLATE_SELECTED_NODES);
    ASSUMPTION(!are_data_invalidated());
    return m_nodes_translation_data;
}

scene_nodes_translation_data&  scene_edit_data::get_translation_data()
{
    ASSUMPTION(get_mode() == SCENE_EDIT_MODE::TRANSLATE_SELECTED_NODES);
    ASSUMPTION(!are_data_invalidated());
    return m_nodes_translation_data;
}
