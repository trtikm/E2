add_library(pycellab SHARED
    ./pycellab.cpp
    ./accessors.cpp
    ./data_classes.cpp
    ./accessors.hpp
    ./data_classes.hpp
    )
set_target_properties(pycellab PROPERTIES PREFIX "")
target_link_libraries(pycellab
    cellab
    utility
    ${Boost_LIBRARIES}
    ${PYTHON_LIBRARIES}
    )
install(TARGETS pycellab DESTINATION "./")
install(FILES ./pycellab.py DESTINATION "./")
