cmake_minimum_required(VERSION 3.27)

add_library(chrony_dbus)
add_library(chrony::dbus ALIAS chrony_dbus)

target_sources(
    chrony_dbus
    PRIVATE chrony_dbus_service.cpp
    PUBLIC FILE_SET
           HEADERS
           BASE_DIRS
           ${PROJECT_SOURCE_DIR}/include
           FILES
           ${PROJECT_SOURCE_DIR}/include/chrony/dbus/chrony_dbus_service.hpp
           ${PROJECT_SOURCE_DIR}/include/chrony/dbus/dbus_interface.hpp)

target_link_libraries(chrony_dbus PUBLIC Simppl::simppl)
install(TARGETS chrony_dbus LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
