Summary: libsim: librerie di utilità in Fortran 90
Name: libsim
Version: 6.1.5
Release: 1
License: GPL2+
Group: Applications/Meteo
URL: https://github.com/arpa-simc/%{name}
Packager: Davide Cesari <dcesari@arpa.emr.it>
#Source: %{name}-%{version}.tar.gz
Source: https://github.com/arpa-simc/%{name}/archive/v%{version}-%{release}.tar.gz#/%{name}-%{version}-%{release}.tar.gz  
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
BuildRequires: fortrangis-devel libdballef-devel >= 7.6 grib_api-devel ncl-devel gdal-devel libdballe-devel help2man log4c log4c-devel
BuildRequires: cnf-devel libpng-devel fortrangis netcdf-fortran-devel shapelib-devel jasper-devel proj-devel popt-devel openjpeg-devel cairo-devel
Requires: libdballef4 >= 7.6 grib_api-1.10.0

#temporaneo
%if 0%{?fedora} < 9
%define _fmoddir       %{_libdir}/gfortran/modules
%endif

%package -n libsim-devel
Summary:  libsim development files
Group: Applications/Meteo

%package -n libsim-doc
Summary:  libsim documentation
Group: Applications/Meteo

%description -n libsim-devel
Libsim is a collection of Fortran libraries and command-line tools.

This package contains the development files, necessary for building
applications using libsim.

%description -n libsim-doc
Libsim is a collection of Fortran libraries and command-line tools.

This package contains the doxygen documentation for libsim.

%description
Libsim is a collection of Fortran libraries and command-line tools.

The libraries include a general purpose ''base'' library with modules
for handling character variables, csv files, command-line arguments,
physical constants, date and time computations, georeferenced
coordinates, growable arrays and list structures of any type and
other.

Another set of libraries is specific to Meteorology and Earth Science
and allows to work with gridded and sparse georeferenced data, perform
interpolations, statistical processing in time, data quality control,
thermodynamic computations.

The ready-to-use command-line tools allow to perform many kinds of
space interpolations and time computations on georeferenced data in
GRIB and BUFR format.

%prep
%setup -q -n %{name}-%{version}-%{release}
sh autogen.sh

%build

%configure FCFLAGS="%{optflags} -I%{_fmoddir}"  --enable-f2003-features  --enable-alchimia --enable-shapelib --enable-netcdf --enable-gribapi --enable-gdal --enable-f2003-extended-features --disable-oraclesim

make 

%install
make DESTDIR=%{buildroot} install
%if 0%{?fedora} >= 9
mkdir -p $RPM_BUILD_ROOT%{_fmoddir}
mv $RPM_BUILD_ROOT%{_includedir}/*.mod $RPM_BUILD_ROOT%{_fmoddir}
%endif

%files
%defattr(-,root,root)
%{_libdir}/*.so.*
%{_bindir}/*
%{_datadir}/%{name}/*
%{_mandir}/man1

%files -n libsim-devel
%defattr(-,root,root)
%{_libdir}/*.a
%{_libdir}/*.la
%{_libdir}/*.so
%{_libdir}/pkgconfig/%{name}.pc
%if 0%{?fedora} < 9
%{_includedir}/*
%else
%{_fmoddir}/*.mod
%endif

%files -n libsim-doc
%defattr(-,root,root)
%doc examples/*.f90
%doc %{_docdir}/%{name}/html

%clean
rm -rf %{buildroot}

%changelog
* Wed Mar 30 2016 Dario Mastropasqua,,, <dariomas@users.noreply.github.com> - 6.1.5-1%{dist}
- corrected dependencies Require for RHEL (CentOS)

* Mon Feb 22 2016 dcesari <dcesari@arpa.emr.it> - 6.1.5-1%{dist}
- prefer seconds for timedelta to avoid problems with long timeranges
- timedelta can be initialised in sec as well
- fix bitmap key in grib2
- handle better missing values in vertical levels
- correct order for B14018
- handle correctly stdin and stdout in dballe_class
- implemented n-1 variant of variance and stddev, variance guaranteed >=0

* Mon Jan 18 2016 dcesari <dcesari@arpa.emr.it> - 6.1.4-1%{dist}
- devel package
- stddev subtype in stencilinter type

* Wed Dec 2 2015 dbranchini <dbranchini@arpa.emr.it> - 6.1.3-1%{dist}
- no querybest in ana
- sort level and timerange after rounding, fix timerange 254 in rounding

* Mon Nov 23 2015 dbranchini <dbranchini@arpa.emr.it> - 6.1.2-1%{dist}
- fix bug on endofftimeinterval, avoid timerange 206 which is now valid

* Tue Oct 27 2015 dbranchini <dbranchini@arpa.emr.it> - 6.1.1-1%{dist}
- fixed dba_qcfilter bug

* Thu Oct 1 2015 dbranchini <dbranchini@arpa.emr.it> - 6.1.0-1505%{dist}
- implemented heap sort for all vol7d objects
- stdin/stdout in dballe fortran api change
- fix for grib2 output on Lamber conformal
- use qc without data_id
- new function in termo for specific humidity
- fix in termo for missing values when scaled values

* Thu Feb 12 2015 dbranchini <dbranchini@arpa.emr.it> - 6.0.1-1462%{dist}
- Fixed too short dns buffer
- Fixed not initialized list in v7d2dba and other not initialized var in vol7d_dballe_init

* Wed Jan 16 2013 dcesari <dcesari@arpa.emr.it> - 5.0.0-1215%{dist}
- New version requiring latest vapor version

* Thu May 31 2012 dbranchini <dbranchini@arpa.emr.it> - 4.5.0-1128%{dist}
- aggiunta gestione (correzione) campi cumulati ecmwf

* Mon May 14 2012 dbranchini <dbranchini@arpa.emr.it> - 4.5.0-1121%{dist}
- modifiche per variabili leaf wetness e evapotranspiration.

* Tue Mar 17 2009 root <root@strip.metarpa> - 2.6.7-1
- cambiato il mone degli eseguibili con prefix vg6d_

* Mon Jul  7 2008 Davide Cesari <cesari@malina.metarpa> - 2.2-1
- New version, -I/usr/include now as FCFLAGS, ORA_VER added

* Wed Apr 16 2008 Davide Cesari <cesari@malina.metarpa> - 2.1-1
- New version

* Fri Jan 18 2008 Davide Cesari <cesari@malina.metarpa> - 1.4-2
- Add search path for oracle libraries.

* Thu Jan 17 2008 Davide Cesari <cesari@malina.metarpa> - 1.4-1
- New version.

* Mon Dec 10 2007 Davide Cesari <cesari@malina.metarpa> - 1.3-2
- Fixed makeinstall for Fedora 8.

* Thu Nov 22 2007 Davide Cesari <cesari@malina.metarpa> - 1.2-1
- Nuova versione

* Fri Jun 15 2007 Davide Cesari <cesari@malina.metarpa> - 1.1-1
- Prima pacchettizzazione su Fedora Core 4

