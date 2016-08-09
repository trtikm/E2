#include "./nenet.hpp"
#include <utility/random.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
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
    INVARIANT(cmap.bucket_count() >= (natural_64_bit)num_elems_x * (natural_64_bit)num_elems_y * (natural_64_bit)num_elems_c);
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

void  init_output_terminals(output_terminal::pos_map&  omap, cell::pos_map&  cmap, natural_16_bit const  num_terminals_per_cell)
{
    //vector3 const  origin = omap.hash_function().origin() - 0.5f * omap.hash_function().inter_distance();
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
            while (true)
            {
                scalar const  tx = get_random_natural_32_bit_in_range(0U, 999U) / 1000.0f;
                scalar const  ty = get_random_natural_32_bit_in_range(0U, 999U) / 1000.0f;
                scalar const  tc = get_random_natural_32_bit_in_range(0U, 999U) / 1000.0f;
                vector3 const  u = lo + (vector3(tx,ty,tc).array() * (hi - lo).array()).matrix();
                auto const  result = omap.insert({ u,{} });
                if (result.second)
                {
                    result.first->second.set_cell(it);
                    it->second.add_output_terminal(result.first);
                    break;
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
    cell::pos_map&  cmap, input_spot::pos_map&  imap,
    scalar const percentage_of_territories_overlap)
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
typename cell::pos_map_template<T>::const_iterator
find_closest_element(cell::pos_map_template<T> const&  map,
                     vector3 const&  origin, vector3 const&  ray, scalar const  radius,
                     scalar* const  param)
{
    scalar const ray_dot = dot_product(ray, ray);
    ASSUMPTION(ray_dot > 1e-3f);
    scalar  result_t = (param != nullptr) ? *param : 1e30f;
    typename cell::pos_map_template<T>::const_iterator  result = map.cend();
    for (typename cell::pos_map_template<T>::const_iterator it = map.cbegin(); it != map.cend(); ++it)
    {
        scalar const  t = dot_product(ray, it->first - origin) / ray_dot;
        if (t >= 1.0f)
        {
            vector3 const  X = origin + t * ray;
            vector3 const  u = it->first - X;
            scalar const  dist2 = dot_product(u, u);
            if (dist2 <= radius * radius && (result == map.cend() || result_t > t))
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
    vector3 const  u = (pos - (m_origin - 0.5f * m_intercell_distance)).array() / m_intercell_distance.array();
    natural_64_bit  x = (u(0) <= 0.0) ? 0ULL : (natural_64_bit)u(0);
    natural_64_bit  y = (u(1) <= 0.0) ? 0ULL : (natural_64_bit)u(1);
    natural_64_bit  c = (u(2) <= 0.0) ? 0ULL : (natural_64_bit)u(2);
    ASSUMPTION(x < m_num_cells_x && y < m_num_cells_y && c < m_num_cells_c);
    return c * (m_num_cells_x * m_num_cells_y) + y * m_num_cells_x + x;
}

cell::pos_equal::pos_equal(vector3 const&  max_distance)
    : m_max_distance(max_distance)
{
    ASSUMPTION(m_max_distance(0) > 0.0f && m_max_distance(1) > 0.0f && m_max_distance(2) > 0.0f);
}

bool  cell::pos_equal::operator()(vector3 const&  l, vector3 const&  r) const
{
    vector3 const  u = l - r;
    return std::abs(u(0)) <= m_max_distance(0) && std::abs(u(1)) <= m_max_distance(1) && std::abs(u(2)) <= m_max_distance(2);
}


cell::cell()
    : m_input_spots()
    , m_output_terminals()
    , m_output_area_center(0.0f,0.0f,0.0f)
//    , m_output_area_radius(1.0f)
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

output_terminal::output_terminal()
    : m_cell()
{}


nenet::nenet(
    vector3 const&  lo_corner, vector3 const&  hi_corner,
    natural_8_bit const  num_cells_x, natural_8_bit const  num_cells_y, natural_8_bit const  num_cells_c,
    natural_16_bit const  max_num_inputs_to_cell,
    scalar const percentage_of_territories_overlap
    )
    : m_lo_corner(lo_corner)
    , m_hi_corner(hi_corner)

    , m_num_cells_x(num_cells_x)
    , m_num_cells_y(num_cells_y)
    , m_num_cells_c(num_cells_c)

    , m_max_num_inputs_to_cell(max_num_inputs_to_cell)

    , m_percentage_of_territories_overlap(percentage_of_territories_overlap)

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

    , m_output_terminals(m_input_spots.bucket_count(),m_input_spots.hash_function(),m_input_spots.key_eq())
{
    TMPROF_BLOCK();

    ASSUMPTION(m_lo_corner(0) < m_hi_corner(0) && m_lo_corner(1) < m_hi_corner(1) && m_lo_corner(2) < m_hi_corner(2));
    ASSUMPTION(m_num_cells_x != 0 && m_num_cells_y != 0 && m_num_cells_c != 0);
    ASSUMPTION(m_max_num_inputs_to_cell != 0);
    ASSUMPTION(m_percentage_of_territories_overlap >= 0.0);
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

    interconnect_cells_with_input_spots(m_cells, m_input_spots, m_percentage_of_territories_overlap);

    init_output_areas(m_cells);
    init_output_terminals(m_output_terminals, m_cells, m_max_num_inputs_to_cell);

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

output_terminal::pos_map::const_iterator  nenet::find_closest_output_terminal(vector3 const&  origin, vector3 const&  ray, scalar const  radius,
                                                                              scalar* const  param) const
{
    return find_closest_element(output_terminals(), origin, ray, radius, param);
}

void  nenet::update()
{
    TMPROF_BLOCK();
}
