#!/bin/bash

printf "🗂️  Building RPM...\n"
cpack

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

ret=$?
if [ $ret -ne 0 ]; then
    printf "❌ ${RED}Building RPM failed. Did you build successfully before (pressed F7)?${NC}\n"
else
    printf "✅ ${GREEN}RPM build successfully. Output: $(pwd)${NC}\n"
fi

exit $ret
