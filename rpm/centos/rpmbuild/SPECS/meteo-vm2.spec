Name:           meteo-vm2
Version:        0.36
Release:        1
Summary:        C++ library for VM2 data 

License:        GPLv2+
URL:            https://github.com/arpa-simc/%{name}
Source0:        https://github.com/arpa-simc/%{name}/archive/v%{version}-%{release}.tar.gz#/%{name}-%{version}-%{release}.tar.gz
BuildRequires:  libtool pkgconfig lua-devel >= 5.1 libdballe-devel >= 5.19 help2man cnf-devel popt-devel gcc-gfortran

%if 0%{?fedora} < 19
Requires:       lua = 5.1
%endif

%if 0%{?fedora} == 20
Requires:       lua >= 5.2
Requires:       lua < 5.3
%endif

%description
VM2 decoding/encoding library

%prep
%setup -q -n %{name}-%{version}-%{release}

%build
sh autogen.sh
%configure
make %{?_smp_mflags}

%check
make check

%install
rm -rf $RPM_BUILD_ROOT
%make_install

%files
%defattr(-,root,root,-)
%{_libdir}/lib%{name}.so.*
%dir %{_sharedstatedir}/%{name}
%{_sharedstatedir}/%{name}/source/default.lua*
%{_sharedstatedir}/%{name}/source/bufr.lua*

%package devel
Summary:        C++ library for VM2 data - development files
Requires:       %{name} = %{?epoch:%epoch:}%{version}-%{release}

%description devel
VM2 decoding/encoding library - development files

