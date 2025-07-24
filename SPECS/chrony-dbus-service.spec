ExclusiveArch:  x86_64
Name:           chrony-dbus-service
Version:        0.1.0
Release:        %{?aps_release_id}%{!?aps_release_id:1}
License:        GPL-2.0
Group:          Unspecified
Summary:        RPM for configuring chrony via DBus.
Distribution:   PhotonPonyOS

URL:            https://github.com/AP-Sensing/crony-dbus-service
Vendor:         AP Sensing
Packager:       AP Sensing
Provides:       chrony-dbus-service = %{version}-%{release}

Source0:        %{name}-%{version}.tar.gz

Requires:       systemd
Requires:       dbus-1
BuildRequires:  g++
BuildRequires:  cmake
# Required for the '{_unitdir}' macro to be available during build time
BuildRequires:  systemd
BuildRequires:  dbus-1

%{?systemd_requires}

%description
RPM package providing the functionality to configure chrony via DBus by utilizing the chronyc socket API.

%prep
tar -xzf %{SOURCE0} .

%build
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel

%install
# Executable
install -d -m 755 $RPM_BUILD_ROOT/usr/bin/
install -m 755 build/src/%{name} $RPM_BUILD_ROOT/usr/bin

# Systemd
install -d -m 755 $RPM_BUILD_ROOT/usr/lib/systemd/system
install -m 644 systemd/%{name}.service $RPM_BUILD_ROOT%{_unitdir}

install -d -m 755 $RPM_BUILD_ROOT/usr/lib/systemd/system-preset
install -m 644 systemd/42-%{name}.preset $RPM_BUILD_ROOT/usr/lib/systemd/system-preset

%post
%systemd_post %{name}.service

%preun
%systemd_preun %{name}.service

%postun
%systemd_postun_with_restart %{name}.service

%files
# Executable
%attr(755, root, root) /usr/bin/%{name}

# Systemd
%attr(644, root, root) %{_unitdir}/%{name}.service

%attr(644, root, root) /usr/lib/systemd/system-preset/42-%{name}.preset

%changelog
* Wed Jul 23 2025 Samuel Stirtzel <s.stirtzel@googlemail.com> - 0.1.0
- Initial test version 0.1.0
