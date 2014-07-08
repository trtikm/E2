#include "./accessors.hpp"
#include <boost/python.hpp>

using namespace boost::python;

BOOST_PYTHON_MODULE(pycellab)
{
    class_<pytest::data_holder>("data_holder", init<unsigned int>())
        .def("get_data", &pytest::data_holder::get_data)
        .def("set_data", &pytest::data_holder::set_data)
    ;
}


//char const* greet()
//{
//   return "hello, marek's world";
//}

//#include <boost/python.hpp>

//BOOST_PYTHON_MODULE(pytest)
//{
//    using namespace boost::python;
//    def("greet", greet);
//}
