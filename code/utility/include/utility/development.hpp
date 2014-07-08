#ifndef UTILITY_DEVELOPMENT_HPP_INCLUDED
#   define UTILITY_DEVELOPMENT_HPP_INCLUDED

#   include <utility/fail_message.hpp>
#   include <stdexcept>
#   include <string>
#   include <iostream>

#   define NOT_IMPLEMENTED_YET() { throw under_construction(FAIL_MSG("NOT IMPLEMENTED YET !")); }
#   define NOT_SUPPORTED() { throw under_construction(FAIL_MSG("NOT SUPPORTED !")); }
#   define TODO(MSG) { std::clog << "TODO: " << FAIL_MSG(MSG) << std::endl; }

struct under_construction : public std::logic_error {
    under_construction(std::string const& msg) : std::logic_error(msg) {}
};

#endif
