%define debug_package %{nil}

Name:          mqtt2bufr
Version:       1.8
Release:       1%{?dist}
Summary:       Tools to publish BUFR messages to MQTT and to convert MQTT messages to BUFR
Group:         Development
Distribution:  simc
Packager:      Paolo Patruno,,, <ppatruno@arpa.emr.it>
Source:        mqtt2bufr-%{version}.tar.gz
License:       G.P.L.
BuildRoot:     %{_tmppath}/%{name}-%{version}-root
#BuildRequires:  libtool jansson-devel libdballe-devel libwibble-devel popt-devel mosquitto-devel help2man
BuildRequires:  gcc python3 python3-dballe python3-rmap help2man
Requires:  python3 python3-dballe python3-rmap>=16.8

%description
Tools to publish BUFR messages to MQTT and to convert MQTT messages to BUFR.

%prep
%setup -q

%build
%configure
make
make check

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
* Wed Mar 30 2016 Dario Mastropasqua,,, <dariomas@users.noreply.github.com> - 0.11-1%{dist}
- corrected dependencies Require for RHEL (CentOS)

* Fri Sep 12 2014 Paolo Patruno <ppatruno@arpa.emr.it> - 0.2-1
- first build
