cmake_minimum_required(VERSION 3.27)

aps_generate_rpm_from_spec(
    "${CMAKE_CURRENT_LIST_DIR}/SPECS/chrony_dbus_service_selinux.spec" "${CMAKE_CURRENT_LIST_DIR}/SOURCES"
    IS_APPLICATION ON LINT_FILE "${CMAKE_CURRENT_LIST_DIR}/rpmlint_fedora.toml")
