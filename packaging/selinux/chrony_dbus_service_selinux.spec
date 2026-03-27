# vim: sw=4:ts=4:et


%define relabel_files() \
restorecon -R /usr/bin/chrony-dbus-service; \

%define selinux_policyver 40.30-1

Name:   chrony-dbus-service-selinux
Version:	1.0
Release:	1%{?dist}
Summary:	SELinux policy module for chrony_dbus_service

Group:	System Environment/Base
License:	GPLv2+

URL:		https://github.com/AP-Sensing/chrony-dbus-service
Source0:	chrony_dbus_service.pp
Source1:	chrony_dbus_service.if


Requires: policycoreutils-python-utils, libselinux-utils
Requires(post): selinux-policy-base >= %{selinux_policyver}, policycoreutils-python-utils
Requires(postun): policycoreutils-python-utils
Requires(post): chrony-dbus-service, chrony-dbus-service, chrony-dbus-service
BuildArch: noarch

%description
This package installs and sets up the  SELinux policy security module for chrony_dbus_service.

%install
install -d %{buildroot}%{_datadir}/selinux/packages
install -m 644 %{SOURCE0} %{buildroot}%{_datadir}/selinux/packages
install -d %{buildroot}%{_datadir}/selinux/devel/include/contrib
install -m 644 %{SOURCE1} %{buildroot}%{_datadir}/selinux/devel/include/contrib/

%post
semodule -n -i %{_datadir}/selinux/packages/chrony_dbus_service.pp

if /usr/sbin/selinuxenabled ; then
    /usr/sbin/load_policy
    %relabel_files
fi;
exit 0

%postun
if [ $1 -eq 0 ]; then

    semodule -n -r chrony_dbus_service
    if /usr/sbin/selinuxenabled ; then
       /usr/sbin/load_policy
       %relabel_files
    fi;
fi;
exit 0

%files
%attr(0600,root,root) %{_datadir}/selinux/packages/chrony_dbus_service.pp
%{_datadir}/selinux/devel/include/contrib/chrony_dbus_service.if


%changelog
* Thu Mar 26 2026 Samuel Stirtzel <s.stirtzel@googlemail.com> 1.0-1
- Initial version

