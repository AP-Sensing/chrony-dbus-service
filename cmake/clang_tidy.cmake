cmake_minimum_required(VERSION 3.27)

if(WIN32)
    message(FATAL_ERROR "clang-tidy is not supported when building for Windows")
endif()

if(APS_CHRONY_DBUS_SERVICE_ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy)
    if(CLANG_TIDY_EXECUTABLE STREQUAL "CLANG_TIDY_EXECUTABLE-NOTFOUND")
        message(FATAL_ERROR "Clang-Tidy not found but it is required.")
    endif()

    if(APS_CHRONY_DBUS_SERVICE_ENABLE_CLANG_TIDY_FIX)
        set(CLANG_TIDY_FIX_COMMANDS ";--fix;--fix-errors;--fix-notes")
        message(STATUS "Clang-Tidy enabled with automatic error fixing")
    else()
        set(CLANG_TIDY_FIX_COMMANDS "")
        message(STATUS "Clang-Tidy enabled without automatic error fixing")
    endif()

    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXECUTABLE};-warnings-as-errors=*${CLANG_TIDY_FIX_COMMANDS}")
endif()

# Temporarily disable clang-tidy linting for all target declaration that follow this macro until you call
# 'enable_clang_tidy()' again.
macro(disable_clang_tidy)
    clear_variable(DESTINATION CMAKE_CXX_CLANG_TIDY BACKUP CMAKE_CXX_CLANG_TIDY_BKP)
    clear_variable(DESTINATION CMAKE_C_CLANG_TIDY BACKUP CMAKE_C_CLANG_TIDY_BKP)
endmacro()

# Enables clang-tidy for all  for all target declaration that follow this macro again. Pendant to
# 'disable_clang_tidy()'.
macro(enable_clang_tidy)
    restore_variable(DESTINATION CMAKE_CXX_CLANG_TIDY BACKUP CMAKE_CXX_CLANG_TIDY_BKP)
    restore_variable(DESTINATION CMAKE_C_CLANG_TIDY BACKUP CMAKE_C_CLANG_TIDY_BKP)
endmacro()