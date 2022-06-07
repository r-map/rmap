Summary: Mosquitto Auth Plugin
Name: mosquitto-auth-plug
Version: 1.3
Release: 2
License: BSD
Group: System Environment/Base
Source: mosquitto-auth-plug.tar.gz
%define debug_package %{nil}
BuildRequires: gcc mosquitto-devel libcurl-devel openssl-devel
Requires: mosquitto

%description
Mosquitto Auth Plugin

%prep -n %{name}
%setup -q -n %{name}

%build -d  %{name}
make RPM_OPT_FLAGS="$RPM_OPT_FLAGS"

%install
%make_install

%files
/usr/lib64/auth-plug.so

%changelog
* Tue Sep 6 2016 David Achenbach <dachenbach@mydevices.com>
- RPM build directly from 0.0.7 source
