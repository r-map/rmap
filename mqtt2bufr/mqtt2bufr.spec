Name:          mqtt2bufr
Version:       0.2
Release:       1%{?dist}
Summary:       Tools to publish BUFR messages to MQTT and to convert MQTT messages to BUFR
Group:         Development
Vendor:        starlink
Distribution:  sim
Packager:      Paolo Patruno,,, <ppatruno@arpa.emr.it>
Source:        mqtt2bufr-%{version}.tar.gz
License:       G.P.L.
BuildRoot:     %{_tmppath}/%{name}-%{version}-root
BuildRequires: jansson-devel
BuildRequires: libdballe-devel
BuildRequires: mosquitto-devel
BuildRequires: libwibble-devel
BuildRequires: help2man

%description
Tools to publish BUFR messages to MQTT and to convert MQTT messages to BUFR.

%prep
%setup -q

%build
%configure
make

%install
[ "%{buildroot}" != / ] && rm -rf "%{buildroot}"
%makeinstall

%clean
[ "%{buildroot}" != / ] && rm -rf "%{buildroot}"

%files 
%defattr(-,root,root)
%doc README 
%dir %{_mandir}/man1
%doc %{_mandir}/man1/*
%{_bindir}/*
  

%changelog
* Fri Sep 12 2014 Paolo Patruno <ppatruno@arpa.emr.it> - 0.2-1
- first build
