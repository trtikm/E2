#ifndef UTILITY_TEST_HPP_INCLUDED
#   define UTILITY_TEST_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <utility/log.hpp>
#   include <iostream>

#   define TEST_EVALUATION(COND,RESULT,LOGSUCCESS) \
    {\
        logging_severity_level const  original_severity_level = get_minimal_severity_level();\
        try\
        {\
            if (!(RESULT))\
                set_minimal_severity_level(logging_severity_level::testing);\
            if ((COND) == (RESULT))\
            {\
                ++test_statistics::num_tests_which_succeeded_without_exception();\
                if (LOGSUCCESS) { LOG(testing,"TEST SUCCEEDED: " #COND); }\
                if (LOGSUCCESS) { std::cout << "TEST SUCCEEDED: " #COND << "\n"; }\
            }\
            else\
            {\
                ++test_statistics::num_tests_which_failed_without_exception();\
                if (true) { LOG(testing,"TEST FAILED: " #COND); }\
                if (true) { std::cout << "TEST FAILED: " #COND << "\n"; }\
                if (true) { ::private_test_internal_implementation_details::on_test_fail(); }\
            }\
            set_minimal_severity_level(original_severity_level);\
        }\
        catch (...)\
        {\
            set_minimal_severity_level(original_severity_level);\
            if (RESULT)\
            {\
                ++test_statistics::num_tests_which_failed_by_exception();\
                if (true) { LOG(testing,"TEST FAILED (by throwing exception): " #COND); }\
                if (true) { std::cout << "TEST FAILED (by throwing exception): " #COND << "\n"; }\
                if (true) { ::private_test_internal_implementation_details::on_test_fail(); }\
            }\
            else\
            {\
                ++test_statistics::num_tests_which_succeeded_by_exception();\
                if (LOGSUCCESS) { LOG(testing,"TEST SUCCEEDED (by throwing exception): " #COND); }\
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

#   define TEST_LOG(LVL,MSG) \
            {\
                logging_severity_level const  old_level = get_minimal_severity_level();\
                set_minimal_severity_level(LVL);\
                LOG(LVL,MSG);\
                set_minimal_severity_level(old_level);\
            }


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
void  on_test_fail();
}

#endif
