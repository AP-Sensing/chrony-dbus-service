cmake_minimum_required(VERSION 3.22)

if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    find_program(CLANG_TIDY_PATH NAMES clang-tidy)
    if(CLANG_TIDY_PATH STREQUAL "CLANG_TIDY_PATH-NOTFOUND")
        message(FATAL_ERROR "clang-tidy not found but it is required when the APS_CHRONY_DBUS_SERVICE_ENABLE_CLANG_TIDY option is enabled.")
    endif()
    
    message(STATUS "Enabling clang-tidy")
    # Do not add "-p" "${CMAKE_BINARY_DIR}/compile_commands.json" since then it would all the time check all files over and over instead of just the current target.
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_PATH}" "--config-file=${CMAKE_SOURCE_DIR}/.clang-tidy")
    add_compile_definitions(CLANG_TIDY)
else()
    message(FATAL_ERROR "clang-tidy is not supported when building for windows")
endif()
