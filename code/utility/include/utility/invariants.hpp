#ifndef UTILITY_INVARIANTS_HPP_INCLUDED
#   define UTILITY_INVARIANTS_HPP_INCLUDED

#   include <utility/config.hpp>

#   if !((BUILD_DEBUG() == 1 && defined(DEBUG_DISABLE_INVARIANT_CHECKING)) || \
         (BUILD_RELEASE() == 1 && defined(RELEASE_DISABLE_INVARIANT_CHECKING)))
#       include <utility/fail_message.hpp>
#       include <utility/log.hpp>
#       include <stdexcept>
#       include <string>
        struct invariant_failure : public std::logic_error {
            explicit invariant_failure(std::string const& msg) : std::logic_error(msg) {}
        };
#       define INVARIANT(C) { if (!(C)) { LOG(error,"Invariant failure.");\
                                          throw invariant_failure(FAIL_MSG("Invariant failure.")); } }
#       define UNREACHABLE() { LOG(error,"Unreachable location reached.");\
                               throw invariant_failure(FAIL_MSG("Unreachable location reached.")); }
#   else
#       define INVARIANT(C)
#       define UNREACHABLE()
#   endif

#endif
