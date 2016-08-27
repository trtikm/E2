#ifndef NENET_HPP_INCLUDED
#   define NENET_HPP_INCLUDED

#   include <utility/tensor_math.hpp>
#   include <vector>
#   include <unordered_map>
#   include <unordered_set>
#   include <memory>

struct  input_spot;
struct  output_terminal;

struct cell
{
    struct  pos_hasher
    {
        pos_hasher(vector3 const&  origin, vector3 const&  intercell_distance,
                   natural_64_bit const  num_cells_x, natural_64_bit const  num_cells_y, natural_64_bit const  num_cells_c);

        std::size_t  operator()(vector3 const&  pos) const;

        std::size_t  bucket(natural_64_bit const  x, natural_64_bit const  y, natural_64_bit const  c) const;
        void  bucket_indices(vector3 const&  pos, natural_64_bit&  x, natural_64_bit&  y, natural_64_bit&  c) const;
        vector3  bucket_centre(natural_64_bit const  x, natural_64_bit const  y, natural_64_bit const  c) const;
        vector3  bucket_centre(vector3 const&  pos) const;

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

    using  output_terminal_ptr = output_terminal*;

    cell();

    void  add_input_spot(input_spot_iterator const  ispot) { m_input_spots.push_back(ispot); }
    std::vector<input_spot_iterator> const&  input_spots() const noexcept { return m_input_spots; }

    void  add_output_terminal(output_terminal_ptr const  oterm) { m_output_terminals.push_back(oterm); }
    std::vector<output_terminal_ptr> const&  output_terminals() const noexcept { return m_output_terminals; }

    vector3 const&  output_area_center() const noexcept { return m_output_area_center; }
    void  set_output_area_center(vector3 const&  center) { m_output_area_center = center; }

    scalar  spiking_potential() const noexcept { return m_spiking_potential; }
    void  set_spiking_potential(scalar const  value) { m_spiking_potential = value; }

    natural_64_bit  last_update() const noexcept { return m_last_update; }
    void  set_last_update(natural_64_bit const  value) { m_last_update = value; }

    bool  is_excitatory() const noexcept { return m_is_excitatory; }
    bool  is_inhibitory() const noexcept { return !is_excitatory(); }

private:
    std::vector<input_spot_iterator>  m_input_spots;
    std::vector<output_terminal_ptr>  m_output_terminals;
    vector3  m_output_area_center;
    scalar  m_spiking_potential;
    natural_64_bit  m_last_update;
    bool  m_is_excitatory;
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
    struct  pos_hasher
    {
        pos_hasher(vector3 const&  origin, vector3 const&  intercell_distance,
            natural_64_bit const  num_cells_x, natural_64_bit const  num_cells_y, natural_64_bit const  num_cells_c);

        std::size_t  operator()(std::pair<vector3, output_terminal*> const&  key) const;

        std::size_t  bucket(natural_64_bit const  x, natural_64_bit const  y, natural_64_bit const  c) const;
        void  bucket_indices(vector3 const&  pos, natural_64_bit&  x, natural_64_bit&  y, natural_64_bit&  c) const;
        vector3  bucket_centre(natural_64_bit const  x, natural_64_bit const  y, natural_64_bit const  c) const;
        vector3  bucket_centre(vector3 const&  pos) const;

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
        bool  operator()(std::pair<vector3, output_terminal*> const&  l, std::pair<vector3, output_terminal*> const&  r) const;
        vector3 const&  max_distance() const noexcept { return  m_max_distance; }
    private:
        vector3  m_max_distance;
    };

    using  pos_set = std::unordered_set< std::pair<vector3,output_terminal*>, pos_hasher, pos_equal>;

    using  cells_map = cell::pos_map_template<cell>;
    using  cell_iterator = cells_map::iterator;

    output_terminal();

    vector3 const&  pos() const noexcept { return m_pos; }
    void  set_pos(vector3 const&  pos) { m_pos = pos; }

    vector3 const&  velocity() const noexcept { return m_velocity; }
    void  set_velocity(vector3 const&  v) { m_velocity = v; }

