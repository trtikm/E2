message("Including the following E2 tests to the build:")


message("-- run_tests.py")
install(
    FILES
    "./run_tests.py"
    DESTINATION
    "tests"
    )
