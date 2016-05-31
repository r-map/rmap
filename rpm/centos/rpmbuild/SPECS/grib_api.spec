# Compile options:
# --with simcdef          : enable simc special support

%{?_with_simcdef:
%define DefName  def_simc
Provides: grib_def
}
%{!?_with_simcdef:
%define DefName def
}

Version:        1.10.0
Name:           grib_api
Release:        11%{dist}
Summary:        ECMWF encoding/decoding GRIB software
License:        Apache License Version 2.0
URL:            http://www.ecmwf.int/products/data/software/grib_api.html
Source0:        http://www.ecmwf.int/products/data/software/download/software_files/%{name}-%{version}.tar.gz
Source1:	utm_grib2.tmpl
Source2:	%{name}-%{version}-20130304def.tar.gz
#Patch0:         %{name}-%{version}-amconditional.diff
#Patch1:         %{name}-%{version}-fortran.diff
#Patch2:         %{name}-%{version}-paths.diff
Patch0:         %{name}-%{version}_sharepath.diff
#Patch1:         %{name}-%{version}_20130304def.diff

%{?_with_simcdef:Patch3: grib_def_all_simc.patch}

BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires:  gcc-gfortran jasper-devel libpng-devel perl bison flex openjpeg-devel python-devel numpy netcdf-devel


%if 0%{?fedora} < 9
%define _fmoddir       %{_libdir}/gfortran/modules
%endif

%{!?python_sitelib: %define python_sitelib %(%{__python} -c "from distutils.sysconfig import get_python_lib; print get_python_lib()")}
%{!?python_sitearch: %define python_sitearch %(%{__python} -c "from distutils.sysconfig import get_python_lib; print get_python_lib(1)")}
%{!?python_siteinc: %define python_siteinc %(%{__python} -c "from distutils.sysconfig import get_python_inc; print get_python_inc()")}

%description
null


%package -n %{name}-%{version}
Requires: %{name}-%{DefName}-%{version} = %{version}
Summary:        ECMWF encoding/decoding GRIB software
Group:          System Environment/Libraries
Obsoletes:	grib_api-1.8


%description -n %{name}-%{version}
Command line tools and library for interactive and batch processing 
of GRIB (GRIdded Binary) data.

%package        devel
Summary:        Development files for %{name}
Group:          Development/Libraries
Requires:       %{name}-%{version} = %{version}-%{release} jasper-devel libpng-devel openjpeg-devel netcdf-devel
Provides:       %{name}-static = %{version}-%{release}

%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.



%package        tools
Summary:        tools for %{name}
Group:          Development/Libraries
Requires:       %{name}-%{version} = %{version}-%{release}

%description    tools
The %{name}-tools package contains %{name} tools.


%package   	%DefName-%{version}
Summary:        ECMWF encoding/decoding GRIB definition
Group:          System Environment/Libraries
Requires: %{name}-%{version} = %{version}
Obsoletes:	grib_def_simc


%description   	%DefName-%{version}
Definition file for processing of GRIB (GRIdded Binary) data.


%package -n python-%{name}
Summary:  grib_api Python library
Group:    Applications/Meteo
Requires: %{name}-%{version} = %{?epoch:%epoch:}%{version}-%{release},numpy

%description -n python-%{name}
Command line tools and library for interactive and batch processing 
of GRIB (GRIdded Binary) data. 
These are the python bindings.

%prep 
%setup -q -n %{name}-%{version}
%patch0 -p0

rm -rf definitions
tar xvf %{SOURCE2}

#%patch1 -p0
#%patch2 -p1
cd definitions
%{?_with_simcdef:%patch3 -p1}
cd ..

autoreconf -if

