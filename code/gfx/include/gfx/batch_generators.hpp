#ifndef GFX_BATCH_GENERATORS_HPP_INCLUDED
#   define GFX_BATCH_GENERATORS_HPP_INCLUDED

#   include <gfx/batch.hpp>
#   include <gfx/texture.hpp>
#   include <gfx/image.hpp>
#   include <angeo/tensor_math.hpp>
#   include <array>
#   include <vector>
#   include <string>
#   include <boost/filesystem/path.hpp>
#   include <boost/property_tree/ptree.hpp>

namespace gfx {


batch  create_lines3d(
        std::vector< std::array<float_32_bit, 3> > const&  vertices,
        std::vector< std::array<float_32_bit, 4> > const&  colours,
        FOG_TYPE const  fog_type_ = FOG_TYPE::NONE,
        std::string const&  id = ""
        );

batch  create_lines3d(
        std::vector< std::array<float_32_bit, 3> > const&  vertices,
        std::array<float_32_bit, 4> const&  common_colour,
        FOG_TYPE const  fog_type_ = FOG_TYPE::NONE,
        std::string const&  id = ""
        );

batch  create_lines3d(
        std::vector< std::array<float_32_bit, 3> > const&  vertices,
        vector4 const&  common_colour,
        FOG_TYPE const  fog_type_ = FOG_TYPE::NONE,
        std::string const&  id = ""
        );

batch  create_lines3d(
        std::vector< std::pair<vector3,vector3> > const&  lines,
        std::vector< vector4 > const&  colours_of_lines,
        FOG_TYPE const  fog_type_ = FOG_TYPE::NONE,
        std::string const&  id = ""
        );

batch  create_lines3d(
        std::vector< std::pair<vector3,vector3> > const&  lines,
        vector4 const&  common_colour,
        FOG_TYPE const  fog_type_ = FOG_TYPE::NONE,
        std::string const&  id = ""
        );


enum struct  GRID_MAIN_AXES_ORIENTATION_MARKER_TYPE : natural_8_bit
{
    NONE                        = 0,
    TRIANGLE                    = 1,
    RIGHT_ANGLES_TO_Z_AXIS      = 2,
    RIGHT_ANGLES_TO_ALL_AXES    = 3,
};

batch  create_grid(
        float_32_bit const  max_x_coordinate,
        float_32_bit const  max_y_coordinate,
        float_32_bit const  max_z_coordinate,
        float_32_bit const  step_along_x_axis,
        float_32_bit const  step_along_y_axis,
        std::array<float_32_bit, 4> const&  colour_for_x_lines,
        std::array<float_32_bit, 4> const&  colour_for_y_lines,
        std::array<float_32_bit, 4> const&  colour_for_highlighted_x_lines,
        std::array<float_32_bit, 4> const&  colour_for_highlighted_y_lines,
        std::array<float_32_bit, 4> const&  colour_for_central_x_line,
        std::array<float_32_bit, 4> const&  colour_for_central_y_line,
        std::array<float_32_bit, 4> const&  colour_for_central_z_line,
        natural_32_bit const  highlight_every,
        GRID_MAIN_AXES_ORIENTATION_MARKER_TYPE const  main_exes_orientation_marker_type,
        FOG_TYPE const  fog_type_ = FOG_TYPE::NONE,
        std::string const&  id = ""
        );

batch  create_default_grid();
batch  create_default_grid_2d(float_32_bit const  size = 100.0f);


std::string  get_sketch_id_prefix();

inline std::string  sketch_kind_basis_vectors() { return "basis_vectors"; }
inline std::string  sketch_kind_coord_cross() { return "coord_cross"; }
inline std::string  sketch_kind_arrow() { return "arrow"; }
inline std::string  sketch_kind_perspective_frustum() { return "perspective_frustum"; }
inline std::string  sketch_kind_box() { return "box"; }
inline std::string  sketch_kind_capsule() { return "capsule"; }
inline std::string  sketch_kind_sphere() { return "sphere"; }
inline std::string  sketch_kind_mesh() { return "mesh"; }
inline std::string  sketch_kind_convex_hull() { return "convex_hull"; }


batch  create_basis_vectors();


batch  create_coord_cross(float_32_bit const  size, vector4 const&  colour);
std::string  make_coord_cross_id_without_prefix(float_32_bit const  size, vector4 const&  colour);


batch  create_arrow(float_32_bit const  size, vector4 const&  colour);
std::string  make_arrow_id_without_prefix(float_32_bit const  size, vector4 const&  colour);


batch  create_wireframe_perspective_frustum(
        float_32_bit const  near_plane,
        float_32_bit const  far_plane,
        float_32_bit const  left_plane,
        float_32_bit const  right_plane,
        float_32_bit const  top_plane,
        float_32_bit const  bottom_plane,
        vector4 const&  colour,
        bool const  with_axis = false
        );
std::string  make_perspective_frustum_id_without_prefix(
        float_32_bit const  near_plane,
        float_32_bit const  far_plane,
        float_32_bit const  left_plane,
        float_32_bit const  right_plane,
        float_32_bit const  top_plane,
        float_32_bit const  bottom_plane,
        vector4 const&  colour,
        bool const  with_axis = false
        );


bool  read_sketch_info_from_id(std::string const& id, boost::property_tree::ptree&  output_info);


batch  create_wireframe_box( // Deprecated!
        vector3 const&  lo_corner,
        vector3 const&  hi_corner,
        vector4 const&  colour,
        FOG_TYPE const  fog_type_ = FOG_TYPE::NONE,
        std::string const&  id = ""
        );
batch  create_wireframe_box(
        vector3 const&  half_sizes_along_axes,
        vector4 const&  colour,
        FOG_TYPE const  fog_type_ = FOG_TYPE::NONE,
        std::string const&  id = ""
        );
batch  create_solid_box(
        vector3 const&  half_sizes_along_axes,
        vector4 const&  colour,
        FOG_TYPE const  fog_type_ = FOG_TYPE::NONE,
        std::string const&  id = ""
        );
std::string  make_box_id_without_prefix(
        vector3 const&  half_sizes_along_axes,
        vector4 const&  colour,
        bool const  wireframe,
        FOG_TYPE const  fog_type = FOG_TYPE::NONE
        );
bool  parse_box_info_from_id(
        boost::property_tree::ptree const&  ptree,
        vector3&  half_sizes_along_axes,
        vector4&  colour,
        FOG_TYPE&  fog_type,
        bool&  wireframe
        );


batch  create_wireframe_capsule(
        float_32_bit const  half_distance_between_end_points,
        float_32_bit const  thickness_from_central_line,
        natural_8_bit const  num_lines_per_quarter_of_circle,
        vector4 const&  colour,
        FOG_TYPE const  fog_type_ = FOG_TYPE::NONE,
        std::string const&  id = ""
        );
batch  create_solid_capsule(
        float_32_bit const  half_distance_between_end_points,
        float_32_bit const  thickness_from_central_line,
        natural_8_bit const  num_lines_per_quarter_of_circle,
        vector4 const&  colour,
        FOG_TYPE const  fog_type_ = FOG_TYPE::NONE,
        std::string const&  id = ""
        );
std::string  make_capsule_id_without_prefix(
        float_32_bit const  half_distance_between_end_points,
        float_32_bit const  thickness_from_central_line,
        natural_8_bit const  num_lines_per_quarter_of_circle,
        vector4 const&  colour,
        bool const  wireframe,
        FOG_TYPE const  fog_type = FOG_TYPE::NONE
        );
bool  parse_capsule_info_from_id(
        boost::property_tree::ptree const&  ptree,
        float_32_bit&  half_distance_between_end_points,
        float_32_bit&  thickness_from_central_line,
        natural_8_bit&  num_lines_per_quarter_of_circle,
        vector4&  colour,
        FOG_TYPE&  fog_type,
        bool&  wireframe
        );


batch  create_wireframe_sphere(
        float_32_bit const  radius,
        natural_8_bit const  num_lines_per_quarter_of_circle,
        vector4 const&  colour,
        FOG_TYPE const  fog_type_ = FOG_TYPE::NONE,
        std::string const&  id = ""
        );
batch  create_solid_smooth_sphere(
        float_32_bit const  radius,
        natural_8_bit const  num_lines_per_quarter_of_circle,
        vector4 const&  colour,
        FOG_TYPE const  fog_type_ = FOG_TYPE::NONE,
        std::string const&  id = ""
        );
batch  create_solid_sphere(
        float_32_bit const  radius,
        natural_8_bit const  num_lines_per_quarter_of_circle,
        vector4 const&  colour,
        FOG_TYPE const  fog_type_ = FOG_TYPE::NONE,
        std::string const&  id = ""
        );
std::string  make_sphere_id_without_prefix(
        float_32_bit const  radius,
        natural_8_bit const  num_lines_per_quarter_of_circle,
        vector4 const&  colour,
        bool const  wireframe,
        FOG_TYPE const  fog_type = FOG_TYPE::NONE
        );
bool  parse_sphere_info_from_id(
        boost::property_tree::ptree const&  ptree,
        float_32_bit&  radius,
        natural_8_bit&  num_lines_per_quarter_of_circle,
        vector4&  colour,
        FOG_TYPE&  fog_type,
        bool&  wireframe
        );


inline batch  create_wireframe_diamond(
        float_32_bit const  radius,
        vector4 const&  colour,
        FOG_TYPE const  fog_type_ = FOG_TYPE::NONE,
        std::string const&  id = ""
        ) { return create_wireframe_sphere(radius, 1U, colour, fog_type_, id); }
inline batch  create_solid_diamond(
        float_32_bit const  radius,
        vector4 const&  colour,
        FOG_TYPE const  fog_type_ = FOG_TYPE::NONE,
        std::string const&  id = ""
        ) { return create_solid_sphere(radius, 1U, colour, fog_type_, id);}



batch  create_triangle_mesh(
        gfx::buffer  vertex_buffer,
        gfx::buffer  index_buffer,
        vector4 const&  colour,
        FOG_TYPE const  fog_type_ = FOG_TYPE::NONE,
        std::string const&  id = ""
        );


batch  create_triangle_mesh(
        gfx::buffer  vertex_buffer,
        gfx::buffer  index_buffer,
        gfx::buffer  normal_buffer,
        vector4 const&  colour,
        FOG_TYPE const  fog_type_ = FOG_TYPE::NONE,
        std::string const&  id = ""
        );


batch  create_triangle_mesh(
        gfx::buffer  vertex_buffer,
        gfx::buffer  index_buffer,
        gfx::buffer  texcoord_buffer,
        texture const&  diffuse,
        FOG_TYPE const  fog_type_ = FOG_TYPE::NONE,
        std::string const&  id = ""
        );


batch  create_triangle_mesh(
        std::vector< std::array<float_32_bit, 3> > const&  vertices,
        std::vector< std::array<float_32_bit, 2> > const&  texcoords,
        texture const&  diffuse,
        bool const  use_alpha_blending = false,
        FOG_TYPE const  fog_type_ = FOG_TYPE::NONE,
        std::string const&  id = ""
        );


batch  create_triangle_mesh(
        std::vector< std::array<float_32_bit, 3> > const&  vertices,
        vector4 const&  colour,
        FOG_TYPE const  fog_type_,
        std::string const&  id
        );


batch  create_triangle_mesh(
        std::vector< std::array<float_32_bit, 3> > const& vertices,
        std::vector< std::array<float_32_bit, 3> > const& normals,
        vector4 const&  colour,
        FOG_TYPE const  fog_type_,
        std::string const&  id
        );


batch  create_triangle_mesh(
        std::vector< std::array<float_32_bit, 3> > const&  vertices,
        std::vector< std::array<natural_32_bit, 3> > const&  indices,
        std::vector< std::array<float_32_bit, 3> > const&  normals,
        vector4 const&  colour,
        FOG_TYPE const  fog_type_,
        std::string const&  id
        );


batch  create_sprite(image_rgba_8888 const&  img);
batch  create_sprite(texture const  t);
texture  get_sprite_texture(batch const  sprite_batch);
void  update_sprite(batch const  sprite_batch, image_rgba_8888 const&  img);


struct  font_mono_props
{
    std::string  font_texture;

