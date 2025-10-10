cmake_minimum_required(VERSION 3.27)

add_subdirectory(dbus)

add_library(chrony)

target_sources(
    chrony
    PUBLIC FILE_SET
           HEADERS
           BASE_DIRS
           ${PROJECT_SOURCE_DIR}/include
           FILES
           ${PROJECT_SOURCE_DIR}/include/chrony/chronyaddressing.h
           ${PROJECT_SOURCE_DIR}/include/chrony/chronycandm.h
           ${PROJECT_SOURCE_DIR}/include/chrony/chronyclient.h
           ${PROJECT_SOURCE_DIR}/include/chrony/chronynameserv.h
           ${PROJECT_SOURCE_DIR}/include/chrony/chronypktlength.h
           ${PROJECT_SOURCE_DIR}/include/chrony/chronysocket.h
           ${PROJECT_SOURCE_DIR}/include/chrony/chronyutil.h)
set_target_properties(chrony PROPERTIES LINKER_LANGUAGE CXX)
