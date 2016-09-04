#include "./nenet.hpp"
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <unordered_set>
#include <algorithm>

namespace {


vector3  compute_intercell_distance(
    vector3 const&  lo_corner, vector3 const&  hi_corner,
    natural_8_bit const  num_cells_x, natural_8_bit const  num_cells_y, natural_8_bit const  num_cells_c
    )
{
    ASSUMPTION(lo_corner(0) < hi_corner(0) && lo_corner(1) < hi_corner(1) && lo_corner(2) < hi_corner(2));
    ASSUMPTION(num_cells_x != 0 && num_cells_y != 0 && num_cells_c != 0);
    vector3 const  u = hi_corner - lo_corner;
    return vector3{ u(0)/(scalar)num_cells_x, u(1) / (scalar)num_cells_y, u(2) / (scalar)num_cells_c };
}

vector3  compute_interspot_distance(
    vector3 const&  lo_corner, vector3 const&  hi_corner, natural_16_bit const  max_num_inputs_to_cell,
    natural_8_bit const  num_cells_x, natural_8_bit const  num_cells_y, natural_8_bit const  num_cells_c,
    natural_16_bit&  num_spots_x,  natural_16_bit&  num_spots_y, natural_16_bit&  num_spots_c
    )
{
    ASSUMPTION(lo_corner(0) < hi_corner(0) && lo_corner(1) < hi_corner(1) && lo_corner(2) < hi_corner(2));
    ASSUMPTION(num_cells_x != 0 && num_cells_y != 0 && num_cells_c != 0);
    ASSUMPTION(max_num_inputs_to_cell != 0);
    num_spots_x = num_spots_y = num_spots_c = 1;
    for (int  i = 0; num_spots_x * num_spots_y * num_spots_c < max_num_inputs_to_cell; i = (i + 1) % 3)
        switch (i)
        {
        case 0: ++num_spots_x; break;
        case 1: ++num_spots_y; break;
        case 2: ++num_spots_c; break;
        default: UNREACHABLE();
        }
    num_spots_x *= num_cells_x;
    num_spots_y *= num_cells_y;
    num_spots_c *= num_cells_c;
    vector3 const  u = hi_corner - lo_corner;
    return vector3{ u(0) / (scalar)num_spots_x, u(1) / (scalar)num_spots_y, u(2) / (scalar)num_spots_c };
}

using  holes_set = std::unordered_set<vector3, cell::pos_hasher, cell::pos_equal>;

void  compute_holes(
    //cell::pos_hasher const&  hasher,
    vector3 const&  origin, vector3 const&  interelem_distance,
    natural_8_bit const  num_clusters_x, natural_8_bit const  num_clusters_y, natural_8_bit const  num_clusters_c,
    natural_16_bit const  num_elems_x, natural_16_bit const  num_elems_y, natural_16_bit const  num_elems_c,
    natural_16_bit const  num_valid_elems_in_cluster, holes_set&  holes)
{
    ASSUMPTION(num_clusters_x != 0 && num_clusters_y != 0 && num_clusters_c != 0);
    ASSUMPTION(num_elems_x >= num_clusters_x && num_elems_y >= num_clusters_y && num_elems_c >= num_clusters_c);
    ASSUMPTION(num_elems_x % num_clusters_x == 0U && num_elems_y % num_clusters_y == 0U && num_elems_c % num_clusters_c == 0U);
    ASSUMPTION(holes.empty());
    natural_16_bit const  dx = num_elems_x / num_clusters_x;
    natural_16_bit const  dy = num_elems_y / num_clusters_y;
    natural_16_bit const  dc = num_elems_c / num_clusters_c;
    ASSUMPTION(dx * dy * dc >= num_valid_elems_in_cluster);
    natural_16_bit const  nholes = dx * dy * dc - num_valid_elems_in_cluster;
    for (natural_8_bit x = 0; x != num_elems_x; x += dx)
        for (natural_8_bit y = 0; y != num_elems_y; y += dy)
            for (natural_8_bit c = 0; c != num_elems_c; c += dc)
                for (natural_16_bit i = 0; i != nholes; ++i)
                    while (true)
                    {
                        natural_16_bit const  hx = get_random_natural_32_bit_in_range(0U, dx - 1U);
                        natural_16_bit const  hy = get_random_natural_32_bit_in_range(0U, dy - 1U);
                        natural_16_bit const  hc = get_random_natural_32_bit_in_range(0U, dc - 1U);
                        vector3 const  u = origin + (vector3((scalar)(x+hx), (scalar)(y+hy), (scalar)(c+hc)).array()
                                                            * interelem_distance.array()).matrix();
                        if (holes.count(u) == 0ULL)
                        {
                            holes.insert(u);
                            break;
                        }
                    }
    INVARIANT(holes.size() ==
            (natural_64_bit)nholes
                * (natural_64_bit)num_clusters_x
                * (natural_64_bit)num_clusters_y
                * (natural_64_bit)num_clusters_c
        );
}

template<typename T>
void  init_pos_map(
    cell::pos_map_template<T>&  cmap, vector3 const&  origin, vector3 const&  interelem_distance,
    natural_16_bit const  num_elems_x, natural_16_bit const  num_elems_y, natural_16_bit const  num_elems_c,
    std::unordered_set<vector3, cell::pos_hasher, cell::pos_equal>&  holes)
{
    ASSUMPTION(cmap.empty());
    ASSUMPTION(interelem_distance(0) > 0.0f && interelem_distance(1) > 0.0f && interelem_distance(2) > 0.0f);
    ASSUMPTION(num_elems_x != 0 && num_elems_y != 0 && num_elems_c != 0);
    for (natural_16_bit  x = 0; x != num_elems_x; ++x)
        for (natural_16_bit y = 0; y != num_elems_y; ++y)
            for (natural_16_bit c = 0; c != num_elems_c; ++c)
            {
                vector3 const  u = origin + (vector3(x, y, c).array() * interelem_distance.array()).matrix();
                if (holes.count(u) == 0ULL)
                {
                    auto const  result = cmap.insert({u,{}});
                    INVARIANT(result.second);
                }
            }
    INVARIANT(cmap.hash_function().num_buckets() >= (natural_64_bit)num_elems_x * (natural_64_bit)num_elems_y * (natural_64_bit)num_elems_c);
    INVARIANT(cmap.size() + holes.size() == (natural_64_bit)num_elems_x * (natural_64_bit)num_elems_y * (natural_64_bit)num_elems_c);
}

void  init_output_areas(cell::pos_map&  map)
{
    for (auto&  pos_cell : map)
    {
        pos_cell.second.set_output_area_center(pos_cell.first);
        //pos_cell.second.set_output_area_radius(radius);
    }
    vector3 const&  origin = map.hash_function().origin();
    vector3 const&  inter_distance = map.hash_function().inter_distance();
    vector3 const  d = 0.5f * inter_distance;
    natural_32_bit const  size_x = (natural_32_bit)map.hash_function().size_x();
    natural_32_bit const  size_y = (natural_32_bit)map.hash_function().size_y();
    natural_32_bit const  size_c = (natural_32_bit)map.hash_function().size_c();
    for (auto& pos_cell : map)
        while (pos_cell.first(0) == pos_cell.second.output_area_center()(0) &&
               pos_cell.first(1) == pos_cell.second.output_area_center()(1) &&
               pos_cell.first(2) == pos_cell.second.output_area_center()(2) )
        {
            scalar  const  x = (scalar)get_random_natural_32_bit_in_range(0U, size_x - 1U);
            scalar  const  y = (scalar)get_random_natural_32_bit_in_range(0U, size_y - 1U);
            scalar  const  c = (scalar)get_random_natural_32_bit_in_range(0U, size_c - 1U);
            vector3 const  u = origin + (vector3(x, y, c).array() * inter_distance.array()).matrix();
            cell::pos_map::iterator const  it = map.find(u);
            INVARIANT(it != map.end());
            vector3 const&  v = it->first - pos_cell.first;
            vector3 const  w = it->second.output_area_center() - pos_cell.first;
            if (std::abs(v(0)) + std::abs(v(1)) + std::abs(v(2)) > d(0) + d(1) + d(2) &&
                std::abs(w(0)) + std::abs(w(1)) + std::abs(w(2)) > d(0) + d(1) + d(2) )
            {
                INVARIANT(&it->second != &pos_cell.second);
                pos_cell.second.set_output_area_center(it->second.output_area_center());
                it->second.set_output_area_center(pos_cell.first);
            }
        }
}

void  init_output_terminals(std::vector<output_terminal>&  oterms, output_terminal::pos_set&  oset, cell::pos_map&  cmap,
                            natural_16_bit const  num_terminals_per_cell)
{
    //vector3 const  origin = omap.hash_function().origin() - 0.5f * omap.hash_function().inter_distance();
    oterms.reserve(cmap.size() * num_terminals_per_cell);
    std::unordered_map<cell::pos_map::value_type*, cell::pos_map::value_type*>  filled;
    for (cell::pos_map::iterator it = cmap.begin(); it != cmap.end(); ++it)
    {
        auto const  oit = cmap.find(it->second.output_area_center());
        cell::pos_map::value_type* ptr = &*oit;
        auto const  fit = filled.find(ptr);
        INVARIANT(fit == filled.cend());
        filled.insert({ ptr,&*it});

        vector3 const  lo = it->second.output_area_center() - 0.5f * cmap.hash_function().inter_distance();
        vector3 const  hi = lo + cmap.hash_function().inter_distance();
        for (natural_16_bit i = 0U; i != num_terminals_per_cell; ++i)
        {
            oterms.push_back({});
            while (true)
            {
                scalar const  tx = get_random_natural_32_bit_in_range(0U, 999U) / 1000.0f;
                scalar const  ty = get_random_natural_32_bit_in_range(0U, 999U) / 1000.0f;
                scalar const  tc = get_random_natural_32_bit_in_range(0U, 999U) / 1000.0f;
                vector3 const  u = lo + (vector3(tx,ty,tc).array() * (hi - lo).array()).matrix();
                auto const  result = oset.insert({ u,&oterms.back() });
                if (result.second)
                {
                    oterms.back().set_pos(u);
                    oterms.back().set_cell(it);
                    it->second.add_output_terminal(&oterms.back());
                    break;
                }
            }
        }
    }
}


//template<typename T>
//void  init_pos_random(
//    cell::pos_map_template<T>&  cmap, vector3 const&  lo, vector3 const&  hi,
//    natural_64_bit const  num_elements_to_initialise)
//{
//    ASSUMPTION(cmap.empty());
//    for (natural_64_bit i = 0; i != num_elements_to_initialise; ++i)
//        while (true)
//        {
//            scalar const  tx = get_random_natural_32_bit_in_range(0U, 999U) / 1000.0f;
//            scalar const  ty = get_random_natural_32_bit_in_range(0U, 999U) / 1000.0f;
//            scalar const  tc = get_random_natural_32_bit_in_range(0U, 999U) / 1000.0f;
//            vector3 const  u = lo + (vector3(tx,ty,tc).array() * (hi - lo).array()).matrix();
//            auto const  result = cmap.insert({ u,{} });
//            if (result.second)
//                break;
//        }
//}

void  interconnect_cells_with_input_spots(
    cell::pos_map&  cmap, input_spot::pos_map&  imap)
{
    for (input_spot::pos_map::iterator it = imap.begin(); it != imap.end(); ++it)
    {
        cell::pos_map::iterator const  cit = cmap.find(it->first);
        ASSUMPTION(cit != cmap.end());
        it->second.set_cell(cit);
    }

    // TODO: here do some permutations of spots inbetween cells according to 'percentage_of_territories_overlap'

    //natural_64_bit const  dx = imap.hash_function().size_x() / cmap.hash_function().size_x();
    //natural_64_bit const  dy = imap.hash_function().size_y() / cmap.hash_function().size_y();
    //natural_64_bit const  dc = imap.hash_function().size_c() / cmap.hash_function().size_c();
    //for (natural_64_bit x = 0; x != imap.hash_function().size_x(); ++x)
    //    for (natural_64_bit y = 0; y != imap.hash_function().size_y(); ++y)
    //        for (natural_64_bit c = 0; c != imap.hash_function().size_c(); ++c)
    //        {
    //            natural_64_bit const cx = x / dx;
    //        }


    for (input_spot::pos_map::iterator it = imap.begin(); it != imap.end(); ++it)
        it->second.cell()->second.add_input_spot(it);
}

template<typename T>
typename T::const_iterator
find_closest_element(T const&  dict, vector3 const&  origin, vector3 const&  ray, scalar const  radius, scalar* const  param)
{
    scalar const ray_dot = dot_product(ray, ray);
    ASSUMPTION(ray_dot > 1e-3f);
    scalar  result_t = (param != nullptr) ? *param : 1e30f;
    typename T::const_iterator  result = dict.cend();
    for (typename T::const_iterator it = dict.cbegin(); it != dict.cend(); ++it)
    {
        scalar const  t = dot_product(ray, it->first - origin) / ray_dot;
        if (t >= 1.0f)
        {
            vector3 const  X = origin + t * ray;
            vector3 const  u = it->first - X;
            scalar const  dist2 = dot_product(u, u);
            if (dist2 <= radius * radius && (result == dict.cend() || result_t > t))
            {
                result = it;
                result_t = t;
            }
        }
    }
    if (param != nullptr)
        *param = result_t;
    return result;
}

//template<typename T>
//typename cell::pos_map_template<T>::const_iterator
//find_closest_element(cell::pos_map_template<T> const&  map,
//                     vector3 const&  origin, vector3 const&  ray, scalar const  radius,
//                     scalar* const  param)
//{
//    scalar const ray_dot = dot_product(ray, ray);
//    ASSUMPTION(ray_dot > 1e-3f);
//    scalar  result_t = (param != nullptr) ? *param : 1e30f;
//    typename cell::pos_map_template<T>::const_iterator  result = map.cend();
//    for (typename cell::pos_map_template<T>::const_iterator it = map.cbegin(); it != map.cend(); ++it)
//    {
//        scalar const  t = dot_product(ray, it->first - origin) / ray_dot;
//        if (t >= 1.0f)
//        {
//            vector3 const  X = origin + t * ray;
//            vector3 const  u = it->first - X;
//            scalar const  dist2 = dot_product(u, u);
//            if (dist2 <= radius * radius && (result == map.cend() || result_t > t))
//            {
//                result = it;
//                result_t = t;
//            }
//        }
//    }
//    if (param != nullptr)
//        *param = result_t;
//    return result;
//}


void  update_potential_of_cell_to_current_time(cell* const  pcell, natural_64_bit const  update_id, nenet::params_ptr const  params,
                                               stats_of_cell* const  stats_cell)
{
    if (pcell->last_update() < update_id)
    {
        scalar const  dt = 1000.0f * (scalar)params->update_time_step_in_seconds();
        scalar  v = pcell->spiking_potential();
        for (natural_64_bit i = pcell->last_update(); i != update_id; ++i)
        {
            scalar const  coef = v < 0.0f ? params->potential_ascend_coef() : params->potential_descend_coef();
            scalar const  dvdt = -coef * (v - params->resting_potential());
            v = v + dt * dvdt;
        }
        pcell->set_spiking_potential(v);
        pcell->set_last_update(update_id);

        if (stats_cell != nullptr && pcell == &stats_cell->get_cell())
        {
        }
    }
}

output_terminal*  find_connected_output_terminal(output_terminal::pos_set const&  set_of_output_temrinals,
                                                 vector3 const&  input_spot_position,
                                                 nenet::params_ptr const  params)
{
    natural_64_bit const  obucket = set_of_output_temrinals.bucket({ input_spot_position,nullptr });
    for (auto it = set_of_output_temrinals.begin(obucket), end = set_of_output_temrinals.end(obucket); it != end; ++it)
    {
        vector3 const  u = input_spot_position - it->first;
        scalar const  dist = length(u);
        if (dist <= params->max_connection_distance())
            return it->second;
    }
    return nullptr;
}

vector3  gen_random_velocity(scalar const  magnitude)
{
    while (true)
    {
        natural_32_bit const  tx = get_random_natural_32_bit_in_range(0U,10000U);
        natural_32_bit const  ty = get_random_natural_32_bit_in_range(0U, 10000U);
        natural_32_bit const  tc = get_random_natural_32_bit_in_range(0U, 10000U);
        vector3 const  u(((scalar)tx / 10000.0f) - 0.5f, ((scalar)ty / 10000.0f) - 0.5f, ((scalar)tc / 10000.0f) - 0.5f);
        scalar const  u_len = length(u);
        if (u_len >= 0.01f)
            return (magnitude / u_len) * u;
    }
}

vector3  update_magnitude_of_velocity(vector3 const&  v, scalar const  magnitude, nenet::params_ptr const  params)
{
    scalar const  speed = length(v);
    if (speed >= params->output_terminal_velocity_min_magnitude())
    {
        if (speed <= params->output_terminal_velocity_max_magnitude())
            return v;
        return (params->output_terminal_velocity_max_magnitude() / speed) * v;
    }
    return gen_random_velocity(magnitude);
}


}

