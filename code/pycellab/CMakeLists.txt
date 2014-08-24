set(THIS_TARGET_NAME pycellab)

add_library(${THIS_TARGET_NAME} SHARED
    ./pycellab.cpp
    ./pycellabutils.py
    ./pycellab-test-001-construction-of-static-state-of-neural-tissue.py
    )

#set_target_properties(${THIS_TARGET_NAME} PROPERTIES PREFIX "")

target_link_libraries(${THIS_TARGET_NAME}
    cellab
    utility
    ${BOOST_LIST_OF_LIBRARIES_TO_LINK_WITH}
    ${PYTHON_LIST_OF_LIBRARIES_TO_LINK_WITH}
    )

#set_target_properties(${THIS_TARGET_NAME} PROPERTIES
#    OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_${CMAKE_BUILD_TYPE}"
#    )
install(TARGETS ${THIS_TARGET_NAME} DESTINATION "./")

#install(TARGETS pycellab DESTINATION "./")

install(
    FILES
    ./pycellabutils.py
    ./pycellab-test-001-construction-of-static-state-of-neural-tissue.py
    DESTINATION
    "./"
    )