    natural_8_bit  min_ascii_code;
    natural_8_bit  max_ascii_code;

    // Matrix of characters in the texture
    natural_8_bit  max_chars_in_row;
    float_32_bit  min_u;    // u-coord where the matrix of chars starts in the texture
    float_32_bit  min_v;    // v-coord where the matrix of chars starts in the texture 
    float_32_bit  char_uv_width;
    float_32_bit  char_uv_height;
    float_32_bit  char_separ_u;
    float_32_bit  char_separ_v;

    // Text formating in camera space; all values are in meters.
    float_32_bit  space_size;
    natural_32_bit  tab_size;   // How many space ' ' characters it represents.
    float_32_bit  char_width;
    float_32_bit  char_height;
    float_32_bit  char_separ_dist_x;
    float_32_bit  char_separ_dist_y;

    void release() { __batch_template__.release(); }

    // Next follow PRIVATE DATA; DEFAULT-INITIALISE AND DO NOT MODIFY!
    mutable batch  __batch_template__;  // Used to prevent re-creating those components of batch which are common for all batchech returned
                                        // from the function 'create_text'. This is especially important for reusing the font texture
                                        // instead in loading it from the disk over and over
                                        // The field is set by 'create_text' function below (when it is empty).
};


void  load_font_mono_props(boost::filesystem::path const&  pathname, font_mono_props&  output);
void  save_font_mono_props(
        boost::filesystem::path const&  pathname,
        boost::filesystem::path const&  data_root_dir,
        font_mono_props const&  props
        );
void  build_font_batch_template_if_not_built_yet(font_mono_props const&  props, std::string const&  id = "");

texture  get_font_texture(font_mono_props const&  props, bool const  wait_for_resource = true);


struct  text_info
{
    text_info() : num_rows(0U), cursor_row(0U), cursor_column(0U), cursor_pos(vector2_zero()) {}
    natural_32_bit  num_rows;
    natural_32_bit  cursor_row;
    natural_32_bit  cursor_column;
    vector2  cursor_pos;
};


batch  create_text(
        std::string const&  text,
        font_mono_props const&  props,
        float_32_bit const  max_text_width = 0.0f, // When positive, the text will be wrapped not to exceed that width.
        text_info* const  out_info_ptr = nullptr,
        natural_8_bit const  cursor_char = 0xffU,
        bool const  wrap_whole_words = true, // Used only if max_text_width > 0.0f
        bool const  do_char_escaping = true,
        std::string const&  id = ""
        );


}

#endif
