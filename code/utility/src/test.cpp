#include <utility/test.hpp>
#include <array>
#include <algorithm>

namespace test_statistics {


natural_32_bit&  num_tests_which_succeeded_without_exception()
{
    static natural_32_bit  value = 0;
    return value;
}

natural_32_bit&  num_tests_which_succeeded_by_exception()
{
    static natural_32_bit  value = 0;
    return value;
}

natural_32_bit&  num_tests_which_failed_without_exception()
{
    static natural_32_bit  value = 0;
    return value;
}

natural_32_bit&  num_tests_which_failed_by_exception()
{
    static natural_32_bit  value = 0;
    return value;
}

void print_test_statistical_data_to_log_and_standard_output()
{
    natural_32_bit const  total_num_tests =
            num_tests_which_succeeded_without_exception() +
            num_tests_which_succeeded_by_exception() +
            num_tests_which_failed_without_exception() +
            num_tests_which_failed_by_exception()
            ;
    natural_32_bit const  total_num_succeeded_tests =
            num_tests_which_succeeded_without_exception() +
            num_tests_which_succeeded_by_exception()
            ;
    natural_32_bit const  total_num_failed_tests =
            num_tests_which_failed_without_exception() +
            num_tests_which_failed_by_exception()
            ;

    std::cout << "RESULTS FROM TESTING:\n";
    std::cout << "Total number of tests: " << total_num_tests << "\n";
    std::cout << "Total number of successfull tests: " << total_num_succeeded_tests << "\n";
    std::cout << "Total number of failed tests: " << total_num_failed_tests << "\n";

    if (total_num_failed_tests == 0)
    {
        LOG(info,"The testing was SUCCESSFUL. [Total/Succeeded/Failed = "
                  << total_num_tests << "/"
                  << total_num_succeeded_tests << "/"
                  << total_num_failed_tests
                  << "]");
        std::cout << "The testing was SUCCESSFUL.\n";
    }
    else
    {
        LOG(info,"The testing has FAILED. [Total/Succeeded/Failed = "
                  << total_num_tests << "/"
                  << total_num_succeeded_tests << "/"
                  << total_num_failed_tests
                  << "]");
        std::cout << "The testing has FAILED.\n";
    }
}


}


namespace private_test_internal_implementation_details {

void  print_next_test_progress_character()
{
    static std::array<natural_8_bit,4> char_set = { '|', '/', '-', '\\' };
    std::rotate(char_set.begin(),char_set.begin()+1,char_set.end());
    std::cout << char_set.at(0);
    std::cout.flush();
}

void  hide_test_progress_character()
{
    std::cout << '\b';
    std::cout.flush();
}

}