cell::pos_hasher::pos_hasher(vector3 const&  origin, vector3 const&  intercell_distance,
                             natural_64_bit const  num_cells_x, natural_64_bit const  num_cells_y, natural_64_bit const  num_cells_c)
    : m_origin(origin)
    , m_intercell_distance(intercell_distance)
    , m_num_cells_x(num_cells_x)
    , m_num_cells_y(num_cells_y)
    , m_num_cells_c(num_cells_c)
{
    ASSUMPTION(m_intercell_distance(0) > 0.0f && m_intercell_distance(1) > 0.0f && m_intercell_distance(2) > 0.0f);
    ASSUMPTION(m_num_cells_x != 0 && m_num_cells_y != 0 && m_num_cells_c != 0);
}

std::size_t  cell::pos_hasher::operator()(vector3 const&  pos) const
{
    TMPROF_BLOCK();

    natural_64_bit  x, y, c;
    bucket_indices(pos, x, y, c);
    return bucket(x, y, c);
}

std::size_t  cell::pos_hasher::bucket(natural_64_bit const  x, natural_64_bit const  y, natural_64_bit const  c) const
{
    return c * (m_num_cells_x * m_num_cells_y) + y * m_num_cells_x + x;
}

void  cell::pos_hasher::bucket_indices(vector3 const&  pos, natural_64_bit&  x, natural_64_bit&  y, natural_64_bit&  c) const
{
    vector3 const  u = (pos - (m_origin - 0.5f * m_intercell_distance)).array() / m_intercell_distance.array();
    x = (u(0) <= 0.0) ? 0ULL : (u(0) >= m_num_cells_x) ? m_num_cells_x - 1ULL : (natural_64_bit)u(0);
    y = (u(1) <= 0.0) ? 0ULL : (u(1) >= m_num_cells_y) ? m_num_cells_y - 1ULL : (natural_64_bit)u(1);
    c = (u(2) <= 0.0) ? 0ULL : (u(2) >= m_num_cells_c) ? m_num_cells_c - 1ULL : (natural_64_bit)u(2);
}

