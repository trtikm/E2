#include <gfxtuner/scene_edit_utils.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>


void  scene_nodes_translation_data::update_keys(bool const  x_down, bool const  y_down, bool const  z_down)
{
    bool const  change = m_x_down != x_down || m_y_down != y_down || m_z_down != z_down;
    if (!change)
        return;
    m_x_down = x_down;
    m_y_down = y_down;
    m_z_down = z_down;
    invalidate_plain_point();
    choose_normal_and_reduction();
}

void  scene_nodes_translation_data::choose_normal_and_reduction()
{
}

vector3  scene_nodes_translation_data::reduce_shift_vector(vector3 const&  shift)
{
    return shift;
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
