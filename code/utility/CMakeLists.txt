add_library(utility
    ./src/bitfield.cpp
    ./src/endian.cpp
    ./src/fail_message.cpp
    ./src/timeprof.cpp
    ./src/log.cpp
    ./src/bits_reference.cpp
    ./include/utility/bits_reference.hpp
    ./include/utility/log.hpp
    ./include/utility/assumptions.hpp
    ./include/utility/development.hpp
    ./include/utility/invariants.hpp
    ./include/utility/bitfield.hpp
    ./include/utility/endian.hpp
    ./include/utility/fail_message.hpp
    ./include/utility/timeprof.hpp
    ./include/utility/config.hpp
    )

#install(TARGETS brod_utils DESTINATION lib/utils)

