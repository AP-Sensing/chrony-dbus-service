#!/bin/sh -e

printf "🗂️  Building Policy...\n"

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'


ret=$?
if [ $ret -ne 0 ]; then
    printf "❌ ${RED}Building Policy failed.?${NC}\n"
else
    printf "✅ ${GREEN}Policy build successfully. Output: $(pwd)${NC}\n"
fi

printf "🗂️  Building Policy RPM...\n"

pwd=$(pwd)
rpmbuild --define "_sourcedir ${pwd}/SOURCES" --define "_specdir ${pwd}/SPECS" --define "_builddir ${pwd}" --define "_srcrpmdir ${pwd}" --define "_rpmdir ${pwd}" --define "_buildrootdir ${pwd}/.build"  -ba ./SPECS/chrony_dbus_service_selinux.spec

if [ $ret -ne 0 ]; then
    printf "❌ ${RED}Building Policy RPM failed. Did you build successfully before (pressed F7)?${NC}\n"
else
    printf "✅ ${GREEN}Policy RPM build successfully. Output: $(pwd)${NC}\n"
fi

exit $ret