vector3  cell::pos_hasher::bucket_centre(natural_64_bit const  x, natural_64_bit const  y, natural_64_bit const  c) const
{
    return m_origin + (vector3((scalar)x, (scalar)y, (scalar)c).array() * m_intercell_distance.array()).matrix();
}

vector3  cell::pos_hasher::bucket_centre(vector3 const&  pos) const
{
    TMPROF_BLOCK();

    natural_64_bit  x,y,c;
    bucket_indices(pos,x,y,c);
    return bucket_centre(x,y,c);
}

cell::pos_equal::pos_equal(vector3 const&  max_distance)
    : m_max_distance(max_distance)
{
    ASSUMPTION(m_max_distance(0) > 0.0f && m_max_distance(1) > 0.0f && m_max_distance(2) > 0.0f);
}

bool  cell::pos_equal::operator()(vector3 const&  l, vector3 const&  r) const
{
    TMPROF_BLOCK();

    vector3 const  u = l - r;
    return std::abs(u(0)) <= m_max_distance(0) && std::abs(u(1)) <= m_max_distance(1) && std::abs(u(2)) <= m_max_distance(2);
}


cell::cell()
    : m_input_spots()
    , m_output_terminals()
    , m_output_area_center(0.0f,0.0f,0.0f)
    , m_spiking_potential(0.0f)
    , m_last_update(0ULL)
    , m_is_excitatory(true)
{}