    cell_iterator  cell() const noexcept { return m_cell; }
    void  set_cell(cell_iterator const  cell) { m_cell = cell; }

    float_32_bit  synaptic_weight() const noexcept { return m_synaptic_weight; }
    void  set_synaptic_weight(float_32_bit const  w) { m_synaptic_weight = w; }

private:
    vector3  m_pos;
    vector3  m_velocity;
    cell_iterator  m_cell;

    float_32_bit  m_synaptic_weight;
};

struct nenet
{
    struct params
    {
        static std::shared_ptr<params>  create_default();

        params(
            scalar const  update_time_step_in_seconds,
            scalar const  mini_spiking_potential_magnitude,
            scalar const  average_mini_spiking_period_in_seconds,
            scalar const  spiking_potential_magnitude,
            scalar const  resting_potential,
            scalar const  spiking_threshold,
            scalar const  after_spike_potential,
            scalar const  potential_descend_coef,
            scalar const  potential_ascend_coef,
            scalar const  max_connection_distance,
            scalar const  output_terminal_velocity_max_magnitude,
            scalar const  output_terminal_velocity_min_magnitude
            );

        scalar  update_time_step_in_seconds() const noexcept { return m_update_time_step_in_seconds; }

        scalar  mini_spiking_potential_magnitude() const noexcept { return m_mini_spiking_potential_magnitude; }
        scalar  average_mini_spiking_period_in_seconds() const noexcept { return m_average_mini_spiking_period_in_seconds; }

        scalar  spiking_potential_magnitude() const noexcept { return m_spiking_potential_magnitude; }
        scalar  resting_potential() const noexcept { return m_resting_potential; }
        scalar  spiking_threshold() const noexcept { return m_spiking_threshold; }
        scalar  after_spike_potential() const noexcept { return m_after_spike_potential; }
        scalar  potential_descend_coef() const noexcept { return m_potential_descend_coef; }
        scalar  potential_ascend_coef() const noexcept { return m_potential_ascend_coef; }
        scalar  max_connection_distance() const noexcept { return m_max_connection_distance; }

        scalar  output_terminal_velocity_max_magnitude() const noexcept { return m_output_terminal_velocity_max_magnitude; }
        scalar  output_terminal_velocity_min_magnitude() const noexcept { return m_output_terminal_velocity_min_magnitude; }

        void  set_update_time_step_in_seconds(scalar const  value) { m_update_time_step_in_seconds = value; }

        void  set_mini_spiking_potential_magnitude(scalar const  value) { m_mini_spiking_potential_magnitude = value; }
        void  set_average_mini_spiking_period_in_seconds(scalar const  value) { m_average_mini_spiking_period_in_seconds = value; }

        void  set_spiking_potential_magnitude(scalar const  value) { m_spiking_potential_magnitude = value; }
        void  set_resting_potential(scalar const  value) { m_resting_potential = value; }
        void  set_spiking_threshold(scalar const  value) { m_spiking_threshold = value; }
        void  set_after_spike_potential(scalar const  value) { m_after_spike_potential = value; }
        void  set_potential_descend_coef(scalar const  value) { m_potential_descend_coef = value; }
        void  set_potential_ascend_coef(scalar const  value) { m_potential_ascend_coef = value; }
        void  set_max_connection_distance(scalar const  value) { m_max_connection_distance = value; }

        void  set_output_terminal_velocity_max_magnitude(scalar const  value) { m_output_terminal_velocity_max_magnitude = value; }
        void  set_output_terminal_velocity_min_magnitude(scalar const  value) { m_output_terminal_velocity_min_magnitude = value; }
    private:
        scalar  m_update_time_step_in_seconds;

        scalar  m_mini_spiking_potential_magnitude;
        scalar  m_average_mini_spiking_period_in_seconds;

        scalar  m_spiking_potential_magnitude;
        scalar  m_resting_potential;
        scalar  m_spiking_threshold;
        scalar  m_after_spike_potential;
        scalar  m_potential_descend_coef;
        scalar  m_potential_ascend_coef;
        scalar  m_max_connection_distance;

