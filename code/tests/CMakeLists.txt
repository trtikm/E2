message("Including the following E2 tests to the build:")

add_subdirectory(./unsigned_integer_wrap_operations)
    message("-- unsigned_integer_wrap_operations")

add_subdirectory(./tmprof_thread_safety)
    message("-- tmprof_thread_safety")

add_subdirectory(./bits_reference_operations)
    message("-- bits_reference_operations")

add_subdirectory(./instance_wrapper)
    message("-- instance_wrapper")

add_subdirectory(./array_of_bit_units)
    message("-- array_of_bit_units")

add_subdirectory(./homogenous_slice_of_tissue)
    message("-- homogenous_slice_of_tissue")

add_subdirectory(./tissue_allocation_and_memory_accesses)
    message("-- tissue_allocation_and_memory_accesses")

add_subdirectory(./synchronisation_of_tissue_algorithms)
    message("-- synchronisation_of_tissue_algorithms")

add_subdirectory(./neural_tissue_construction_and_simulation)
    message("-- neural_tissue_construction_and_simulation")

add_subdirectory(./efloop_construction_and_simulation)
    message("-- efloop_construction_and_simulation")

add_subdirectory(./random)
    message("-- random")

add_subdirectory(./tensor_math)
    message("-- tensor_math")


message("-- run_tests.py")
install(
    FILES
    "./run_tests.py"
    DESTINATION
    "./tests"
    )
