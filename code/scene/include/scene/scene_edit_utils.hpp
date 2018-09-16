#ifndef E2_SCENE_SCENE_EDIT_UTILS_HPP_INCLUDED
#   define E2_SCENE_SCENE_EDIT_UTILS_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <unordered_set>
#   include <string>
#   include <vector>

namespace scn {


enum struct SCENE_EDIT_MODE : natural_8_bit
{
    SELECT_SCENE_OBJECT = 0,
    TRANSLATE_SELECTED_NODES = 1,
    ROTATE_SELECTED_NODES = 2,
};


struct  scene_nodes_selection_data
{
    scene_nodes_selection_data()
        : m_suppressed_nodes()
    {}

    std::string  choose_best_selection(std::vector<std::string> const&  nodes_on_line);

private:
    std::vector<std::string>  m_suppressed_nodes;
};


struct  scene_nodes_translation_data
{
    scene_nodes_translation_data() : scene_nodes_translation_data(vector3_zero()) {}
    scene_nodes_translation_data(vector3 const&  origin)
        : m_origin(origin)
        , m_normal(vector3_unit_z())
        , m_reduction(vector3_unit_z())
        , m_x_down(false)
        , m_y_down(false)
        , m_z_down(false)
        , m_plain_point(origin)
        , m_is_plain_point_valid(false)
    {}
    void  update(bool const  x_down, bool const  y_down, bool const  z_down, vector3 const&  camera_origin);
    vector3 const&  get_origin() const { return m_origin; }
    vector3 const&  get_normal() const { return m_normal; }
    vector3 const&  get_reduction() const { return m_reduction; }
    vector3 const&  get_plain_point() const { return m_plain_point; }
    bool  is_plain_point_valid() const { return m_is_plain_point_valid; }
    void  invalidate_plain_point() { m_is_plain_point_valid = false; }
    void  set_plain_point(vector3 const&  plain_point) { m_plain_point = plain_point; m_is_plain_point_valid = true; }
    vector3  get_shift(vector3 const&  new_plane_point);

private:

    void  choose_normal_and_reduction(vector3 const&  camera_origin);
    vector3  reduce_shift_vector(vector3 const&  shift);

    vector3  m_origin;
    vector3  m_normal;
    vector3  m_reduction;
    bool  m_x_down;
    bool  m_y_down;
    bool  m_z_down;
    vector3  m_plain_point;
    bool  m_is_plain_point_valid;
};


struct  scene_nodes_rotation_data
{
    scene_nodes_rotation_data() : scene_nodes_rotation_data(vector3_zero()) {}
    scene_nodes_rotation_data(vector3 const&  origin)
        : m_origin(origin)
    {}

    vector3 const&  get_origin() const { return m_origin; }

private:

    vector3  m_origin;
};


struct scene_edit_data final
{
    scene_edit_data(SCENE_EDIT_MODE const initial_mode = SCENE_EDIT_MODE::SELECT_SCENE_OBJECT)
        : m_mode(initial_mode)
        , m_data_invalidated(true)
        , m_operation_started(false)
        , m_nodes_translation_data()
    {}

    SCENE_EDIT_MODE  get_mode() const { return m_mode; };
    void  set_mode(SCENE_EDIT_MODE const  mode);

    bool  are_data_invalidated() const { return m_data_invalidated; }
    void  invalidate_data() { m_data_invalidated = true; m_operation_started = false; }

    bool  was_operation_started() const { return m_operation_started; }

    void  initialise_selection_data(scene_nodes_selection_data const&  data);
    scene_nodes_selection_data const&  get_selection_data() const;
    scene_nodes_selection_data&  get_selection_data();

    void  initialise_translation_data(scene_nodes_translation_data const&  data);
    scene_nodes_translation_data const&  get_translation_data() const;
    scene_nodes_translation_data&  get_translation_data();

    void  initialise_rotation_data(scene_nodes_rotation_data const&  data);
    scene_nodes_rotation_data const&  get_rotation_data() const;
    scene_nodes_rotation_data&  get_rotation_data();    

private:

    SCENE_EDIT_MODE  m_mode;
    bool  m_data_invalidated;
    bool  m_operation_started;
    scene_nodes_selection_data  m_nodes_selection_data;
    scene_nodes_translation_data  m_nodes_translation_data;
    scene_nodes_rotation_data  m_nodes_rotation_data;
};


inline std::string get_pivot_node_name() { return "@pivot"; }


}

#endif