%files devel
%defattr(-,root,root,-)
%dir %{_includedir}/%{name}
%{_includedir}/%{name}/*
%{_libdir}/lib%{name}.a
%{_libdir}/lib%{name}.la
%{_libdir}/lib%{name}.so
%{_libdir}/pkgconfig/meteo-vm2.pc

%package doc
Summary:        C++ library for VM2 data - documentation
BuildRequires:  doxygen texlive-epstopdf

%description doc
VM2 decoding/encoding library - documentation

%files doc
%doc %{_docdir}/%{name}
%defattr(-,root,root,-)

%package fortran
Summary:        meteo-vm2 Fortran library
Requires:       %{name} = %{?epoch:%epoch:}%{version}-%{release}, cnf-devel

%description fortran
VM2 decoding/encoding library - Fortran bindings

%files fortran
%defattr(-,root,root,-)
%{_libdir}/lib%{name}-fortran.so.*

%package fortran-devel
Summary:        meteo-vm2 Fortran development library
Requires:       %{name} = %{?epoch:%epoch:}%{version}-%{release}, %{name}-fortran = %{?epoch:%epoch:}%{version}-%{release}

%description fortran-devel
VM2 decoding/encoding library - Fortran development files

%files fortran-devel
%defattr(-,root,root,-)
%{_includedir}/meteo-vm2-fortran.h
%{_libdir}/lib%{name}-fortran.a
%{_libdir}/lib%{name}-fortran.la
%{_libdir}/lib%{name}-fortran.so

%package utils
Summary:        meteo-vm2 utilities
Requires:       %{name} = %{?epoch:%epoch:}%{version}-%{release}, libdballe6 >= 6.0

%description utils
Collection of utilities for VM2 files

%files utils
%defattr(-,root,root,-)
%{_bindir}/meteo-vm2-to-bufr
%{_bindir}/bufr-to-meteo-vm2
%{_mandir}/*

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%changelog
* Mon Dec 28 2015 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.36-1
- Stations from fidutn network 
- Updated stations and variables

* Wed Oct 14 2015 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.35-1
- Fixed bufr-to-meteo-vm2 exit status

* Wed Oct 14 2015 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.34-1%{dist}
- Pollen unit

* Tue Oct 13 2015 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.33-1%{dist}
- Fixed error message
- Parser::regexp_str public again

* Mon Oct 12 2015 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.32-1%{dist}
- Fixed bug in gcc-4.8 regex parser

* Mon Oct 12 2015 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.31-1%{dist}
- Source updated

* Fri Oct 09 2015 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.30-1%{dist}
- gcc 4.8.3 support

* Thu Oct 01 2015 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.29-1%{dist}
- Removed wibble
- C++11

* Mon Aug 03 2015 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.27-1%{dist}
- Back to old reports

* Fri Jun 19 2015 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.26-1%{dist}
- dballe-7.1-4828

* Fri Jun 05 2015 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.25-3%{dist}
- Fix default.lua, again

* Fri Jun 05 2015 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.25-2%{dist}
- Fix default.lua

* Fri Jun 05 2015 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.25%{dist}
- default.lua and bufr.lua source files

* Thu Jun 04 2015 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.24%{dist}
- New stations and variables
- dballe 7.1-4758 support

* Tue May 26 2015 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.23%{dist}
- Disabled Python bindings

* Tue May 26 2015 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.22%{dist}
- dballe-7.1-4749 support

* Tue May 12 2015 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.21%{dist}
- Fixed l1, l2 and missing values as nil

* Mon May 11 2015 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.20%{dist}
- New stations

* Fri Apr 17 2015 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.19%{dist}
- New stations

* Thu Feb 19 2015 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.18.1-1%{dist}
- Error message

* Wed Feb 18 2015 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.18-1%{dist}
- Fixed parser

* Tue Sep 16 2014 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.17-1%{dist}
- bufr-to-meteo-vm2: optional ident

* Tue Sep 16 2014 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.16-1%{dist}
- Updated stations
- Stations data for BUFR conversion

* Wed Sep 03 2014 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.15-1%{dist}
- New stations 1034, 1035, 1036

* Wed Jun 25 2014 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.14-1%{dist}
- Fixed stations

* Wed Jun 25 2014 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.13-2%{dist}
- Fixed lua dependency

* Wed Jun 25 2014 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.13-1%{dist}
- Updated stations

* Mon Jun 23 2014 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.12-1%{dist}
- Stations 4217,4222,4240,4247,4344 to idrmec-pub
- Station 13023 to rer

* Thu Jun 19 2014 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.11-2%{dist}
- meteo-vm2-to-bufr: fixed error message

* Thu Jun 19 2014 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.11-1%{dist}
- Updated stations

* Wed Jan 15 2014 Daniele Branchini <dbranchini@carenza.metarpa> - 0.10-2%{dist}
- Fixed error in pkg-config

* Wed Jan 15 2014 Daniele Branchini <dbranchini@arpa.emr.it> - 0.10-1%{dist}
- Support for Lua versioning

* Thu Dec 05 2013 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.9-1%{dist}
- Aliases in station attributes (lon, lat, rep)

* Wed Oct 02 2013 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.8-1%{dist}
- default.lua has station attributes with bcodes as keys

* Mon Sep 30 2013 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.7-2%{dist}
- Updated variables and stations

* Tue Aug 27 2013 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.7-1%{dist}
- Fixed p1 for daily airquality variables (p1=86400,p2=86400)

* Fri Aug 23 2013 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.6-1%{dist}
- Airquality stations and variables (experimental)

* Wed Jul 31 2013 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.5-1%{dist}
- Attribute tables updated

* Fri Jul 26 2013 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.4-1%{dist}
- fixed bug in bufr-to-meteo-vm2
- messages with missing value are skipped in meteo-vm2-to-bufr

* Tue May 14 2013 Daniele Branchini <dbranchini@arpa.emr.it> - 0.3-2%{dist}
- fixed dep for libdballe6

* Fri May 10 2013 Daniele Branchini <dbranchini@arpa.emr.it> - 0.3-1%{dist}
- reflecting upstream changes

* Fri Nov 30 2012 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.1-3%{dist}
- %post and %postun directives

* Thu Nov 29 2012 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.1-2%{dist}
- Fortran bindings

* Tue Oct 30 2012 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 0.1-1%{dist}
- First version