chmod a-x AUTHORS README INSTALL LICENSE
# remove empty unused Makefile.am
rm html/Makefile.am
find . -name '*.c' -exec chmod -x {} \;
find . -name 'Makefile*' -exec chmod -x {} \;
rm -rf __dist_doc
mkdir __dist_doc
cp -a examples __dist_doc
rm __dist_doc/examples/*/Makefile*
sed -i -e 's/.*GRIB_DEFINITION_PATH.*//' -e 's/.*GRIB_TEMPLATES_PATH.*//' \
  __dist_doc/examples/*/include.sh


%build 

CFLAGS="-fPIC %{optflags}"
export CFLAGS
 
%configure FC=gfortran F77=gfortran  --with-png-support --enable-python --with-netcdf=/usr   --datadir='%{_datadir}/%{name}-%{version}' --with-ifs-samples='%{_datadir}/%{name}-%{version}'

# CFLAGS=$CFLAGS" -fPIC"

# perl module build doesn't works
# configure F77=gfortran FCFLAGS="$RPM_OPT_FLAGS" CFLAGS="-O1 -g -pipe -Wall -fexceptions -Wp,-DFORTIFY_SOURCE=2" --datadir='${datarootdir}'
# CFLAGS="-O2 -g -pipe -Wall -fexceptions -U_FORTIFY_SOURCE" 
# parallel build seems to be broken %{?_smp_mflags}

######    configure FC=gfortran F77=gfortran FCFLAGS="$RPM_OPT_FLAGS" --enable-python --enable-pthread --enable-align-memory --enable-vector --enable-memory-management --enable-omp-packing --with-png-support --datadir='${datarootdir}/%{name}'

make
#make check

%install
rm -rf $RPM_BUILD_ROOT

make install DESTDIR=$RPM_BUILD_ROOT INSTALL='install -p'
#find $RPM_BUILD_ROOT -name '*.la' -exec rm -f {} ';'
mv $RPM_BUILD_ROOT%{_bindir}/parser $RPM_BUILD_ROOT%{_bindir}/grib_parser
mv $RPM_BUILD_ROOT%{_bindir}/points $RPM_BUILD_ROOT%{_bindir}/grib_points
mkdir -p $RPM_BUILD_ROOT%{_fmoddir}
mv $RPM_BUILD_ROOT%{_includedir}/*.mod $RPM_BUILD_ROOT%{_fmoddir}

%{?_with_simcdef:%{__install} %{SOURCE1} $RPM_BUILD_ROOT%{_datadir}/%{name}-%{version}/samples}

# /usr/bin/ksh breaks
sed -e 's?/usr/bin/ksh?/bin/sh?g' $RPM_BUILD_ROOT%{_bindir}/grib1to2 > $RPM_BUILD_ROOT%{_bindir}/grib1to2.tmp
mv $RPM_BUILD_ROOT%{_bindir}/grib1to2.tmp $RPM_BUILD_ROOT%{_bindir}/grib1to2
# make it executable again
chmod +x $RPM_BUILD_ROOT%{_bindir}/grib1to2

#rm -rf  $RPM_BUILD_ROOT%{_datadir}/definitions

# percento {__install} -d  $RPM_BUILD_ROOT%{_datadir}/%{name}
# percento {__install} -d  $RPM_BUILD_ROOT%{_datadir}/%{name}/definitions

cd definitions
find . -name "*.def"| cpio -pdmu -R root:root $RPM_BUILD_ROOT%{_datadir}/%{name}-%{version}/definitions
cd ..

#rm -f $RPM_BUILD_ROOT%{_datadir}/%{name}-%{version}/definitions/Makefile.*
#rm -f $RPM_BUILD_ROOT%{_datadir}/%{name}-%{version}/definitions/installDefinitions.sh
#rm -f $RPM_BUILD_ROOT%{_datadir}/%{name}-%{version}/definitions/dummy.am
#rm -f $RPM_BUILD_ROOT%{_datadir}/%{name}-%{version}/definitions/extrules.am
#rm -f $RPM_BUILD_ROOT%{_datadir}/%{name}-%{version}/definitions/publish_new_parameters.sh

#chmod -R +rX $RPM_BUILD_ROOT%{_datadir}/%{name}-%{version}/


%clean
rm -rf $RPM_BUILD_ROOT


%check
#make check


%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files -n %{name}-%{version}
%defattr(-,root,root,-)
%doc AUTHORS README LICENSE ChangeLog html
%{_libdir}/libgrib_api-%{version}.so
%{_libdir}/libgrib_api_f77-%{version}.so
%{_libdir}/libgrib_api_f90-%{version}.so

%files devel
%defattr(-,root,root,-)
%doc __dist_doc/examples data
%{_includedir}/grib_api.h
%{_includedir}/grib_api_f77.h
%{_fmoddir}/*.mod
%{_libdir}/libgrib_api.la
%{_libdir}/libgrib_api_f77.la
%{_libdir}/libgrib_api_f90.la
%{_libdir}/libgrib_api.a
%{_libdir}/libgrib_api_f77.a
%{_libdir}/libgrib_api_f90.a
%{_libdir}/libgrib_api.so
%{_libdir}/libgrib_api_f77.so
%{_libdir}/libgrib_api_f90.so
%{_libdir}/pkgconfig/grib_api.pc
%{_libdir}/pkgconfig/grib_api_f90.pc


%files tools
%defattr(-,root,root,-)
%{_bindir}/*grib*
%{_bindir}/gg_sub_area_check
%{_bindir}/tigge_*

%files %DefName-%{version}
%{_datadir}/%{name}-%{version}/


%files -n python-%{name}
%defattr(-,root,root,-)
%dir %{python_sitearch}/grib_api
%{python_sitearch}/grib_api/*


%changelog
* Thu Mar  7 2013 Paolo Patruno <ppatruno@pigna.metarpa> - 1.10.0-8%{dist}
- removed some patch to def and added timerange 13

* Wed Feb 20 2013 Paolo Patruno <ppatruno@pigna.metarpa> - 1.10.0-1%{dist}
- 1.10.0 mainstream version; removed build patch

* Thu Mar 22 2012 dbranchini <dbranchini@arpa.emr.it> - 1.9.9-13%{dist}
- ripristinato python-grib_api

* Wed Feb  9 2011 root <root@pigna> - 1.9.5-8%{dist}
- added local.200.254.def

* Tue Jun 22 2010 root <cesari@arpa.emr.it> - 1.9.0-4
- New version with more GunPipes

* Fri Nov 27 2009 root <cesari@arpa.emr.it> - 1.8.0-6
- patch for grib1to2 script

* Wed Oct 14 2009 root <p.patruno@iperbole.bologna.it> - 1.8.0-4
- patch for shared library, update to 1.8.0, package split

* Wed Dec  3 2008 Patrice Dumas <pertusus@free.fr> 1.6.4-1
- update to 1.6.4

* Tue Sep 30 2008 Patrice Dumas <pertusus@free.fr> 1.6.1-1
- update to 1.6.1

* Sat Feb 23 2008 Patrice Dumas <pertusus@free.fr> 1.4.0-1
- update to 1.4.0

* Sat Dec 29 2007 Patrice Dumas <pertusus@free.fr> 1.3.0-1
- initial release
