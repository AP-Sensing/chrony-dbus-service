thisDir=$(dirname -- "$(readlink -f -- "${BASH_SOURCE[0]}")")

cp -f ${thisDir}/chronyd.service /usr/lib/systemd/system/
cp -f ${thisDir}/../packaging/rpm/org.freedesktop.ChronyDBus.conf /usr/share/dbus-1/system.d/

systemctl daemon-reload
systemctl restart chronyd.service
