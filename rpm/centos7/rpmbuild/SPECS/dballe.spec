Summary: DB-ALLe is a database for punctual metereological data  (Command line tools)
Name: dballe
Version: 7.8
Release: 1
License: GPL
Group: Applications/Meteo
URL: https://github.com/ARPA-SIMC/dballe
#Source0: %{name}-%{version}.tar.gz
Source0: https://github.com/arpa-simc/%{name}/archive/v%{version}-%{release}.tar.gz#/%{name}-%{version}-%{release}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
BuildRequires: unixODBC-devel, gperf, cnf-devel,  doxygen, latex2html, python-docutils, lua-devel, libwreport-devel >= 3.2 , swig , python-devel, popt-devel, postgresql-devel, mariadb-devel
BuildRequires: %{?rhel: texlive-tetex, texlive-latex, texlive-moreverb, texlive-subfigure} %{?fedora: tetex, tetex-latex}
BuildRequires: sqlite-devel, help2man, gcc-gfortran, python-wreport3
Requires: %{name}-common = %{?epoch:%epoch:}%{version}-%{release}, unixODBC, sqliteodbc, mysql-connector-odbc, python-%{name}
Obsoletes: provami <= 7.6

%{!?python_sitelib: %define python_sitelib %(%{__python} -c "from distutils.sysconfig import get_python_lib; print get_python_lib()")}
%{!?python_sitearch: %define python_sitearch %(%{__python} -c "from distutils.sysconfig import get_python_lib; print get_python_lib(1)")}
%{!?python_siteinc: %define python_siteinc %(%{__python} -c "from distutils.sysconfig import get_python_inc; print get_python_inc()")}

%description
 Database for punctual meteorological data (Command line tools)
 DB-All.e is a fast on-disk database where meteorological observed and
 forecast data can be stored, searched, retrieved and updated.
 .
 This framework allows to manage large amounts of data using its simple
 Application Program Interface, and provides tools to visualise, import
 and export in the standard formats BUFR, AOF and CREX.
 .
 The main characteristics of DB-ALL.e are:
 .
  * Fortran, C, C++ and Python APIs are provided.
  * To make computation easier, data is stored as physical quantities,
    that is, as measures of a variable in a specific point of space and
    time, rather than as a sequence of report.
  * Internal representation is similar to BUFR and CREX WMO standard
    (table code driven) and utility for import and export are included
    (generic and ECMWF template).
  * Representation is in 7 dimensions: latitude and longitude geographic
    coordinates, table driven vertical coordinate, reference time,
    table driven observation and forecast specification, table driven
    data type.
  * It allows to store extra information linked to the data, such as
    confidence intervals for quality control.
  * It allows to store extra information linked to the stations.
  * Variables can be represented as real, integer and characters, with
    appropriate precision for the type of measured value.
  * It is based on physical principles, that is, the data it contains are
    defined in terms of homogeneous and consistent physical data. For
    example, it is impossible for two incompatible values to exist in the
    same point in space and time.
  * It can manage fixed stations and moving stations such as airplanes or
    ships.
  * It can manage both observational and forecast data.
  * It can manage data along all three dimensions in space, such as data
    from soundings and airplanes.
  * Report information is preserved. It can work based on physical
    parameters or on report types.

%package  -n libdballe-devel
Summary:  DB-ALL.e core C development library
Group:    Applications/Meteo
Requires: lib%{name}6 = %{?epoch:%epoch:}%{version}-%{release} lua-devel postgresql-devel mariadb-devel sqlite-devel libwreport-devel
Obsoletes: libdballepp-devel 

%description -n libdballe-devel
 DB-ALL.e core C development library
 DB-All.e is a fast on-disk database where meteorological observed and
 forecast data can be stored, searched, retrieved and updated.

The Fedora packaging of DB-All.e includes all the features of the libraries,
 but any subset can be used without interference from other subsets.  It is
 also possible to rebuild the library to include only those features that are
 needed.
 .
 Features provided:
 .
  * Unit conversion
  * Handling of physical variables
  * Encoding and decoding of BUFR and CREX reports from:
     * fixed land and sea stations, like synops and buoys
     * mobile stations: ships, airplanes
     * soundings: temp, pilot
     * METAR reports
     * Satellite strides (decode only)
  * Decoding of AOF reports
  * Interpretation of weather reports as physical data precisely located in
    space and time, and encoding of physical data into weather reports.
  * Smart on-disk database for observed and forecast weather data based on
    physical principles, built to support operations such as quality control,
    data thinning, correlation of data from mixed sources

