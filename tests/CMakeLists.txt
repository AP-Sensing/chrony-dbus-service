cmake_minimum_required(VERSION 3.27)

include(google_test)

# Define the GTEST macro in order to be able to invoke test specific functions
add_compile_definitions(GTEST)

find_package(GTest REQUIRED)

function(add_chrony_dbus_service_test_executable)
    cmake_parse_arguments(TEST "" "NAME" "SRCS;LIBS" ${ARGN})

    add_executable(${TEST_NAME})
    target_sources(${TEST_NAME} PRIVATE ${TEST_SRCS})
    target_link_libraries(${TEST_NAME} PRIVATE ${TEST_LIBS}
                                               GTest::gtest_main
                                               GTest::gtest
                                               GTest::gmock
                                               simppl)
    gtest_discover_tests(${TEST_NAME}
                         TEST_PREFIX "aps_chronydbusservice_"
                         XML_OUTPUT_DIR "${PROJECT_BINARY_DIR}/results")

    # Group under the "tests" project folder in IDEs such as Visual Studio
    set_property(TARGET ${TEST_NAME} PROPERTY FOLDER "tests")

    message(STATUS "Added APS chronydbusservice test executable: ${TEST_NAME}.")
endfunction()

##########################################
# Tests
##########################################

add_chrony_dbus_service_test_executable(NAME "chronydbusservice_test"
                                 SRCS "${PROJECT_SOURCE_DIR}/src/tests/testmain.cpp"
                                      "${PROJECT_SOURCE_DIR}/src/tests/chronydbustest.h"
                                      "${PROJECT_SOURCE_DIR}/src/tests/chronydbustest.cpp"
                                 LIBS simppl
                                      ${DBUS_LIBRARIES})

