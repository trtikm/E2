#include "./program_info.hpp"
#include "./program_options.hpp"
#include <utility/basic_numeric_types.hpp>
#include <utility/instance_wrapper.hpp>
#include <utility/test.hpp>
#include <utility/timeprof.hpp>
#include <algorithm>
#include <functional>
#include <vector>
#include <string>
#include <sstream>


typedef std::function<natural_32_bit()> function_generate_id;
typedef std::function<void(natural_32_bit const,std::string const&)> function_push_message;
typedef std::pair<function_generate_id,function_push_message> pair_of_functions;


struct my_object
{
    my_object(pair_of_functions const& functions)
        : m_functions(functions)
        , m_ID(m_functions.first())
    {
        m_functions.second(m_ID,FUNCTION_PROTOTYPE());
    }

    my_object(my_object const& other)
        : m_functions(other.m_functions)
        , m_ID(m_functions.first())
    {
        m_functions.second(m_ID,FUNCTION_PROTOTYPE());
        log_id_message(other.m_ID, " was the source in my copy constructor.");
    }

    my_object& operator=(my_object const& other)
    {
        // do not change the state, only do the logging!
        m_functions.second(m_ID,FUNCTION_PROTOTYPE());
        log_id_message(other.m_ID, " was the source in my 'operator='.");
        return *this;
    }

    ~my_object()
    {
        m_functions.second(m_ID,FUNCTION_PROTOTYPE());
    }

    void log_message(std::string const& msg) const
    {
        std::stringstream sstr;
        sstr << FUNCTION_PROTOTYPE() << " ==> " << msg;
        m_functions.second(m_ID,sstr.str());
    }

    void log_id_message(natural_32_bit otherID, std::string const& msg) const
    {
        std::stringstream sstr;
        sstr << FUNCTION_PROTOTYPE() << " ==> " << otherID << msg;
        m_functions.second(m_ID,sstr.str());
    }

    bool foo() const
    {
        m_functions.second(m_ID,FUNCTION_PROTOTYPE());
        return true;
    }

private:
    pair_of_functions m_functions;
    natural_32_bit m_ID;
};

static natural_32_bit generate_id(natural_32_bit& counter)
{
    return ++counter;
}

static void push_message(std::vector<std::string>& store, natural_32_bit const ID, std::string const& msg)
{
    std::stringstream sstr;
    sstr << ID << "@" << msg;
    store.push_back(sstr.str());
}


static void test_copy_construction_and_copy_assignment(instance_wrapper<my_object> wobj, my_object obj)
{
    instance_wrapper<my_object> wother = wobj;
    my_object other = obj;

    wobj = wother;
    obj = other;

    wobj = other;
    obj = wother;

    wobj->log_message("copy construction and copy asignment tested.");
    obj.log_message("copy construction and copy asignment tested.");
}

static void do_the_test()
{
    natural_32_bit wcounter = 0U;
    natural_32_bit counter = 0U;
    std::vector<std::string> whistory;
    std::vector<std::string> history;
    {
        instance_wrapper<my_object> wobj;
        TEST_FAILURE(wobj->foo());
        TEST_FAILURE(wobj.reference_to_instance().foo());
        wobj.construct_instance(
                pair_of_functions(
                    std::bind(&generate_id,std::ref(wcounter)),
                    std::bind(&push_message,std::ref(whistory),std::placeholders::_1,std::placeholders::_2)
                    )
                );
        TEST_SUCCESS(wobj->foo());
        TEST_SUCCESS(wobj.reference_to_instance().foo());
        wobj->log_message("initial logged message!");

        my_object obj(
                pair_of_functions(
                    std::bind(&generate_id,std::ref(counter)),
                    std::bind(&push_message,std::ref(history),std::placeholders::_1,std::placeholders::_2)
                    )
                );
        obj.foo();
        obj.foo();
        obj.log_message("initial logged message!");

        test_copy_construction_and_copy_assignment(wobj,obj);
    }
    TEST_SUCCESS(wcounter == counter);
    TEST_SUCCESS(whistory.size() == history.size());
    for (natural_32_bit i = 0; i < std::min(whistory.size(), history.size()); ++i)
        TEST_SUCCESS(whistory.at(i) == history.at(i));
}

void run()
{
    TMPROF_BLOCK();

    TEST_PROGRESS_SHOW();
    do_the_test();
    TEST_PROGRESS_HIDE();

    TEST_PRINT_STATISTICS();
}
