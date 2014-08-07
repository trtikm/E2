add_library(utility
    ./include/utility/assumptions.hpp

    ./include/utility/basic_numeric_types.hpp

    ./src/bitfield.cpp
    ./include/utility/bitfield.hpp

    ./src/bits_reference.cpp
    ./include/utility/bits_reference.hpp

    ./include/utility/config.hpp

    ./include/utility/development.hpp

    ./src/endian.cpp
    ./include/utility/endian.hpp

    ./src/fail_message.cpp
    ./include/utility/fail_message.hpp

    ./include/utility/invariants.hpp

    ./src/log.cpp
    ./include/utility/log.hpp

    ./src/timeprof.cpp
    ./include/utility/timeprof.hpp

    ./src/checked_number_operations.cpp
    ./include/utility/checked_number_operations.hpp
    )

#install(TARGETS brod_utils DESTINATION lib/utils)

