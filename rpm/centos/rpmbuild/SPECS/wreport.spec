Summary: Library for working with (coded) weather reports
Name: wreport
Version: 3.4
Release: 1
License: GPL2
Group: Applications/Meteo
URL: http://www.arpa.emr.it/dettaglio_documento.asp?id=514&idlivello=64
Source0: https://github.com/arpa-simc/%{name}/archive/v%{version}-%{release}.tar.gz#/%{name}-%{version}-%{release}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
BuildRequires: doxygen, libtool, lua-devel >= 5.1.1, python-devel, gcc-c++
%description
null

Summary: Tools for working with weather reports
Group: Applications/Meteo
Requires: lib%{name}-common

%description
 libwreport is a C++ library to read and write weather reports in BUFR and CREX
 formats.

The tools provide simple weather bulletin handling functions


%package -n lib%{name}3
Summary: shared library for working with weather reports
Group: Applications/Meteo
Requires: lib%{name}-common

%description -n lib%{name}3
 libwreport is a C++ library to read and write weather reports in BUFR and CREX
 formats.
 
 This is the shared library for C programs.

%package -n lib%{name}-common
Summary: shared library for working with weather reports
Group: Applications/Meteo

%description -n lib%{name}-common
 libwreport is a C++ library to read and write weather reports in BUFR and CREX
 formats.
 
 This is the shared library for C programs.


%package -n lib%{name}-doc
Summary: documentation for libwreport
Group: Applications/Meteo

%description -n lib%{name}-doc
libwreport is a C++ library to read and write weather reports in BUFR and CREX
 formats.

 This is the documentation for the library.

%package -n lib%{name}-devel
Summary:  Library for working with (coded) weather reports
Group: Applications/Meteo
Requires: lib%{name}3 = %{version}

%description -n lib%{name}-devel
libwreport is a C++ library to read and write weather reports in BUFR and CREX
 formats.
 .
 It also provides a useful abstraction to handle values found in weather
 reports, with awareness of significant digits, measurement units, variable
 descriptions, unit conversion and attributes on variables.
 .
 Features provided:
 .
  * Unit conversion
  * Handling of physical variables
  * Read and write BUFR version 2, 3, and 4
  * Read and write CREX

%package -n python-%{name}3
Summary: shared library for working with weather reports
Group: Applications/Meteo
Requires: lib%{name}3

%description -n python-%{name}3
libwreport is a C++ library to read and write weather reports in BUFR and CREX
 formats.

 This is the Python library


%prep
%setup -q -n %{name}-%{version}-%{release}

%build

autoreconf -ifv

%configure
make

%check
make check

%install
[ "%{buildroot}" != / ] && rm -rf "%{buildroot}"
make install DESTDIR="%{buildroot}"

%clean
[ "%{buildroot}" != / ] && rm -rf "%{buildroot}"

%files
%defattr(-,root,root,-)
%{_bindir}/wrep
%{_bindir}/wrep-importtable

%files -n lib%{name}3
%defattr(-,root,root,-)
%{_libdir}/libwreport.so.*

%files -n lib%{name}-common
%defattr(-,root,root,-)
%{_datadir}/wreport/[BD]*


%files -n lib%{name}-devel
%defattr(-,root,root,-)
%{_libdir}/libwreport.a
%{_libdir}/libwreport.la
%{_libdir}/pkgconfig/libwreport.pc
%{_libdir}/libwreport.so

%dir %{_includedir}/%{name}
%{_includedir}/%{name}/*


%files -n lib%{name}-doc
%defattr(-,root,root,-)
%doc %{_docdir}/%{name}/libwreport.doxytags
%doc %{_docdir}/%{name}/apidocs/*
%doc %{_docdir}/%{name}/examples/*

%files -n python-%{name}3
%defattr(-,root,root,-)
%dir %{python_sitelib}/wreport
%{python_sitelib}/wreport/*
%dir %{python_sitearch}
%{python_sitearch}/*.a
%{python_sitearch}/*.la
%{python_sitearch}/*.so*

%doc %{_docdir}/wreport/python-wreport.html
%doc %{_docdir}/wreport/python-wreport.rst

%changelog
* Tue Sep 15 2015 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 3.2-1%{dist}
- gcc 4.8.3 support (lambdas and variadic templates)
- Removed every reference to libwreport-test.pc.in
- version 3.2 and version-info 3:2:0
- Normalise CODE TABLE and FLAG TABLE units
-  Removed dependency on wibble in favour of new utils/ code in sync with wobble

* Mon Aug 31 2015 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 3.1-1%{dist}
- Fixed FLAGTABLE and CODETABLE conversion errors

* Wed Jul 29 2015 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 3.0-0.1%{dist}
- wreport 3.0 pre-release

* Tue Aug 05 2014 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 2.13-4298%{dist}
- Updated wrep-importtable to deal with new zipfiles and XML files published by
  WMO
- Ship more tables (CREX table 17 is a copy of table 18 as a workaround, since
  I could not find a parseable version of table 17)
- Added more unit conversions to deal with the changed unit names in new BUFR
  tables

* Wed Nov 13 2013 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 2.10-4116%{dist}
- Fixed linking bug.

* Tue Nov 12 2013 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 2.10-4104%{dist}
- No changes, but bumped the version number to succeed an internally released 2.9

* Fri Aug 23 2013 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 2.9-3968%{dist}
- Aggiunta conversione ug/m**3->KG/M**3 (e viceversa)

* Fri Nov 23 2012 root <dbranchini@arpa.emr.it> - 2.5-3724%{dist}
- Aggiornamento sorgenti

* Tue Jun 12 2012 root <dbranchini@arpa.emr.it> - 2.5-3633%{dist}
- Aggiornamento sorgenti

* Tue May 8 2012 root <dbranchini@arpa.emr.it> - 2.4-3621%{dist}
- Aggiunta conversione da minuti (oracle) a S

* Tue Sep 28 2010 root <ppatruno@arpa.emr.it> - 1.0
- Initial build.