//cell::cell(vector3 const&  output_area_center, scalar const  output_area_radius)
//    : m_input_spots()
//    , m_output_terminals()
//    , m_output_area_center(output_area_center)
//    , m_output_area_radius(output_area_radius)
//{}

input_spot::input_spot()
    : m_cell()
{}


output_terminal::pos_hasher::pos_hasher(vector3 const&  origin, vector3 const&  intercell_distance,
    natural_64_bit const  num_cells_x, natural_64_bit const  num_cells_y, natural_64_bit const  num_cells_c)
    : m_origin(origin)
    , m_intercell_distance(intercell_distance)
    , m_num_cells_x(num_cells_x)
    , m_num_cells_y(num_cells_y)
    , m_num_cells_c(num_cells_c)
{
    ASSUMPTION(m_intercell_distance(0) > 0.0f && m_intercell_distance(1) > 0.0f && m_intercell_distance(2) > 0.0f);
    ASSUMPTION(m_num_cells_x != 0 && m_num_cells_y != 0 && m_num_cells_c != 0);
}

std::size_t  output_terminal::pos_hasher::operator()(std::pair<vector3, output_terminal*> const&  key) const
{
    TMPROF_BLOCK();

    natural_64_bit  x, y, c;
    bucket_indices(key.first, x, y, c);
    return bucket(x,y,c);
}

std::size_t  output_terminal::pos_hasher::bucket(natural_64_bit const  x, natural_64_bit const  y, natural_64_bit const  c) const
{
    return c * (m_num_cells_x * m_num_cells_y) + y * m_num_cells_x + x;
}

