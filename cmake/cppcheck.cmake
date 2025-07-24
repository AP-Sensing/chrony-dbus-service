cmake_minimum_required(VERSION 3.22)

if(APS_CHRONY_DBUS_SERVICE_BUILD_TESTS)
    message(FATAL_ERROR "Cppcheck is incompatible with building tests. Make sure to disable APS_CHRONY_DBUS_SERVICE_ENABLE_CPPCHECK or disable tests by setting APS_CHRONY_DBUS_SERVICE_BUILD_TESTS to OFF. This is because Cppcheck would try to check the google tests source code and then fail. ")
endif()

find_program(CMAKE_CXX_CPPCHECK NAMES cppcheck)
if(CMAKE_CXX_CPPCHECK STREQUAL "CMAKE_CXX_CPPCHECK-NOTFOUND")
    message(FATAL_ERROR "cppcheck not found but it is required when the APS_GRPC_ENABLE_CPPCHECK option is enabled.")
endif()

list(APPEND CMAKE_CXX_CPPCHECK "--xml"
                               "--error-exitcode=1"
                               "--enable=warning,style"
                               "--force" 
                               "--inline-suppr"
                               "--addon=y2038"
                               "--std=c++${CMAKE_CXX_STANDARD}"
                               "--cppcheck-build-dir=${PROJECT_BINARY_DIR}"
                               "--suppress-xml=${PROJECT_SOURCE_DIR}/cppcheck-suppressions.xml"
                               "--output-file=${PROJECT_BINARY_DIR}/cppcheck.xml")

if(APS_CHRONY_DBUS_SERVICE_ENABLE_CPPCHECK_EXHAUSTIVE)
    list(APPEND CMAKE_CXX_CPPCHECK "--check-level=exhaustive")
    message(STATUS "Cppcheck in exhaustive mode enabled.")
else()
    list(APPEND CMAKE_CXX_CPPCHECK "--check-level=normal")
    message(STATUS "Cppcheck in normal enabled.")
endif()
