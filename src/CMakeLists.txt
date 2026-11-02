cmake_minimum_required(VERSION 3.27)

add_subdirectory(aps)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE aps::chronydbus
                                              aps::chronydbus::version
                                              ${DBUS_LIBRARIES})
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT RuntimeLibraries)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT "Executable")