void  output_terminal::pos_hasher::bucket_indices(vector3 const&  pos, natural_64_bit&  x, natural_64_bit&  y, natural_64_bit&  c) const
{
    vector3 const  u = (pos - (m_origin - 0.5f * m_intercell_distance)).array() / m_intercell_distance.array();
    x = (u(0) <= 0.0) ? 0ULL : (u(0) >= m_num_cells_x) ? m_num_cells_x - 1ULL : (natural_64_bit)u(0);
    y = (u(1) <= 0.0) ? 0ULL : (u(1) >= m_num_cells_y) ? m_num_cells_y - 1ULL : (natural_64_bit)u(1);
    c = (u(2) <= 0.0) ? 0ULL : (u(2) >= m_num_cells_c) ? m_num_cells_c - 1ULL : (natural_64_bit)u(2);
}

vector3  output_terminal::pos_hasher::bucket_centre(natural_64_bit const  x, natural_64_bit const  y, natural_64_bit const  c) const
{
    return m_origin + (vector3((scalar)x, (scalar)y, (scalar)c).array() * m_intercell_distance.array()).matrix();
}

vector3  output_terminal::pos_hasher::bucket_centre(vector3 const&  pos) const
{
    TMPROF_BLOCK();

    natural_64_bit  x, y, c;
    bucket_indices(pos, x, y, c);
    return bucket_centre(x,y,c);
}

output_terminal::pos_equal::pos_equal(vector3 const&  max_distance)
    : m_max_distance(max_distance)
{
    ASSUMPTION(m_max_distance(0) > 0.0f && m_max_distance(1) > 0.0f && m_max_distance(2) > 0.0f);
}

bool  output_terminal::pos_equal::operator()(std::pair<vector3, output_terminal*> const&  l, std::pair<vector3, output_terminal*> const&  r) const
{
    TMPROF_BLOCK();
    return l.second == r.second;
}

output_terminal::output_terminal()
    : m_pos(0.0f,0.0f,0.0f)
    , m_velocity(vector3(0.0f, 0.0f, 0.0f))
    , m_cell()
    , m_synaptic_weight(1.0f)
{}


stats_of_input_spot::stats_of_input_spot(input_spot::pos_map::const_iterator const  ispot_iter, natural_64_bit const  start_update)
    : m_ispot_iter(ispot_iter)
    , m_start_update(start_update)

    , m_num_mini_spikes(0ULL)
    , m_last_mini_spike_update_id(m_start_update)
{}

scalar  stats_of_input_spot::average_mini_spikes_rate(scalar const  update_time_step_in_seconds) const
{
    scalar const  update_delta = (scalar)(last_mini_spike_update() - start_update()) * update_time_step_in_seconds;
    return (update_delta > std::max(update_time_step_in_seconds,1e-5f)) ? num_mini_spikes() / update_delta : 0.0f;
}

void  stats_of_input_spot::on_mini_spike(natural_64_bit const  update_id, output_terminal const* const  oterm)
{
    ASSUMPTION(m_last_mini_spike_update_id <= update_id);
    ++m_num_mini_spikes;
    m_last_mini_spike_update_id = update_id;
}


stats_of_output_terminal::stats_of_output_terminal(output_terminal const* const  oterm, natural_64_bit const  start_update)
    : m_oterm(oterm)
    , m_start_update(start_update)

    , m_num_mini_spikes(0ULL)
    , m_last_mini_spike_update_id(m_start_update)
{}

scalar  stats_of_output_terminal::average_mini_spikes_rate(scalar const  update_time_step_in_seconds) const
{
    scalar const  update_delta = (scalar)(last_mini_spike_update() - start_update()) * update_time_step_in_seconds;
    return (update_delta > std::max(update_time_step_in_seconds, 1e-5f)) ? num_mini_spikes() / update_delta : 0.0f;
}

void  stats_of_output_terminal::on_mini_spike(natural_64_bit const  update_id, input_spot::pos_map::const_iterator const  ispot_iter)
{
    ASSUMPTION(m_last_mini_spike_update_id <= update_id);
    ++m_num_mini_spikes;
    m_last_mini_spike_update_id = update_id;
}


stats_of_cell::stats_of_cell(cell::pos_map::const_iterator const  cell_iter, natural_64_bit const  start_update)
    : m_cell_iter(cell_iter)
    , m_start_update(start_update)

    , m_num_mini_spikes(0ULL)
    , m_last_mini_spike_update_id(m_start_update)

    , m_num_spikes(0ULL)
    , m_last_spike_update_id(m_start_update)
{}

scalar  stats_of_cell::average_mini_spikes_rate(scalar const  update_time_step_in_seconds) const
{
    scalar const  update_delta = (scalar)(last_mini_spike_update() - start_update()) * update_time_step_in_seconds;
    return (update_delta > std::max(update_time_step_in_seconds, 1e-5f)) ? num_mini_spikes() / update_delta : 0.0f;
}

scalar  stats_of_cell::average_spikes_rate(scalar const  update_time_step_in_seconds) const
{
    scalar const  update_delta = (scalar)(last_spike_update() - start_update()) * update_time_step_in_seconds;
    return (update_delta > std::max(update_time_step_in_seconds, 1e-5f)) ? num_spikes() / update_delta : 0.0f;
}

void  stats_of_cell::on_mini_spike(natural_64_bit const  update_id,
                                   input_spot::pos_map::const_iterator const  ispot_iter,
                                   output_terminal const* const  oterm)
{
    ASSUMPTION(m_last_mini_spike_update_id <= update_id);
    ++m_num_mini_spikes;
    m_last_mini_spike_update_id = update_id;
}

void  stats_of_cell::on_spike(natural_64_bit const  update_id)
{
    ASSUMPTION(m_last_spike_update_id <= update_id);
    ++m_num_spikes;
    m_last_spike_update_id = update_id;
}



