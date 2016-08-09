#ifndef NENET_HPP_INCLUDED
#   define NENET_HPP_INCLUDED

#   include <utility/tensor_math.hpp>
#   include <vector>
#   include <unordered_map>

struct  input_spot;
struct  output_terminal;

struct cell
{
    struct  pos_hasher
    {
        pos_hasher(vector3 const&  origin, vector3 const&  intercell_distance,
                   natural_64_bit const  num_cells_x, natural_64_bit const  num_cells_y, natural_64_bit const  num_cells_c);

        std::size_t  operator()(vector3 const&  pos) const;

        vector3 const&  origin() const noexcept { return  m_origin; }
        vector3 const&  inter_distance() const noexcept { return  m_intercell_distance; }
        natural_64_bit  size_x() const noexcept { return m_num_cells_x; }
        natural_64_bit  size_y() const noexcept { return m_num_cells_y; }
        natural_64_bit  size_c() const noexcept { return m_num_cells_c; }

    private:
        vector3  m_origin;
        vector3  m_intercell_distance;
        natural_64_bit  m_num_cells_x;
        natural_64_bit  m_num_cells_y;
        natural_64_bit  m_num_cells_c;
    };

    struct  pos_equal
    {
        pos_equal(vector3 const&  max_distance);
        bool  operator()(vector3 const&  l, vector3 const&  r) const;
        vector3 const&  max_distance() const noexcept { return  m_max_distance; }
    private:
        vector3  m_max_distance;
    };

    template<typename T>
    using  pos_map_template = std::unordered_map<vector3, T, pos_hasher, pos_equal>;

    using  pos_map = pos_map_template<cell>;

    using  input_spots_map = pos_map_template<input_spot>;
    using  input_spot_iterator = input_spots_map::iterator;

    using  output_terminals_map = pos_map_template<output_terminal>;
    using  output_terminal_iterator = output_terminals_map::iterator;

    cell();
    //cell(vector3 const&  output_area_center, scalar const  output_area_radius);

    void  add_input_spot(input_spot_iterator const  ispot) { m_input_spots.push_back(ispot); }
    std::vector<input_spot_iterator> const&  input_spots() const noexcept { return m_input_spots; }

    void  add_output_terminal(output_terminal_iterator const  oterm) { m_output_terminals.push_back(oterm); }
    std::vector<output_terminal_iterator> const&  output_terminals() const noexcept { return m_output_terminals; }

    vector3 const&  output_area_center() const noexcept { return m_output_area_center; }
    //scalar  output_area_radius() const noexcept { return m_output_area_radius; }
    void  set_output_area_center(vector3 const&  center) { m_output_area_center = center; }
    //void  set_output_area_radius(scalar const  radius) { m_output_area_radius = radius; }

private:
    std::vector<input_spot_iterator>  m_input_spots;
    std::vector<output_terminal_iterator>  m_output_terminals;
    vector3  m_output_area_center;
    //scalar  m_output_area_radius;
};

struct  input_spot
{
    using  pos_hasher = cell::pos_hasher;
    using  pos_equal = cell::pos_equal;
    using  pos_map = cell::pos_map_template<input_spot>;

    using  cells_map = cell::pos_map_template<cell>;
    using  cell_iterator = cells_map::iterator;

    input_spot();

    void  set_cell(cell_iterator const  cell) { m_cell = cell; }
    cell_iterator  cell() const noexcept { return m_cell; }

private:
    cell_iterator  m_cell;
};

struct  output_terminal
{
    using  pos_hasher = cell::pos_hasher;
    using  pos_equal = cell::pos_equal;
    using  pos_map = cell::pos_map_template<output_terminal>;

    using  cells_map = cell::pos_map_template<cell>;
    using  cell_iterator = cells_map::iterator;

    output_terminal();

    void  set_cell(cell_iterator const  cell) { m_cell = cell; }
    cell_iterator  cell() const noexcept { return m_cell; }

private:
    cell_iterator  m_cell;
};

struct nenet
{
    nenet(
        vector3 const&  lo_corner, vector3 const&  hi_corner,
        natural_8_bit const  num_cells_x, natural_8_bit const  num_cells_y, natural_8_bit const  num_cells_c,
        natural_16_bit const  max_num_inputs_to_cell,
        scalar const percentage_of_territories_overlap
        );

    vector3 const&  lo_corner() const noexcept { return m_lo_corner; }
    vector3 const&  hi_corner() const noexcept { return m_hi_corner; }

    natural_8_bit  num_cells_x() const noexcept { return m_num_cells_x; }
    natural_8_bit  num_cells_y() const noexcept { return m_num_cells_y; }
    natural_8_bit  num_cells_c() const noexcept { return m_num_cells_c; }

    natural_16_bit  max_num_inputs_to_cell() const noexcept { return m_max_num_inputs_to_cell; }
    
    scalar  percentage_of_territories_overlap() const noexcept { return m_percentage_of_territories_overlap; }

    vector3 const&  intercell_distance() const noexcept { return  m_intercell_distance; }
    vector3 const&  cells_origin() const noexcept { return  m_cells_origin; }
    cell::pos_map const&  cells() const noexcept { return  m_cells; }

    natural_16_bit  num_spots_x() const noexcept { return  m_num_spots_x; }
    natural_16_bit  num_spots_y() const noexcept { return  m_num_spots_y; }
    natural_16_bit  num_spots_c() const noexcept { return  m_num_spots_c; }
    vector3 const&  interspot_distance() const noexcept { return  m_interspot_distance; }
    vector3 const&  spots_origin() const noexcept { return  m_spots_origin; }
    input_spot::pos_map const&  input_spots() const noexcept { return  m_input_spots; }

    output_terminal::pos_map const&  output_terminals() const noexcept { return  m_output_terminals; }

    cell::pos_map::const_iterator  find_closest_cell(vector3 const&  origin, vector3 const&  ray, scalar const  radius,
                                                     scalar* const  param = nullptr) const;
    input_spot::pos_map::const_iterator  find_closest_input_spot(vector3 const&  origin, vector3 const&  ray, scalar const  radius,
                                                                 scalar* const  param = nullptr) const;
    output_terminal::pos_map::const_iterator  find_closest_output_terminal(vector3 const&  origin, vector3 const&  ray, scalar const  radius,
                                                                           scalar* const  param = nullptr) const;

    static float_64_bit  update_time_step_in_seconds() noexcept { return 0.001; }
    void  update();

private:
    
    vector3  m_lo_corner;
    vector3  m_hi_corner;

    natural_8_bit  m_num_cells_x;
    natural_8_bit  m_num_cells_y;
    natural_8_bit  m_num_cells_c;

    natural_16_bit  m_max_num_inputs_to_cell;

    scalar  m_percentage_of_territories_overlap;

    vector3  m_intercell_distance;
    vector3  m_cells_origin;
    cell::pos_map  m_cells;

    natural_16_bit  m_num_spots_x;
    natural_16_bit  m_num_spots_y;
    natural_16_bit  m_num_spots_c;
    vector3  m_interspot_distance;
    vector3  m_spots_origin;
    input_spot::pos_map  m_input_spots;

    output_terminal::pos_map  m_output_terminals;
};



#endif
