#ifndef ANGEO_UTILITY_HPP_INCLUDED
#   define ANGEO_UTILITY_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <utility/random.hpp>

namespace angeo {


template<typename  output_forward_iterator_type>
void  get_corner_points_of_bounding_box(
        vector3 const&  bbox_lo,
        vector3 const&  bbox_hi,
        output_forward_iterator_type  output_iterator
        )
{
    *output_iterator = bbox_lo;                                         ++output_iterator;
    *output_iterator = vector3{ bbox_hi(0), bbox_lo(1), bbox_lo(2) };   ++output_iterator;
    *output_iterator = vector3{ bbox_hi(0), bbox_hi(1), bbox_lo(2) };   ++output_iterator;
    *output_iterator = vector3{ bbox_lo(0), bbox_hi(1), bbox_lo(2) };   ++output_iterator;

    *output_iterator = vector3{ bbox_lo(0), bbox_lo(1), bbox_hi(2) };   ++output_iterator;
    *output_iterator = vector3{ bbox_hi(0), bbox_lo(1), bbox_hi(2) };   ++output_iterator;
    *output_iterator = bbox_hi;                                         ++output_iterator;
    *output_iterator = vector3{ bbox_lo(0), bbox_hi(1), bbox_hi(2) };   ++output_iterator;
}


void  get_random_vector_of_magnitude(
        float_32_bit const  magnitude,
        random_generator_for_natural_32_bit&   random_generator,
        vector3&  resulting_vector
        );


void  compute_tangent_space_of_unit_vector(
        vector3 const&  input_unit_vector,
        vector3&  output_unit_tangent,
        vector3&  output_unit_bitangent
        );


}

#endif