std::shared_ptr<nenet::params>  nenet::params::create_default()
{
    return std::make_shared<nenet::params>(
        0.001f,                 //  update_time_step_in_seconds

        0.075f,                 //  mini_spiking_potential_magnitude
        10.0f / 1000.0f,        //  average_mini_spiking_period_in_seconds

        0.4f,                   //  spiking_potential_magnitude
        0.0f,                   //  resting_potential
        1.0f,                   //  spiking_threshold
        -1.0f,                  //  after_spike_potential
        0.2f,                   //  potential_descend_coef
        0.01f,                  //  potential_ascend_coef
        0.25f,                  //  max_connection_distance

        0.01f,                  //  output_terminal_velocity_max_magnitude
        0.003f                  //  output_terminal_velocity_min_magnitude
        );
}

nenet::params::params(
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
    )
    : m_update_time_step_in_seconds(update_time_step_in_seconds)
    , m_mini_spiking_potential_magnitude(mini_spiking_potential_magnitude)
    , m_average_mini_spiking_period_in_seconds(average_mini_spiking_period_in_seconds)
    , m_spiking_potential_magnitude(spiking_potential_magnitude)
    , m_resting_potential(resting_potential)
    , m_spiking_threshold(spiking_threshold)
    , m_after_spike_potential(after_spike_potential)
    , m_potential_descend_coef(potential_descend_coef)
    , m_potential_ascend_coef(potential_ascend_coef)
    , m_max_connection_distance(max_connection_distance)
    , m_output_terminal_velocity_max_magnitude(output_terminal_velocity_max_magnitude)
    , m_output_terminal_velocity_min_magnitude(output_terminal_velocity_min_magnitude)
{}

nenet::nenet(
    vector3 const&  lo_corner, vector3 const&  hi_corner,
    natural_8_bit const  num_cells_x, natural_8_bit const  num_cells_y, natural_8_bit const  num_cells_c,
    natural_16_bit const  max_num_inputs_to_cell,
    params_ptr const prms
    )
    : m_lo_corner(lo_corner)
    , m_hi_corner(hi_corner)

    , m_num_cells_x(num_cells_x)
    , m_num_cells_y(num_cells_y)
    , m_num_cells_c(num_cells_c)

    , m_max_num_inputs_to_cell(max_num_inputs_to_cell)

    , m_params(prms)

    , m_intercell_distance(compute_intercell_distance(m_lo_corner, m_hi_corner, m_num_cells_x, m_num_cells_y, m_num_cells_c))
    , m_cells_origin(m_lo_corner + 0.5f * m_intercell_distance)
    , m_cells(
        (natural_64_bit)m_num_cells_x
            * (natural_64_bit)m_num_cells_y
            * (natural_64_bit)m_num_cells_c,
        cell::pos_hasher(m_cells_origin,m_intercell_distance, m_num_cells_x,m_num_cells_y,m_num_cells_c),
        cell::pos_equal(0.5f * m_intercell_distance)
        )

    , m_num_spots_x(0)
    , m_num_spots_y(0)
    , m_num_spots_c(0)
    , m_interspot_distance(compute_interspot_distance(m_lo_corner, m_hi_corner, m_max_num_inputs_to_cell,
                                                      m_num_cells_x, m_num_cells_y, m_num_cells_c,
                                                      m_num_spots_x, m_num_spots_y, m_num_spots_c))
    , m_spots_origin(m_lo_corner + 0.5f * m_interspot_distance)
    , m_input_spots(
        (natural_64_bit)m_num_cells_x
            * (natural_64_bit)m_num_cells_y
            * (natural_64_bit)m_num_cells_c
            * (natural_64_bit)m_max_num_inputs_to_cell,
        cell::pos_hasher(m_spots_origin, m_interspot_distance, m_num_spots_x, m_num_spots_y, m_num_spots_c),
        cell::pos_equal(0.5f * m_interspot_distance)
        )

    , m_output_terminals()
    , m_output_terminals_set(
        m_input_spots.bucket_count(),
        output_terminal::pos_hasher(m_spots_origin, m_interspot_distance, m_num_spots_x, m_num_spots_y, m_num_spots_c),
        output_terminal::pos_equal(0.5f * m_interspot_distance)
        )

    , m_update_id(0ULL)

    , m_current_spikers(new std::unordered_set<cell*>)
    , m_next_spikers(new std::unordered_set<cell*>)

    , m_mini_spiking_random_generator()
{
    TMPROF_BLOCK();

    ASSUMPTION(m_lo_corner(0) < m_hi_corner(0) && m_lo_corner(1) < m_hi_corner(1) && m_lo_corner(2) < m_hi_corner(2));
    ASSUMPTION(m_num_cells_x != 0 && m_num_cells_y != 0 && m_num_cells_c != 0);
    ASSUMPTION(m_max_num_inputs_to_cell != 0);
    ASSUMPTION(m_intercell_distance(0) > 0.0f && m_intercell_distance(1) > 0.0f && m_intercell_distance(2) > 0.0f);

    init_pos_map(m_cells,m_cells_origin,m_intercell_distance,m_num_cells_x,m_num_cells_y,m_num_cells_c,holes_set(1U, m_cells.hash_function(), m_cells.key_eq()));

    {
        holes_set  holes(1U, m_input_spots.hash_function(), m_input_spots.key_eq());
        compute_holes(
            m_spots_origin,m_interspot_distance,
            m_num_cells_x, m_num_cells_y, m_num_cells_c,
            m_num_spots_x, m_num_spots_y, m_num_spots_c,
            m_max_num_inputs_to_cell,holes
            );
        init_pos_map(m_input_spots, m_spots_origin, m_interspot_distance, m_num_spots_x, m_num_spots_y, m_num_spots_c,holes);
    }

    interconnect_cells_with_input_spots(m_cells, m_input_spots);

    init_output_areas(m_cells);
    init_output_terminals(m_output_terminals, m_output_terminals_set, m_cells, m_max_num_inputs_to_cell);

    //init_pos_random(m_output_terminals, m_lo_corner, m_hi_corner, m_input_spots.size());

}

