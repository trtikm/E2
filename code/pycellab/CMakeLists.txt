add_library(pycellab SHARED
    ./pycellab.cpp
    ./pycellabutils.py
    ./pycellabtest.py
    )
set_target_properties(pycellab PROPERTIES PREFIX "")
target_link_libraries(pycellab
    cellab
    utility
    ${Boost_LIBRARIES}
    ${PYTHON_LIBRARIES}
    )
install(TARGETS pycellab DESTINATION "./")
install(
    FILES
    ./pycellabutils.py
    ./pycellabtest.py
    DESTINATION
    "./"
    )
