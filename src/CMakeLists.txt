cmake_minimum_required(VERSION 3.27)

add_executable(${PROJECT_NAME} main.cpp
    dbus/chronydbusservice.h dbus/chronydbusservice.cpp
    dbus/dbusinterface.h
    chrony/chronyutil.h
    chrony/chronypktlength.h
    chrony/chronysocket.h
    chrony/chronyclient.h
    chrony/chronynameserv.h
)
target_link_libraries(${PROJECT_NAME} PRIVATE simppl
                                              ${DBUS_LIBRARIES})
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT RuntimeLibraries)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT "Executable")