cell::pos_map::const_iterator  nenet::find_closest_cell(vector3 const&  origin, vector3 const&  ray, scalar const  radius,
                                                        scalar* const  param) const
{
    return find_closest_element(cells(), origin, ray, radius, param);
}

input_spot::pos_map::const_iterator  nenet::find_closest_input_spot(vector3 const&  origin, vector3 const&  ray, scalar const  radius,
                                                                    scalar* const  param) const
{
    return find_closest_element(input_spots(),origin,ray,radius,param);
}

output_terminal::pos_set::const_iterator  nenet::find_closest_output_terminal(vector3 const&  origin, vector3 const&  ray, scalar const  radius,
                                                                              scalar* const  param) const
{
    return find_closest_element(output_terminals_set(), origin, ray, radius, param);
}

void  nenet::update(
    bool const  use_spiking,
    bool const  use_mini_spiking,
    bool const  use_movement_of_teminals,
    stats_of_input_spot* const  stats_ispot,
    stats_of_output_terminal* const  stats_oterm,
    stats_of_cell* const  stats_cell
    )
{
    ++m_update_id;

    if (use_spiking || use_mini_spiking)
        update_spiking(!use_spiking, stats_ispot, stats_oterm, stats_cell);
    if (use_mini_spiking)
        update_mini_spiking(stats_ispot, stats_oterm, stats_cell);

    std::swap(m_current_spikers, m_next_spikers);
    m_next_spikers->clear();

    if (use_movement_of_teminals)
        update_movement_of_output_terminals();
}

void  nenet::update_spiking(
    bool const  update_only_potential,
    stats_of_input_spot* const  stats_ispot,
    stats_of_output_terminal* const  stats_oterm,
    stats_of_cell* const  stats_cell
    )
{
    for (auto  cit = m_current_spikers->begin(); cit != m_current_spikers->end(); ++cit)
    {
        cell* const  spiking_cell = *cit;

        spiking_cell->set_spiking_potential(get_params()->after_spike_potential());
        spiking_cell->set_last_update(update_id());

        if (stats_cell != nullptr && spiking_cell == &stats_cell->get_cell())
            stats_cell->on_spike(update_id());

        if (update_only_potential)
            continue;

        for (auto const  iit : spiking_cell->input_spots())
        {
            output_terminal*  oterm = find_connected_output_terminal(output_terminals_set(), iit->first,get_params());
            input_spot* const  ispot = &iit->second;

            if (oterm == nullptr)
            {
                // TODO: Put here code for 'on post-synaptic spike' for not-connected 'ispot'
            }
            else
            {
                {
                    // TODO: Put here code for 'on post-synaptic spike' for connected 'ispot'
                }
                {
                    // TODO: Put here code for 'on post-synaptic spike' for connected 'oterm'
                }
            }
        }

        for (auto const  oterm : spiking_cell->output_terminals())
        {
            auto const  iit = m_input_spots.find(oterm->pos());
            if (iit == input_spots().end() || length(iit->first - oterm->pos()) > get_params()->max_connection_distance())
            {
                // TODO: Put here code for 'on pre-synaptic spike' for not-connected 'oterm'
            }
            else
            {
                {
                    // TODO: Put here code for 'on pre-synaptic spike' for connected 'oterm'
                }

                input_spot* const  ispot = &iit->second;

                {
                    // TODO: Put here code for 'on pre-synaptic spike' for connected 'ispot'
                }

                cell* const  pcell = &ispot->cell()->second;
                {
                    update_potential_of_cell_to_current_time(pcell, update_id(), get_params(),stats_cell);

                    pcell->set_spiking_potential(
                        pcell->spiking_potential()
                        + (pcell->is_excitatory() ? 1.0f : -1.0f) * (oterm->synaptic_weight() * get_params()->spiking_potential_magnitude())
                        );

                    if (pcell->spiking_potential() >= get_params()->spiking_threshold())
                        m_next_spikers->insert(pcell);
                    else
                        m_next_spikers->erase(pcell);
                }
            }
        }
    }
}

void  nenet::update_mini_spiking(
    stats_of_input_spot* const  stats_ispot,
    stats_of_output_terminal* const  stats_oterm,
    stats_of_cell* const  stats_cell
    )
{
    ASSUMPTION(get_params()->average_mini_spiking_period_in_seconds() > 1e-5f);

    natural_32_bit  num_mini_spikes_to_generate = (natural_32_bit)
        std::round(m_input_spots.size() * (get_params()->update_time_step_in_seconds() / get_params()->average_mini_spiking_period_in_seconds()));
    for (natural_64_bit i = 0ULL; i < 2ULL * m_input_spots.size() && num_mini_spikes_to_generate != 0ULL; ++i)
    {
        natural_32_bit const  bucket = get_random_natural_32_bit_in_range(0U, (natural_32_bit)m_input_spots.hash_function().max_bucket(),
                                                                          m_mini_spiking_random_generator);
        auto it = m_input_spots.begin(bucket), end = m_input_spots.end(bucket);
        if (it != end)
        {
            INVARIANT(std::next(it) == end);

            output_terminal const*  oterm = find_connected_output_terminal(output_terminals_set(), it->first, get_params());
            if (oterm != nullptr)
            {
                cell* const  pcell = &it->second.cell()->second;

                update_potential_of_cell_to_current_time(pcell, update_id(), get_params(), stats_cell);

                pcell->set_spiking_potential(
                    pcell->spiking_potential()
                    + (oterm->cell()->second.is_excitatory() ? 1.0f : -1.0f) * get_params()->mini_spiking_potential_magnitude()
                    );

                if (pcell->spiking_potential() >= get_params()->spiking_threshold())
                    m_next_spikers->insert(pcell);
                else
                    m_next_spikers->erase(pcell);

                --num_mini_spikes_to_generate;

                if (stats_ispot != nullptr && it == stats_ispot->ispot_iter())
                    stats_ispot->on_mini_spike(update_id(), oterm);
                if (stats_oterm != nullptr && oterm == &stats_oterm->get_output_terminal())
                    stats_oterm->on_mini_spike(update_id(), it);
                if (stats_cell != nullptr && pcell == &stats_cell->get_cell())
                    stats_cell->on_mini_spike(update_id(), it, oterm);
            }
        }
    }
}

