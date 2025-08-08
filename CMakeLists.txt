cmake_minimum_required(VERSION 3.27)

##########################################
# General Project Settings
##########################################

# ******************************************************************
# Only touch these macros, when creating a release or hotfix
# ******************************************************************
set(PROJECT_INTERNAL_VERSION_MAJOR 0)
set(PROJECT_INTERNAL_VERSION_MINOR 1)
set(PROJECT_INTERNAL_VERSION_PATCH 0)
set(PROJECT_NAME "chrony-dbus-service")
set(PROJECT_DESCRIPTION "The AP Sensing chronydbusservice code.")
# TODO fix crony -> chrony typo after git repo has been renamed
set(PROJECT_HOMEPAGE_URL "https://github.com/AP-Sensing/crony-dbus-service")

project("${PROJECT_NAME}" DESCRIPTION "${PROJECT_DESCRIPTION}"
                          HOMEPAGE_URL "${PROJECT_HOMEPAGE_URL}"
                          LANGUAGES CXX)

# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW)
endif()

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

##########################################
# Setup Version
##########################################

include(version_setup)
message(STATUS "${PROJECT_NAME} project version: ${${PROJECT_NAME}_VERSION_FULL_STRING}")

##########################################
# C++ Standard
##########################################

if(PARENT_CXX_STANDARD)
    # Don't set CMAKE_CXX_STANDARD if it is already set by parent project
    if (PARENT_CXX_STANDARD LESS 23)
        message(FATAL_ERROR "${PROJECT_NAME} does not support ${PARENT_CXX_STANDARD}.")
    endif()
else()
    # Set standard version if not already set by potential parent project
    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

##########################################
# Build Type
##########################################

if(NOT DEFINED CMAKE_BUILD_TYPE OR NOT CMAKE_BUILD_TYPE)
    if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
        set(CMAKE_BUILD_TYPE "Debug")
    else()
        set(CMAKE_BUILD_TYPE "Release")
    endif()
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

##########################################
# Project Options
##########################################

macro(aps_chronydbusservice_option OPTION_NAME OPTION_TEXT OPTION_DEFAULT)
    option(${OPTION_NAME} ${OPTION_TEXT} ${OPTION_DEFAULT})

    if(DEFINED ENV{${OPTION_NAME}})
        # Allow setting the option through an environment variable
        set(${OPTION_NAME} $ENV{${OPTION_NAME}})
    endif()

    if(${OPTION_NAME})
        add_definitions(-D${OPTION_NAME})
    endif()

    message(STATUS "  ${OPTION_NAME}: ${${OPTION_NAME}}")
endmacro()

macro(aps_chronydbusservice_option_string OPTION_NAME OPTION_TEXT OPTION_DEFAULT)
    set(${OPTION_NAME} ${OPTION_DEFAULT} CACHE STRING ${OPTION_TEXT})

    if(DEFINED ENV{${OPTION_NAME}})
        # Allow setting the option through an environment variable
        set(${OPTION_NAME} $ENV{${OPTION_NAME}})
    endif()

    if(${OPTION_NAME})
        add_definitions(-D${OPTION_NAME})
    endif()

    message(STATUS "  ${OPTION_NAME}: ${${OPTION_NAME}}")
endmacro()

macro(aps_template_log_level OPTION_NAME OPTION_TEXT OPTION_DEFAULT)
    set(${OPTION_NAME} ${OPTION_DEFAULT} CACHE STRING ${OPTION_TEXT})
    set_property(CACHE ${OPTION_NAME} PROPERTY STRINGS DEFAULT TRACE DEBUG INFO WARN ERROR FATAL OFF)

    if(DEFINED ENV{${OPTION_NAME}})
        # Allow setting the option through an environment variable
        set(${OPTION_NAME} $ENV{${OPTION_NAME}})
    endif()

    if(${OPTION_NAME})
        add_definitions(-D${OPTION_NAME})
    endif()

    message(STATUS "  ${OPTION_NAME}: ${${OPTION_NAME}}")
endmacro()

