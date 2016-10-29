#include "./program_info.hpp"
#include "./program_options.hpp"
#include <angeo/tensor_math.hpp>
#include <utility/invariants.hpp>
#include <utility/test.hpp>
#include <utility/timeprof.hpp>


void test_basic_vector_operations()
{
    vector3 u(1.0, -1.0, 2.0);
    TEST_SUCCESS(u(0)==1.0 && u(1)==-1.0 && u(2)==2.0);

    vector3 v(-1.0, -3.0, 4.0);
    TEST_SUCCESS(v(0)==-1.0 && v(1)==-3.0 && v(2)==4.0);

    vector3 w = 2 * u - v;
    TEST_SUCCESS(w(0)==3.0 && w(1)==1.0 && w(2)==0.0);

    w = u.cross(v);
    TEST_SUCCESS(w(0)==2.0 && w(1)==-6.0 && w(2)==-4.0);

    scalar a = u.dot(v);
    TEST_SUCCESS(a==10.0);

    TEST_PROGRESS_UPDATE();
}


void run()
{
    TMPROF_BLOCK();

    TEST_PROGRESS_SHOW();

    test_basic_vector_operations();

    TEST_PROGRESS_HIDE();

    TEST_PRINT_STATISTICS();
}
