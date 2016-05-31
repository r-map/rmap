Name:          cnf
Version:       4.0
Release:       2%{?dist}
Summary:       C and Fortran mixed language portable interface subroutines
Group:         Development
Vendor:        starlink
Distribution:  sim
Packager:      Paolo Patruno,,, <ppatruno@arpa.emr.it>
Source:        cnf-%{version}.tar.gz
License:       G.P.L.
BuildRoot:     %{_tmppath}/%{name}-%{version}-root

%description
The CNF package comprises two sets of software which ease the task of writing portable programs in a mixture of FORTRAN and C. F77 is a set of C macros for handling the FORTRAN/C subroutine linkage in a portable way, and CNF is a set of functions to handle the difference between FORTRAN and C character strings, logical values and pointers to dynamically allocated memory.

%package  devel
Summary:  C and Fortran mixed language portable interface subroutines - development library
Group:    Development
#Requires: %{name} = %{?epoch:%epoch:}%{version}-%{release}

%description devel
The CNF package comprises two sets of software which ease the task of writing portable programs in a mixture of FORTRAN and C. F77 is a set of C macros for handling the FORTRAN/C subroutine linkage in a portable way, and CNF is a set of functions to handle the difference between FORTRAN and C character strings, logical values and pointers to dynamically allocated memory.

development library

%prep
%setup -q

%build
%ifarch x86_64
sed -e 's?\$(prefix)/lib?$(prefix)/lib64?g' Makefile > Makefile1
mv -f Makefile1 Makefile
%endif

%install
[ "%{buildroot}" != / ] && rm -rf "%{buildroot}"
%makeinstall

%clean
[ "%{buildroot}" != / ] && rm -rf "%{buildroot}"

%files devel
%defattr(-,root,root)
%doc README 
%{_includedir}/*.h
%{_libdir}/*.a
%doc %{_docdir}/%{name}/*
  

%changelog
* Tue Aug  7 2007 root <dcesari@arpa.emr.itg> - 4.0-2
- Added support for 64 bit build

* Tue Jan 02 2007 Paolo Patruno,,, <ppatruno@arpa.emr.it> 4.0-1
- package created by autospec