message(STATUS "=======================================================")
message(STATUS "Project Options:")
aps_chronydbusservice_option(APS_CHRONY_DBUS_SERVICE_STATIC_ANALYSIS "Set to ON to enable the GCC >= 10 static code analysis. If enabled, APS_CHRONY_DBUS_SERVICE_ENABLE_CLANG_TIDY and APS_CHRONY_DBUS_SERVICE_ENABLE_CPPCHECK have to be disabled." OFF)
aps_chronydbusservice_option(APS_CHRONY_DBUS_SERVICE_ENABLE_CLANG_TIDY "Set to ON to enable clang-tidy code linting. If enabled, APS_CHRONY_DBUS_SERVICE_ENABLE_CPPCHECK and APS_CHRONY_DBUS_SERVICE_STATIC_ANALYSIS have to be disabled." OFF)
aps_chronydbusservice_option(APS_CHRONY_DBUS_SERVICE_ENABLE_CPPCHECK "Set to ON to enable cppcheck static code analysis. If enabled, APS_CHRONY_DBUS_SERVICE_ENABLE_CLANG_TIDY and APS_CHRONY_DBUS_SERVICE_STATIC_ANALYZE have to be disabled." OFF)
aps_chronydbusservice_option(APS_CHRONY_DBUS_SERVICE_ENABLE_CPPCHECK_EXHAUSTIVE "Set to ON to enable cppcheck exhaustive check. This requires APS_CHRONY_DBUS_SERVICE_ENABLE_CPPCHECK to be enabled and will take a loooong time. Recommended for nightly builds." OFF)

aps_template_log_level(APS_CHRONY_DBUS_SERVICE_LOG_LEVEL "The maximum log level enabled. Everything that is more detailed then the one defined here will be optimized out and is therefore not included/available in the resulting binary. Setting it to 'DEFAULT' results in 'INFO' for all 'CMAKE_BUILD_TYPE' values that match the '.*release.*' case insensitive regex. Else it will be set to 'TRACE'." "DEFAULT")
aps_chronydbusservice_option(APS_CHRONY_DBUS_SERVICE_FETCH_CONTENT_USE_GITLAB_CI_TOKEN "Set to ON to enable cloning all repositories that are stored inside our GitLab instance with the use of the GitLab CI Token. If set to OFF, ssh will be used for cloning. Ref: https://docs.gitlab.com/ee/ci/jobs/ci_job_token.html" OFF)

aps_chronydbusservice_option_string(APS_CHRONY_DBUS_SERVICE_CPACK_RPM_TYPE "Define wether to build a RPM or SRPM package. Default: RPM" RPM)

aps_chronydbusservice_option(APS_CHRONY_DBUS_SERVICE_BUILD_TESTS "Set to ON to build tests." OFF)

aps_chronydbusservice_option(APS_CHRONY_DBUS_SERVICE_DEBUG_SANITIZER_FLAG_THREAD "Enables the ThreadSanitizer for debug builds." OFF)
aps_chronydbusservice_option(APS_CHRONY_DBUS_SERVICE_DEBUG_SANITIZER_FLAG_ADDR "Enables the AddressSanitizer for debug builds." OFF)
aps_chronydbusservice_option(APS_CHRONY_DBUS_SERVICE_DEBUG_SANITIZER_FLAG_LEAK "Enables the LeakSanitizer for debug builds." OFF)
aps_chronydbusservice_option(APS_CHRONY_DBUS_SERVICE_DEBUG_SANITIZER_FLAG_UB "Enables the UndefinedBehaviorSanitizer for debug builds." OFF)
aps_chronydbusservice_option(APS_CHRONY_DBUS_SERVICE_DEBUG_SANITIZER_FLAG_ALL "Enables all sanitizers for debug builds except the ThreadSanitizer since it is incompatible with the other sanitizers." OFF)
message(STATUS "=======================================================")

##########################################
# Sanitizer
##########################################

include(sanitizer)

##########################################
# Linting
##########################################

if(APS_CHRONY_DBUS_SERVICE_ENABLE_CLANG_TIDY)
    include(clang_tidy)
endif()


##########################################
# Cppcheck
##########################################

if(APS_CHRONY_DBUS_SERVICE_ENABLE_CPPCHECK)
    include(cmake/cppcheck.cmake)
endif()

##########################################
# Packaging
##########################################

include(GNUInstallDirs)
add_subdirectory(packaging)

##########################################
# Compiler-options
##########################################