%package -n libdballe-doc
Summary:   DB-ALL.e core C development library (documentation)
Group: Applications/Meteo
%description  -n libdballe-doc
 DB-ALL.e core C development library (documentation)
 DB-All.e is a fast on-disk database where meteorological observed and
 forecast data can be stored, searched, retrieved and updated.
 .
 This is the documentation for the core DB_All.e development library.


%package  -n libdballe6
Summary:   DB-ALL.e core shared library
Group:    Applications/Meteo
Requires: %{name}-common = %{?epoch:%epoch:}%{version}-%{release} libwreport3
Obsoletes: libdballe5, libdballe4, libdballepp4 

%description -n libdballe6
DB-ALL.e C shared library
 DB-All.e is a fast on-disk database where meteorological observed and
 forecast data can be stored, searched, retrieved and updated.
 
 This is the shared library for C programs.


%package -n libdballef-devel

Summary:  DB-All.e Fortran development library
Group:    Applications/Meteo
Requires: %{name}-common = %{?epoch:%epoch:}%{version}-%{release} lua-devel cnf-devel libdballef4

%description -n libdballef-devel
 DB-All.e is a fast on-disk database where meteorological observed and
 forecast data can be stored, searched, retrieved and updated.
 .
 This is the DB-All.e Fortran API, designed to make it easy to use the DB-All.e
 database as a smart working area for meteorological software.


%package -n libdballef4

Summary:  DB-ALL.e Fortran shared library
Group:    Applications/Meteo
Requires: %{name}-common = %{?epoch:%epoch:}%{version}-%{release}

%description -n libdballef4
 DB-All.e is a fast on-disk database where meteorological observed and
 forecast data can be stored, searched, retrieved and updated.
 .
 This is the shared library for Fortran programs.


%package common

Summary:  Common data files for all DB-All.e modules
Group:    Applications/Meteo

%description common
Common data files for all DB-All.e modules
 DB-All.e is a fast on-disk database where meteorological observed and
 forecast data can be stored, searched, retrieved and updated.
 .
 This package contains common DB-All.e data files, including variable metadata,
 BUFR and CREX decoding tables, report metadata, level and time range
 descriptions.

%package -n python-dballe
Summary:  DB-ALL.e Python library
Group:    Applications/Meteo
Requires: %{name}-common = %{?epoch:%epoch:}%{version}-%{release}, numpy, python-wreport3
#Requires: rpy,

%description -n python-dballe
 DB-ALL.e Python library for weather research
 DB-All.e is a fast on-disk database where meteorological observed and
 forecast data can be stored, searched, retrieved and updated.

 These are the python bindings.


%prep
%setup -q -n %{name}-%{version}-%{release}

%build

autoreconf -ifv

%configure FC=gfortran F90=gfortan F77=gfortran --enable-dballe-db  --enable-dballef --enable-dballe-python --enable-docs

make

make check

%install
[ "%{buildroot}" != / ] && rm -rf "%{buildroot}"

make install DESTDIR="%{buildroot}" STRIP=/bin/true

%clean
[ "%{buildroot}" != / ] && rm -rf "%{buildroot}"



