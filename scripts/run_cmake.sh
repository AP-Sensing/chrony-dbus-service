#!/bin/bash

printf "Running CMake, exporting compile commands"
rm -rf build/*
cmake . -B build \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DAPS_CHRONY_DBUS_SERVICE_BUILD_TESTS=ON
