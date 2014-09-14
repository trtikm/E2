add_library(utility
    ./include/utility/assumptions.hpp

    ./include/utility/basic_numeric_types.hpp

    ./src/array_of_bit_units.cpp
    ./include/utility/array_of_bit_units.hpp

    ./src/bits_reference.cpp
    ./include/utility/bits_reference.hpp

    ./include/utility/array_of_references_to_bits.hpp

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

    ./src/random.cpp
    ./include/utility/random.hpp

    ./include/utility/instance_wrapper.hpp

    )

#install(TARGETS brod_utils DESTINATION lib/utils)

