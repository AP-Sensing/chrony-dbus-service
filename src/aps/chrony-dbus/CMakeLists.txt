cmake_minimum_required(VERSION 3.27)

add_library(aps_chronydbus)
add_library(aps::chronydbus ALIAS aps_chronydbus)

include_directories(SYSTEM ${chrony_SOURCE_DIR}/include)

target_sources(
    aps_chronydbus
    PRIVATE chrony_dbus_service.cpp
    PUBLIC FILE_SET
           HEADERS
           BASE_DIRS
           ${PROJECT_SOURCE_DIR}/src/include
           FILES
           ${PROJECT_SOURCE_DIR}/src/include/aps/chrony-dbus/chrony_dbus_service.hpp
           ${PROJECT_SOURCE_DIR}/src/include/aps/chrony-dbus/ChronyDbusInterface.hpp
           ${PROJECT_SOURCE_DIR}/src/include/aps/chrony-dbus/polkit_interface.hpp)

target_link_libraries(aps_chronydbus PUBLIC Simppl::simppl)
install(TARGETS aps_chronydbus LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
