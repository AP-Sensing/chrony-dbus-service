#!/bin/bash

printf "🗞️  Building doxygen docs...\n"
mkdir -p build/doxygen
doxygen tools/doxygen/doxygen.cfg
ret=$?

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

if [ $ret -ne 0 ]; then
    printf "❌ ${RED}Building doxygen docs failed.${NC}\n"
else
    printf "✅ ${GREEN}Doxygen docs build. Output: $(pwd)/build/doxygen${NC}\n"
fi

exit $ret
