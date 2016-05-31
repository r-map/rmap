Summary: Archive for weather information
Name: arkimet
Version: 1.0
Release: 5
License: GPL
Group: Applications/Meteo
URL: https://github.com/arpa-simc/%{name}
Source0: https://github.com/arpa-simc/%{name}/archive/v%{version}-%{release}.tar.gz#/%{name}-%{version}-%{release}.tar.gz
Source1: https://github.com/arpa-simc/%{name}/raw/v%{version}-%{release}/fedora/SOURCES/%{name}.init
Source2: https://github.com/arpa-simc/%{name}/raw/v%{version}-%{release}/fedora/SOURCES/%{name}.default
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
BuildRequires: doxygen libdballe-devel >= 5.19 lua-devel >= 5.1 grib_api-devel sqlite-devel >= 3.6 curl-devel geos-devel pkgconfig readline-devel lzo-devel libwreport-devel >= 3.0 flex bison meteo-vm2-devel >= 0.12 hdf5-devel
BuildRequires: jasper-devel libpng-devel openjpeg-devel netcdf-devel popt-devel
Requires: hdf5 meteo-vm2 >= 0.12 grib_api-1.10.0
Requires(preun): /sbin/chkconfig, /sbin/service
Requires(post): /sbin/chkconfig, /sbin/service

%{!?python_sitelib: %define python_sitelib %(%{__python} -c "from distutils.sysconfig import get_python_lib; print get_python_lib()")}
%{!?python_sitearch: %define python_sitearch %(%{__python} -c "from distutils.sysconfig import get_python_lib; print get_python_lib(1)")}
%{!?python_siteinc: %define python_siteinc %(%{__python} -c "from distutils.sysconfig import get_python_inc; print get_python_inc()")}

Requires: python >= 2.5

%description
 Description to be written.

%package  -n arkimet-devel
Summary:  Archive for weather information (development library)
Group:    Applications/Meteo

%description -n arkimet-devel
 Description to be written.

%prep
%setup -q -n %{name}-%{version}-%{release}
sh autogen.sh

%build
%configure
make
make check

%install
[ "%{buildroot}" != / ] && rm -rf %{buildroot}
%makeinstall

install -D -m0755 %{SOURCE1} %{buildroot}%{_sysconfdir}/rc.d/init.d/%{name}
install -bD -m0755 %{SOURCE2} %{buildroot}%{_sysconfdir}/default/arki-server


