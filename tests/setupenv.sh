## This script needs to be sourced
## Starts dbus-daemon and exports the DBUS_SESSION_BUS_ADDRESS so that following processes can use the session bus
## Also prints the export line to easily copy it to other shells
## Starts chronyd without forking and without setting system time to be able to start in a container

_TMPFILE=$(mktemp -t "dbus-session.XXXX")
[ -f ${_TMPFILE} ] ||  { echo "Failed to create tmpfile at '${_TMPFILE}' "; exit 1; }
echo -n "export DBUS_SESSION_BUS_ADDRESS=" > ${_TMPFILE}
dbus-daemon --session --fork --print-address 1 >> ${_TMPFILE}
. ${_TMPFILE}
rm -f ${_TMPFILE}
chronyd -nx &
echo "export DBUS_SESSION_BUS_ADDRESS=${DBUS_SESSION_BUS_ADDRESS}"
