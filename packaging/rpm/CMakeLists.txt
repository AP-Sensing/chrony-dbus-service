cmake_minimum_required(VERSION 3.22)

include(${CMAKE_CURRENT_SOURCE_DIR}/cpack_config.cmake)

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/chrony-dbus-service.service"
        DESTINATION "/usr/lib/systemd/system"
        PERMISSIONS OWNER_READ OWNER_WRITE
                    GROUP_READ
                    WORLD_READ)

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/42-chrony-dbus-service.preset"
        DESTINATION "/usr/lib/systemd/system-preset"
        PERMISSIONS OWNER_READ OWNER_WRITE
                    GROUP_READ
                    WORLD_READ)
