#include "./program_info.hpp"
#include "./program_options.hpp"
#include <utility/array_of_derived.hpp>
#include <utility/timeprof.hpp>
#include <utility/test.hpp>
#include <utility/log.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <type_traits>
#include <vector>
#include <iostream>


natural_64_bit  num_constructions_A = 0UL;
natural_64_bit  num_constructions_B = 0UL;
natural_64_bit  num_destructions_A = 0UL;
natural_64_bit  num_destructions_B = 0UL;


struct A
{
    A(int a) : i(a)
    {
        TEST_SUCCESS(num_constructions_A == num_constructions_B);
        std::cout << "  A::A() [no." << num_constructions_A << "]\n";
        ++num_constructions_A;
    }
    virtual ~A()
    {
        std::cout << "  A::~A() [no." << num_destructions_A << "]\n";
        ++num_destructions_A;
        TEST_SUCCESS(num_destructions_A == num_destructions_B);
    }
    int i;
};

struct B : public A
{
    B(int a, int aa) : A(a), ii(aa)
    {
        std::cout << "  B::B() [no." << num_constructions_B << "]\n";
        ++num_constructions_B;
        TEST_SUCCESS(num_constructions_A == num_constructions_B);
    }
    ~B()
    {
        TEST_SUCCESS(num_destructions_A == num_destructions_B);
        std::cout << "  B::~B() [no." << num_destructions_B << "]\n";
        ++num_destructions_B;
    }
    int ii;
};


static void clear_counters()
{
    num_constructions_A = 0UL;
    num_constructions_B = 0UL;
    num_destructions_A = 0UL;
    num_destructions_B = 0UL;
}

static void check_counters()
{
    TEST_SUCCESS(num_constructions_A == num_destructions_A);
    TEST_SUCCESS(num_constructions_B == num_destructions_B);
}

static void  do_test(
    array_of_derived<A>&  arr,
    natural_64_bit const  array_size,
    int const a_value,
    int const b_value,
    int const half_a_value,
    int const half_b_value,
    natural_64_bit const  num_constructions
    )
{
    natural_64_bit const  half = array_size / 2UL;

    TEST_SUCCESS(array_size == arr.size());
    TEST_SUCCESS(sizeof(B) == arr.element_size());
    TEST_SUCCESS(num_constructions_A == num_constructions);
    TEST_SUCCESS(num_constructions_B == num_constructions);

    dynamic_cast<B&>(arr.at(half)).i = half_a_value;
    dynamic_cast<B&>(arr.at(half)).ii = half_b_value;

    for (natural_64_bit i = 0UL; i < arr.size(); ++i)
    {
        if (i == half)
        {
            TEST_SUCCESS(arr.at(i).i == half_a_value);
            TEST_SUCCESS(dynamic_cast<B&>(arr.at(i)).i == half_a_value);
            TEST_SUCCESS(dynamic_cast<B&>(arr.at(i)).ii == half_b_value);
        }
        else
        {
            TEST_SUCCESS(arr.at(i).i == a_value);
            TEST_SUCCESS(dynamic_cast<B&>(arr.at(i)).i == a_value);
            TEST_SUCCESS(dynamic_cast<B&>(arr.at(i)).ii == b_value);
        }

        std::cout << "  dynamic_cast<B&>(arr.at(" << i << ")).i  = " << dynamic_cast<B&>(arr.at(i)).i << "\n";
        std::cout << "  dynamic_cast<B&>(arr.at(" << i << ")).ii = " << dynamic_cast<B&>(arr.at(i)).ii << "\n";
    }
}

void test_array_of_derived()
{
    TMPROF_BLOCK();

    natural_64_bit const  array_size = 10UL;
    int const a_value = 456;
    int const b_value = 654;
    int const half_a_value = 1111;
    int const half_b_value = 2222;

    std::cout << "*** TEST ***\n";
    clear_counters();
    {
        array_of_derived<A>  arr(array_size, type_envelope<B>(), a_value, b_value);
        do_test(arr, array_size, a_value, b_value, half_a_value, half_b_value,arr.size());
    }
    check_counters();
    TEST_PROGRESS_UPDATE();


    std::cout << "*** TEST ***\n";
    clear_counters();
    {
        std::unique_ptr< array_of_derived<A> >  arr = make_array_of_derived<A,B>(array_size, a_value, b_value);
        do_test(*arr, array_size, a_value, b_value, half_a_value, half_b_value, arr->size());
    }
    check_counters();
    TEST_PROGRESS_UPDATE();


    std::cout << "*** TEST ***\n";
    clear_counters();
    {
        struct local {
            static array_of_derived<A>  create(natural_64_bit const  array_size, int const a_value, int const b_value)
            {
                return array_of_derived<A>(array_size, type_envelope<B>(), a_value, b_value);
            }
        };
        array_of_derived<A>  arr = local::create(array_size, a_value, b_value);
        do_test(arr, array_size, a_value, b_value, half_a_value, half_b_value, arr.size());
    }
    check_counters();
    TEST_PROGRESS_UPDATE();


    std::cout << "*** TEST ***\n";
    clear_counters();
    {
        std::vector< std::unique_ptr< array_of_derived<A> > >   arrs;
        natural_64_bit const  vec_size = 3UL;
        natural_64_bit num_constructions = 0UL;
        for (int i = 0UL; i < vec_size; ++i, num_constructions += arrs.back()->size())
            arrs.push_back(make_array_of_derived<A, B>(array_size + i, a_value + i, b_value + i) );
        TEST_SUCCESS(arrs.size() == vec_size);
        for (int i = 0UL; i < arrs.size(); ++i)
            do_test(*arrs.at(i), array_size + i, a_value + i, b_value + i, half_a_value + i, half_b_value + i, num_constructions);
    }
    check_counters();
    TEST_PROGRESS_UPDATE();
}