%clean
[ "%{buildroot}" != / ] && rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
%dir %{_sysconfdir}/arkimet
%config(noreplace) %{_sysconfdir}/arkimet/match-alias.conf
%{_sysconfdir}/arkimet/bbox/*
%{_sysconfdir}/arkimet/format/*
%{_sysconfdir}/arkimet/qmacro/*
%{_sysconfdir}/arkimet/report/*
%{_sysconfdir}/arkimet/scan-bufr/*
%{_sysconfdir}/arkimet/scan-grib1/*
%{_sysconfdir}/arkimet/scan-grib2/*
%{_sysconfdir}/arkimet/scan-odimh5/*
%{_sysconfdir}/arkimet/targetfile/*
%{_sysconfdir}/arkimet/vm2/*
%{_bindir}/*
%{_sysconfdir}/rc.d/init.d/%{name}
%config(noreplace) %{_sysconfdir}/default/arki-server
%doc %{_mandir}/man1/*
%doc README TODO
%doc %{_docdir}/arkimet/*

%files -n arkimet-devel
%defattr(-,root,root,-)
%{_libdir}/libarkimet*.a
%{_libdir}/libarkimet*.la
%dir %{_includedir}/arki
%{_includedir}/arki/*


%pre
#/sbin/service %{name} stop >/dev/null 2>&1

%post
/sbin/ldconfig
/sbin/chkconfig --add %{name}

%preun

%postun
/sbin/ldconfig
if [ "$1" = "0" ]; then
  # non e' un upgrade, e' una disinstallazione definitiva
  #/sbin/chkconfig --del %{name} (pare farlo in automatico)
  :
else
  /sbin/service %{name} condrestart >/dev/null 2>&1
fi

%changelog
* Thu Jan  7 2016 Daniele Branchini <dbranchini@arpa.emr.it> - 1.0-1%{dist}
- Ported to c++11

* Tue Oct 13 2015 Daniele Branchini <dbranchini@arpa.emr.it> - 0.81-1%{dist}
- Ported to wreport 3
- Fixed argument passing, that caused use of a deallocated string

* Wed Feb  4 2015 Daniele Branchini <dbranchini@arpa.emr.it> - 0.80-3153%{dist}
- fixed arki-scan out of memory bug

* Wed Jan 21 2015 Daniele Branchini <dbranchini@arpa.emr.it> - 0.80-3146%{dist}
- fixed large query bug

* Thu Mar 20 2014 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.75-2876%{dist}
- libwibble-devel dependency (--enable-wibble-standalone in configure)
- VM2 derived values in serialization

* Tue Sep 10 2013 Daniele Branchini <dbranchini@arpa.emr.it> - 0.75-2784%{dist}
- SmallFiles support
- split SummaryCache in its own file
- arkiguide.it

* Wed Jun 12 2013 Daniele Branchini <dbranchini@arpa.emr.it> - 0.74-2763%{dist}
- corretto bug nel sort dei dati

* Wed May 22 2013 Daniele Branchini <dbranchini@arpa.emr.it> - 0.74-2759%{dist}
- arki-check now can do repack and archival in a single run
- arki-check now does not do repack if a file is to be deleted
- added support for VM2 data
- arki-scan now supports bufr:- for scanning BUFR files from standard input
- ODIMH5 support moves towards a generic HDF5 support

* Wed Jan  9 2013 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.73-2711%{dist}
- Rebuild to reflect upstream changes (fixed arki-xargs serialization)

* Mon Nov 26 2012 Daniele Branchini <dbranchini@arpa.emr.it> - 0.73-2677%{dist}
- Rebuild to reflect upstream changes (adding meteo-vm2)

* Thu Apr 19 2012 Daniele Branchini <dbranchini@arkitest> - 0.71-2505%{dist}
- Rebuild to reflect upstream changes

* Thu Feb 23 2012 Daniele Branchini <dbranchini@arkitest> - 0.69-2497%{dist}
- Rebuild to reflect upstream changes

* Mon Sep 19 2011 Daniele Branchini <dbranchini@linus> - 0.64-2451%{dist}
- Rebuild to reflect upstream changes

* Fri Aug 12 2011 Daniele Branchini <dbranchini@linus> - 0.63-2431%{dist}
- Effettivo supporto per i 32bit

* Wed Jul 13 2011 Daniele Branchini <dbranchini@linus> - 0.62-2402%{dist}
- corretto initscript

* Mon Jul 11 2011 Daniele Branchini <dbranchini@linus> - 0.62-2401%{dist}
- Aggiunto initscript

* Thu Oct 14 2010 Daniele Branchini <dbranchini@pigna> - 0.46-2163%{dist}
- Added radarlib support

* Mon Jul 12 2010 Daniele Branchini <dbranchini@linus> - 0.44-2067%{dist}
- Corretta dipendenza da libarkimet0 (obsoleto)

* Thu Jul  8 2010 Daniele Branchini <dbranchini@carenza.metarpa> - 0.44-2066%{dist}
- Rebuild to reflect upstream changes, removed libarkimet0

* Thu Sep 17 2009 root <root@localhost.localdomain> - 0.27-1418
- Rebuild to reflect upstream changes.

* Tue Sep  8 2009 root <root@localhost.localdomain> - 0.26.1-1409
- added some documentation files

* Mon Sep  7 2009 Daniele Branchini <dbranchini@carenza.metarpa> - 0.26.1-1
- Rebuild to reflect upstream changes.

* Wed Aug 26 2009 Daniele Branchini <dbranchini@carenza.metarpa> - 0.25-1
- Rebuild to reflect upstream changes.

* Tue Jul  1 2008 root <enrico@enricozini.org> - 0.4-1
- Initial build.