        scalar  m_output_terminal_velocity_max_magnitude;
        scalar  m_output_terminal_velocity_min_magnitude;
    };

    using  params_ptr = std::shared_ptr<params>;

    nenet(
        vector3 const&  lo_corner, vector3 const&  hi_corner,
        natural_8_bit const  num_cells_x, natural_8_bit const  num_cells_y, natural_8_bit const  num_cells_c,
        natural_16_bit const  max_num_inputs_to_cell,
        params_ptr const prms
        );

    vector3 const&  lo_corner() const noexcept { return m_lo_corner; }
    vector3 const&  hi_corner() const noexcept { return m_hi_corner; }

    natural_8_bit  num_cells_x() const noexcept { return m_num_cells_x; }
    natural_8_bit  num_cells_y() const noexcept { return m_num_cells_y; }
    natural_8_bit  num_cells_c() const noexcept { return m_num_cells_c; }

    natural_16_bit  max_num_inputs_to_cell() const noexcept { return m_max_num_inputs_to_cell; }

    params_ptr  get_params() const noexcept { return m_params; }

    vector3 const&  intercell_distance() const noexcept { return  m_intercell_distance; }
    vector3 const&  cells_origin() const noexcept { return  m_cells_origin; }
    cell::pos_map const&  cells() const noexcept { return  m_cells; }

    natural_16_bit  num_spots_x() const noexcept { return  m_num_spots_x; }
    natural_16_bit  num_spots_y() const noexcept { return  m_num_spots_y; }
    natural_16_bit  num_spots_c() const noexcept { return  m_num_spots_c; }
    vector3 const&  interspot_distance() const noexcept { return  m_interspot_distance; }
    vector3 const&  spots_origin() const noexcept { return  m_spots_origin; }
    input_spot::pos_map const&  input_spots() const noexcept { return  m_input_spots; }

    output_terminal::pos_set const&  output_terminals_set() const noexcept { return  m_output_terminals_set; }
    std::vector<output_terminal> const&  output_terminals() const noexcept { return  m_output_terminals; }

    cell::pos_map::const_iterator  find_closest_cell(vector3 const&  origin, vector3 const&  ray, scalar const  radius,
                                                     scalar* const  param = nullptr) const;
    input_spot::pos_map::const_iterator  find_closest_input_spot(vector3 const&  origin, vector3 const&  ray, scalar const  radius,
                                                                 scalar* const  param = nullptr) const;
    output_terminal::pos_set::const_iterator  find_closest_output_terminal(vector3 const&  origin, vector3 const&  ray, scalar const  radius,
                                                                           scalar* const  param = nullptr) const;

    natural_64_bit  num_passed_updates() const noexcept { return  update_id(); }
    natural_64_bit  update_id() const noexcept { return  m_update_id; }

    void  update(
        bool const  use_spiking = true,
        bool const  use_mini_spiking = true,
        bool const  use_movement_of_teminals = true
        );

private:

    void  update_spiking(bool const  update_only_potential = false);
    void  update_mini_spiking();
    void  update_movement_of_output_terminals();

    vector3  m_lo_corner;
    vector3  m_hi_corner;

    natural_8_bit  m_num_cells_x;
    natural_8_bit  m_num_cells_y;
    natural_8_bit  m_num_cells_c;

    natural_16_bit  m_max_num_inputs_to_cell;

    params_ptr  m_params;
        
    vector3  m_intercell_distance;
    vector3  m_cells_origin;
    cell::pos_map  m_cells;

    natural_16_bit  m_num_spots_x;
    natural_16_bit  m_num_spots_y;
    natural_16_bit  m_num_spots_c;
    vector3  m_interspot_distance;
    vector3  m_spots_origin;
    input_spot::pos_map  m_input_spots;

    output_terminal::pos_set  m_output_terminals_set;
    std::vector<output_terminal>  m_output_terminals;

    natural_64_bit  m_update_id;
    
    std::unique_ptr< std::unordered_set<cell*> >  m_current_spikers;
    std::unique_ptr< std::unordered_set<cell*> >  m_next_spikers;
};


#endif
