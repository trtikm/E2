#ifndef UTILITY_TEST_HPP_INCLUDED
#   define UTILITY_TETS_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <utility/log.hpp>
#   include <iostream>

#   define TEST_EVALUATION(COND,RESULT,TOLOG,TOCOUT) \
    {\
        try\
        {\
            if ((COND) == (RESULT))\
            {\
                ++test_statistics::num_tests_which_succeeded_without_exception();\
                if (TOLOG) { LOG(info,"TEST SUCCEEDED: " #COND); }\
                if (TOCOUT) { std::cout << "TEST SUCCEEDED: " #COND << "\n"; }\
            }\
            else\
            {\
                ++test_statistics::num_tests_which_failed_without_exception();\
                if (TOLOG) { LOG(error,"TEST FAILED: " #COND); }\
                if (TOCOUT) { std::cout << "TEST FAILED: " #COND << "\n"; }\
            }\
        }\
        catch (...)\
        {\
            if (RESULT)\
            {\
                ++test_statistics::num_tests_which_failed_by_exception();\
                if (TOLOG) { LOG(error,"TEST FAILED (by throwing exception): " #COND); }\
                if (TOCOUT) { std::cout << "TEST FAILED (by throwing exception): " #COND << "\n"; }\
            }\
            else\
            {\
                ++test_statistics::num_tests_which_succeeded_by_exception();\
                if (TOLOG) { LOG(info,"TEST SUCCEEDED (by throwing exception): " #COND); }\
                if (TOCOUT) { std::cout << "TEST SUCCEEDED (by throwing exception): " #COND << "\n"; }\
            }\
        }\
    }

#   define TEST_SUCCESS(COND) TEST_EVALUATION(COND,true,true,true)
#   define TEST_FAILURE(COND) TEST_EVALUATION(COND,false,true,true)

#   define TEST_SUCCESS_EX(COND,TOLOG,TOCOUT) TEST_EVALUATION(COND,true,TOLOG,TOCOUT)
#   define TEST_FAILURE_EX(COND,TOLOG,TOCOUT) TEST_EVALUATION(COND,false,TOLOG,TOCOUT)

#   define TEST_PRINT_STATISTICS() test_statistics::print_test_statistical_data_to_log_and_standard_output()

namespace test_statistics {

natural_32_bit&  num_tests_which_succeeded_without_exception();
natural_32_bit&  num_tests_which_succeeded_by_exception();
natural_32_bit&  num_tests_which_failed_without_exception();
natural_32_bit&  num_tests_which_failed_by_exception();

void print_test_statistical_data_to_log_and_standard_output();

}


#endif