void  nenet::update_movement_of_output_terminals()
{
    TMPROF_BLOCK();

    for (output_terminal&  oterm : m_output_terminals)
    {
        vector3  gradient(0.0f,0.0f,0.0f);

        if (true)
        {
            TMPROF_BLOCK();

            scalar const  A = 0.0001f;
            scalar const  D = length(intercell_distance());
            vector3 const  u = oterm.pos() - oterm.cell()->second.output_area_center();
            vector3  grad_f = -((A / (D*D)) * dot_product(u,u)) * u;
            scalar const  u_lenght = length(u);
            if (u_lenght > 1e-6f)
                grad_f /= u_lenght;
            gradient += grad_f;
        }

        if (true)
        {
            TMPROF_BLOCK();

            natural_64_bit  xlo,ylo,clo;
            output_terminals_set().hash_function().bucket_indices(oterm.pos() - interspot_distance(),xlo,ylo,clo);
            natural_64_bit  xhi, yhi, chi;
            output_terminals_set().hash_function().bucket_indices(oterm.pos() + interspot_distance(), xhi, yhi, chi);
            for (natural_64_bit  x = xlo; x <= xhi; ++x)
                for (natural_64_bit y = ylo; y <= yhi; ++y)
                    for (natural_64_bit c = clo; c <= chi; ++c)
                    {
                        vector3 const  C = output_terminals_set().hash_function().bucket_centre(x,y,c);
                        std::size_t const  bucket = output_terminals_set().hash_function().bucket(x,y,c);
                        for (auto  it = output_terminals_set().begin(bucket), end = output_terminals_set().end(bucket); it != end; ++it)
                            if (it->second != &oterm)
                            {
                                vector3  u = oterm.pos() - it->first;
                                scalar const u_lenght = length(u);
                                scalar const  A = 0.01f;
                                scalar const  D = 0.25f * length(interspot_distance());
                                scalar const  e = 0.0001f;
                                scalar const  a = std::log(e / A) / D;
                                scalar const  m = 1.0f / (1.0f + (length(C - it->first) / D));
                                vector3  grad_f = (A * m * std::exp(a*u_lenght)) * u;
                                if (u_lenght > 1e-6f)
                                    grad_f /= u_lenght;
                                gradient += grad_f;
                            }
                    }
        }

        if (true)
        {
            TMPROF_BLOCK();

            natural_64_bit  xlo, ylo, clo;
            input_spots().hash_function().bucket_indices(oterm.pos() - interspot_distance(), xlo, ylo, clo);
            natural_64_bit  xhi, yhi, chi;
            input_spots().hash_function().bucket_indices(oterm.pos() + interspot_distance(), xhi, yhi, chi);
            for (natural_64_bit x = xlo; x <= xhi; ++x)
                for (natural_64_bit y = ylo; y <= yhi; ++y)
                    for (natural_64_bit c = clo; c <= chi; ++c)
                    {
                        vector3 const  C = input_spots().hash_function().bucket_centre(x, y, c);
                        std::size_t const  bucket = input_spots().hash_function().bucket(x,y,c);
                        for (auto it = input_spots().begin(bucket), end = input_spots().end(bucket); it != end; ++it)
                        {
                            vector3  u =  it->first - oterm.pos();
                            scalar const u_lenght = length(u);
                            scalar const  A = 0.001f;
                            scalar const  D = 0.5f * length(interspot_distance());
                            scalar const  e = 0.00001f;
                            scalar const  a = std::log(e/A)/D;
                            scalar const  m = 1.0f / (1.0f + (length(C - it->first) / D));
                            vector3  grad_f = (A * m * std::exp(a*u_lenght)) * u;
                            if (u_lenght > 1e-6f)
                                grad_f /= u_lenght;
                            gradient += grad_f;
                        }
                    }
        }

        {
            TMPROF_BLOCK();

            // We trait the 'gradient' here as an acceleration for an output terminal

            scalar const  dt = 1000.0f * (scalar)get_params()->update_time_step_in_seconds();
            //vector3 const  new_velocity = 1.0f* dt * gradient;
            //vector3 const  new_velocity = oterm.velocity() + dt * gradient;//update_magnitude_of_velocity(oterm.velocity() + dt * gradient);
            vector3 const  new_velocity = update_magnitude_of_velocity(oterm.velocity() + dt * gradient,
                                                                       get_params()->output_terminal_velocity_min_magnitude(),
                                                                       get_params());
            vector3 const  new_pos = oterm.pos() + dt * new_velocity;
            m_output_terminals_set.erase({oterm.pos(),&oterm});
            oterm.set_pos(new_pos);
            oterm.set_velocity(new_velocity);
            auto const result = m_output_terminals_set.insert({new_pos,&oterm});
            INVARIANT(result.second);
        }
    }
}
