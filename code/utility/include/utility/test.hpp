#ifndef UTILITY_TEST_HPP_INCLUDED
#   define UTILITY_TETS_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <utility/log.hpp>
#   include <iostream>

#   define TEST_EVALUATION(COND,RESULT,LOGSUCCESS) \
    {\
        try\
        {\
            if ((COND) == (RESULT))\
            {\
                ++test_statistics::num_tests_which_succeeded_without_exception();\
                if (LOGSUCCESS) { LOG(info,"TEST SUCCEEDED: " #COND); }\
                if (LOGSUCCESS) { std::cout << "TEST SUCCEEDED: " #COND << "\n"; }\
            }\
            else\
            {\
                ++test_statistics::num_tests_which_failed_without_exception();\
                if (true) { LOG(error,"TEST FAILED: " #COND); }\
                if (true) { std::cout << "TEST FAILED: " #COND << "\n"; }\
            }\
        }\
        catch (...)\
        {\
            if (RESULT)\
            {\
                ++test_statistics::num_tests_which_failed_by_exception();\
                if (true) { LOG(error,"TEST FAILED (by throwing exception): " #COND); }\
                if (true) { std::cout << "TEST FAILED (by throwing exception): " #COND << "\n"; }\
            }\
            else\
            {\
                ++test_statistics::num_tests_which_succeeded_by_exception();\
                if (LOGSUCCESS) { LOG(info,"TEST SUCCEEDED (by throwing exception): " #COND); }\
                if (LOGSUCCESS) { std::cout << "TEST SUCCEEDED (by throwing exception): " #COND << "\n"; }\
            }\
        }\
    }

#   define TEST_SUCCESS(COND) TEST_EVALUATION(COND,true,false)
#   define TEST_FAILURE(COND) TEST_EVALUATION(COND,false,false)

#   define TEST_SUCCESS_EX(COND,LOGSUCCESS) TEST_EVALUATION(COND,true,LOGSUCCESS)
#   define TEST_FAILURE_EX(COND,LOGSUCCESS) TEST_EVALUATION(COND,false,LOGSUCCESS)

#   define TEST_PRINT_STATISTICS() test_statistics::print_test_statistical_data_to_log_and_standard_output()

#   define TEST_PROGRESS_SHOW() ::private_test_internal_implementation_details::print_next_test_progress_character()
#   define TEST_PROGRESS_HIDE() ::private_test_internal_implementation_details::hide_test_progress_character()
#   define TEST_PROGRESS_UPDATE() { TEST_PROGRESS_HIDE(); TEST_PROGRESS_SHOW(); }


namespace test_statistics {

natural_32_bit&  num_tests_which_succeeded_without_exception();
natural_32_bit&  num_tests_which_succeeded_by_exception();
natural_32_bit&  num_tests_which_failed_without_exception();
natural_32_bit&  num_tests_which_failed_by_exception();

void print_test_statistical_data_to_log_and_standard_output();

}

namespace private_test_internal_implementation_details {
void  print_next_test_progress_character();
void  hide_test_progress_character();
}

#endif