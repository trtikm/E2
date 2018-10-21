#include <scene/scene_editing.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <algorithm>

namespace scn {


scn::scene_record_id const*  scene_nodes_selection_data::choose_best_selection(
        std::vector<scn::scene_record_id> const&  records_on_line
        )
{
    std::vector<scn::scene_record_id>  suppressed_records;
    for (auto const&  id : m_suppressed_records)
        if (std::find(records_on_line.cbegin(), records_on_line.cend(), id) != records_on_line.cend())
            suppressed_records.push_back(id);

    for (auto const& id : records_on_line)
        if (std::find(suppressed_records.cbegin(), suppressed_records.cend(), id) == suppressed_records.cend())
        {
            m_suppressed_records.swap(suppressed_records);
            m_suppressed_records.push_back(id);

            return &m_suppressed_records.back();
        }

    m_suppressed_records.clear();
    if (suppressed_records.empty())
        return nullptr;

    for (auto  it = std::next(suppressed_records.cbegin()); it != suppressed_records.cend(); ++it)
        m_suppressed_records.push_back(*it);
    m_suppressed_records.push_back(suppressed_records.front());

    return &m_suppressed_records.back();
}


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
    {
        invalidate_data();
        m_mode = mode;
    }
}


void  scene_edit_data::initialise_selection_data(scene_nodes_selection_data const&  data)
{
    ASSUMPTION(get_mode() == SCENE_EDIT_MODE::SELECT_SCENE_OBJECT);
    m_nodes_selection_data = data;
    m_data_invalidated = false;
    m_operation_started = true;
}

scene_nodes_selection_data const&  scene_edit_data::get_selection_data() const
{
    ASSUMPTION(get_mode() == SCENE_EDIT_MODE::SELECT_SCENE_OBJECT);
    ASSUMPTION(!are_data_invalidated());
    return m_nodes_selection_data;
}

scene_nodes_selection_data&  scene_edit_data::get_selection_data()
{
    ASSUMPTION(get_mode() == SCENE_EDIT_MODE::SELECT_SCENE_OBJECT);
    ASSUMPTION(!are_data_invalidated());
    return m_nodes_selection_data;
}


void  scene_edit_data::initialise_translation_data(scene_nodes_translation_data const&  data)
{
    ASSUMPTION(get_mode() == SCENE_EDIT_MODE::TRANSLATE_SELECTED_NODES);
    m_nodes_translation_data = data;
    m_data_invalidated = false;
    m_operation_started = true;
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


void  scene_edit_data::initialise_rotation_data(scene_nodes_rotation_data const&  data)
{
    ASSUMPTION(get_mode() == SCENE_EDIT_MODE::ROTATE_SELECTED_NODES);
    m_nodes_rotation_data = data;
    m_data_invalidated = false;
    m_operation_started = true;
}

scene_nodes_rotation_data const&  scene_edit_data::get_rotation_data() const
{
    ASSUMPTION(get_mode() == SCENE_EDIT_MODE::ROTATE_SELECTED_NODES);
    ASSUMPTION(!are_data_invalidated());
    return m_nodes_rotation_data;
}

scene_nodes_rotation_data&  scene_edit_data::get_rotation_data()
{
    ASSUMPTION(get_mode() == SCENE_EDIT_MODE::ROTATE_SELECTED_NODES);
    ASSUMPTION(!are_data_invalidated());
    return m_nodes_rotation_data;
}


}