%files
%defattr(-,root,root,-)
%{_bindir}/dbadb
%{_bindir}/dbamsg
%{_bindir}/dbatbl
%{_bindir}/dbaexport
%doc %{_mandir}/man1/dbadb*
%doc %{_mandir}/man1/dbamsg*
%doc %{_mandir}/man1/dbatbl*
%doc %{_mandir}/man1/dbaexport*
%doc %{_docdir}/dballe/guide.pdf
%doc %{_docdir}/dballe/guide_html/*
%doc %{_docdir}/dballe/fortran_api/*
%doc %{_docdir}/dballe/libdballef.doxytags

%files common
%defattr(-,root,root,-)
%{_datadir}/wreport/[BD]*
%{_datadir}/wreport/dballe.txt
%{_datadir}/wreport/repinfo.csv

%files -n libdballe6
%defattr(-,root,root,-)
%{_libdir}/libdballe.so.*

%files -n libdballe-devel
%defattr(-,root,root,-)
%doc %{_docdir}/dballe/libdballe.doxytags
%{_includedir}/dballe/*.h
%{_includedir}/dballe/core/*
%{_includedir}/dballe/msg/*
%{_includedir}/dballe/db/*
%{_includedir}/dballe/cmdline/*
%{_includedir}/dballe/simple/*
%{_includedir}/dballe/memdb/*

%{_libdir}/libdballe.a
%{_libdir}/libdballe.la
%{_libdir}/libdballe.so
%{_libdir}/pkgconfig/libdballe.pc
%{_datadir}/aclocal/libdballe.m4


%files -n libdballef-devel
%defattr(-,root,root,-)

%doc %{_docdir}/dballe/fapi_html
%doc %{_docdir}/dballe/fapi.pdf

%{_includedir}/dballe/dballef.h
%{_includedir}/dballe/dballeff.h
%{_libdir}/libdballef*.a
%{_libdir}/pkgconfig/libdballef*
%{_libdir}/libdballef*.la
%{_libdir}/libdballef*.so
%{_datadir}/aclocal/libdballef*.m4




%files -n libdballef4
%defattr(-,root,root,-)
%{_libdir}/libdballef*.so.*


%files -n python-dballe
%defattr(-,root,root,-)
%dir %{python_sitelib}/dballe
%{python_sitelib}/dballe/*
%dir %{python_sitearch}
%{python_sitearch}/*.a
%{python_sitearch}/*.la
%{python_sitearch}/*.so*
%{_bindir}/dbatbl_makeb

%doc %{_docdir}/dballe/python-dballe*


%files -n libdballe-doc
%defattr(-,root,root,-)
%doc %{_docdir}/dballe/c_api


%post
/sbin/ldconfig

%postun
/sbin/ldconfig


%changelog
* Wed Mar 30 2016 Dario Mastropasqua,,, <dariomas@users.noreply.github.com> - 7.8-1%{dist} 
- corrected dependencies Require for RHEL (CentOS)

* Tue Nov 24 2015 Daniele Branchini <dbranchini@arpa.emr.it> - 7.7-2%{dist}
- managing provami messy legacy

* Tue Nov 24 2015 Daniele Branchini <dbranchini@arpa.emr.it> - 7.7-1%{dist}
- virtualenv/pip support
- closed #13, #22

* Thu Nov 12 2015 Daniele Branchini <dbranchini@arpa.emr.it> - 7.6-2%{dist}
- Fix dballe and dballe-python dependencies, excluded old provami files

* Wed Sep 16 2015 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 7.6-1%{dist}
- Fix JSON import from stdin (issue #11)

* Wed Sep 16 2015 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 7.5-1%{dist}
- Fixed tests

* Tue Sep 15 2015 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 7.4-1%{dist}
- JSON import/export (issue #5)
- Fix empty report in import (issue #8)
- Fix opening stdin and stdout from Fortran bindings (issue #3)
- Stable CSV header (issue #1)
- Ported to wreport-3.2
- Removed wibble dependency

* Fri Sep  4 2015 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 7.3-2%{dist}
- Fixed test

* Fri Sep  4 2015 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 7.3-1%{dist}
- Encoding parser ignore case

* Mon Aug  3 2015 Daniele Branchini <dbranchini@arpa.emr.it> - 7.2-1%{dist}
- Requires libwreport v3.0
- Switching to git upstream

* Wed Apr 29 2015 Daniele Branchini <dbranchini@arpa.emr.it> - 7.1-4715%{dist}
- using spostgresql-devel and mariadb-devel

* Wed Feb  4 2015 Daniele Branchini <dbranchini@arpa.emr.it> - 6.8-4479%{dist}
- using sqlite-devel instead of unixodbc

* Wed Feb  5 2014 Daniele Branchini <dbranchini@arpa.emr.it> - 6.6-4233%{dist}
- fixed conversion B07007 (M) <-> B07193 (mm).

* Wed Nov 13 2013 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 6.6-4105%{dist}
- Requires libwreport v2.10

* Wed Sep  4 2013 Paolo Patruno <ppatruno@pigna.metarpa> - 6.3-3991%{dist}
-  * New upstream release
     - refactored cursor+querybuilder for v6 databases
     - do not leave SQL_TIMESTAMP_STRUCT.fraction uninitialised, which caused
       duplicate imports in some cases
     - fortran message api: deal gracefully with uninterpretable messaes
     - fixed data overwrite on V6 databases
     - provami: documented the possibility of passing initial key=val filters
       on command line


* Fri Aug 23 2013 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 6.2-3967%{dist}
- Record::set_from_string: hyphen ("-") as MISSING_INT
- Fixes for PostgreSQL (trim char(n) and sequence support)
- B22049 - Sea-surface temperature
- B15236 - [SIM] C6H6 Concentration

* Tue May 28 2013 Paolo Patruno <ppatruno@pigna.metarpa> - 6.1-3884%{dist}
- dballe 6.1

* Wed Aug 29 2012 Daniele Branchini <dbranchini@arpa.emr.it> - 5.22-3684%{dist}
- corretto bug in provami su query

* Tue Jun 12 2012 Daniele Branchini <dbranchini@arpa.emr.it> - 5.19-3633%{dist}
- Aggiornato a revisione 3633

* Tue May  8 2012 Daniele Branchini <dbranchini@arpa.emr.it> - 5.17-3621%{dist}
- Nuova variabile bagnatura fogliare

* Tue Jul 19 2011 root <root@pigna> - 5.7-3390%{dist}
- port to wreport 2.0

* Thu Jul  8 2010 Daniele Branchini <dbranchini@carenza.metarpa> - 4.0.18-2513%{dist}
- Aggiornato alla revisione 2513

* Thu Feb 25 2010 Daniele Branchini <dbranchini@localhost.localdomain> - 4.0.16-2451
- corretta sintassi release di pacchettizzazione

* Tue Nov 11 2008 root <root@strip.metarpa> - 4.0.8-1
- bug resolved about units conversion in import/export and template used in api query message

* Wed Oct 29 2008 root <root@strip.metarpa> - 4.0.7-1
- bug on query best corrected

* Wed Apr  2 2008 root <root@localhost.localdomain> - 4.0.0-8
- added some units conversion

* Tue Mar 18 2008 root <root@spinacio> - 4.0.0-4
- new package (less packages)

* Tue Dec 19 2006 root <root@strip.metarpa> - 3.0-1
- spitted in more packages for version 3.0

* Wed Nov 29 2006 root <root@strip.metarpa> - 2.6-2
- aggiuntevar e rete per icecast

* Wed Nov 22 2006 root <root@strip.metarpa> - 2.6-1
- added support for sqlite

* Wed Aug  9 2006 root <root@strip.metarpa> - 2.3-3
- Aggiornato alla revisione 1086

* Wed Aug  2 2006 root <root@strip.metarpa> - 2.3-1
- some bugs solved

* Tue May  9 2006 root <root@strip.metarpa> - 2.0-1
- cambio delle api e della struttura db per permette la piu' versatile gestione dell'anagrafica

* Wed May  3 2006 root <root@strip.metarpa> - 1.1-1
- ottimizzazioni! eliminato querybest e introdotto parametro quesy con opzioni best e bigana

* Wed Apr 26 2006 root <root@strip.metarpa> - 1.0-5
- modificate query per ottimizzazione

* Tue Apr 11 2006 root <root@strip.metarpa> - 1.0-4
- inserita indicizzazione

* Tue Apr 11 2006 root <root@strip.metarpa> - 1.0-3
- corretti alcni bug

* Wed Mar 15 2006 root <root@strip.metarpa> - 1.0-2
- corrette conversioni mancanti e gestione generici

* Wed Mar  8 2006 root <root@strip.metarpa> - 1.0-1
- prima release ufficiale

* Wed Feb 15 2006 root <root@strip.metarpa> - 0.7-9
-  a lot of bug fixes

* Wed Feb  8 2006 root <root@strip.metarpa> - 0.7-8
- a lot of bug fixes

* Wed Feb  1 2006 root <root@strip.metarpa> - 0.7-7
- resolved performace iusses and metar implemented + aof fixes

* Wed Jan 25 2006 root <root@strip.metarpa> - 0.7-6
- corretti bug

* Wed Jan 18 2006 root <root@strip.metarpa> - 0.7-5
- about source and table bug

* Tue Jan 17 2006 root <root@strip.metarpa> - 0.7-4
- lot of bug corrected and documentation improvements

* Tue Jan 10 2006 root <patruno@strip.metarpa> - 0.7-2
- corretti vari bug di fine anno

* Tue Sep 13 2005 root <root@strip.metarpa> 
- Initial build.