# Enable chrony ipv6 support
add_compile_definitions("FEAT_IPV6")


# -pipe: Run compiler and assembler in parallel and do not use a temporary file for the assembler input. This can improve compilation performance. (This does not affect code generation.)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror=format-security -pipe")
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT APS_CHRONY_DBUS_SERVICE_ENABLE_CLANG_TIDY)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconcepts-diagnostics-depth=10")
endif()

# Output performance optimization flags
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -mtune=generic -flto=auto -march=x86-64-v3")

# Binary hardening flags based on:
# https://developers.redhat.com/blog/2018/03/21/compiler-and-linker-flags-gcc
# https://src.fedoraproject.org/rpms/redhat-rpm-config/blob/rawhide/f/buildflags.md
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fstack-protector-strong -fstack-clash-protection -fcf-protection -fstack-protector-all -D_FORTIFY_SOURCE=3 -D_GLIBCXX_ASSERTIONS")
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT APS_CHRONY_DBUS_SERVICE_ENABLE_CLANG_TIDY)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wl,-z,defs -Wl,-z,now -Wl,-z,relro -ffat-lto-objects")
endif()

# Configure position independent code manually since `CMAKE_POSITION_INDEPENDENT_CODE` did not work.
# PIE can be enabled for all builds (debug and release).
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIE -fPIC")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie")

# Skip false positives for the date.h library
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT APS_CHRONY_DBUS_SERVICE_ENABLE_CLANG_TIDY)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-error=stringop-overflow -Wno-stringop-overflow")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb3")

string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER)
add_compile_definitions("BUILDTYPE=${CMAKE_BUILD_TYPE}")
if(CMAKE_BUILD_TYPE_LOWER MATCHES ".*release.*")
    message(STATUS "Some sort of release build detected.")
else()
    message(STATUS "Some sort of debug build detected.")
    add_compile_definitions(DEBUG)
endif()

##########################################
# Dependencies
##########################################
set(ROOT_BINARY_DIR "${PROJECT_BINARY_DIR}")
# Ensure we get the root binary directory here
if(ROOT_BINARY_DIR MATCHES ".*\/CMake")
    string(REGEX REPLACE "\/CMake$" "" ROOT_BINARY_DIR "${ROOT_BINARY_DIR}")
endif()

# Make sure CMake can find our <name>-config.cmake files
set(CMAKE_PREFIX_PATH "${ROOT_BINARY_DIR}/build/${CMAKE_BUILD_TYPE}/generators")
message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")

find_package(PkgConfig REQUIRED)
# DBus include directory may vary depending on the distro so let packageconfig handle this
pkg_check_modules(DBUS REQUIRED dbus-1)
include_directories(SYSTEM ${DBUS_INCLUDE_DIRS})

##########################################
# Misc Dependencies
##########################################

include(clear_variable)

# Disable linting for external dependencies to prevent linting them
clear_variable(DESTINATION CMAKE_CXX_CLANG_TIDY BACKUP CMAKE_CXX_CLANG_TIDY_BKP)

include(FetchContent)

# Simppl
set(SIMPPL_USE_POLL OFF CACHE INTERNAL " " FORCE)
FetchContent_Declare(simppl GIT_REPOSITORY https://github.com/AP-Sensing/simppl.git
                            GIT_TAG d475fec04e0717f20a54574a6b557ba6c8f0fb90
                            SYSTEM)
FetchContent_MakeAvailable(simppl)
# Properly specify simppl to be installed else it will be missing when building an RPM
install(TARGETS simppl LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

restore_variable(DESTINATION CMAKE_CXX_CLANG_TIDY BACKUP CMAKE_CXX_CLANG_TIDY_BKP)

##########################################
# Static Code Analysis
##########################################

if(APS_CHRONY_DBUS_SERVICE_STATIC_ANALYSIS)
    # Has to be defined here at the bottom to prevent analyzing our dependencies, tests or gtest
    include(gcc_static_analysis)
endif()

##########################################
# Application
##########################################

add_subdirectory(src)


##########################################
# Tests
##########################################

if(APS_CHRONY_DBUS_SERVICE_BUILD_TESTS)
    # Needs to be called in the root dir for ctest to discover it inside the build directory
    enable_testing()
    add_subdirectory(tests)
endif()
