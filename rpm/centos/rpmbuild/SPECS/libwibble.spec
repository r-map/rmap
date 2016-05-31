Summary: Library of various useful C++ code
Name: libwibble
Version: 1.1
Release: 2%{dist}
License: BSD
Group: Development/Libraries
URL: http://packages.qa.debian.org/libwibble
Source0: %{name}-%{version}.tar.gz
BuildRequires: cmake, doxygen, libtool

%prep
%setup -q -n %{name}-%{version}

%build
%cmake .
make %{?_smp_mflags}
make doc

%check
make unit

%install
[ "%{buildroot}" != / ] && rm -rf "%{buildroot}"
make install DESTDIR="%{buildroot}"

%clean
[ "%{buildroot}" != / ] && rm -rf "%{buildroot}"

%description
 libwibble collects the foundation code that has been used over time
 in various C++ projects by Peter Rockai and Enrico Zini, so that it can
 be maintained properly and in a single place.
 
 The library contains:
  * an exception hierarchy;
  * various useful mixin classes;
  * shortcuts for set operations;
  * a commandline parser that supports cvs-style subcommands;
  * improved macros for tut unit testing;
  * a non-intrusive polimorphic envelope.

%package devel
Summary: Library of various useful C++ code - development files

%description devel
 libwibble collects the foundation code that has been used over time
 in various C++ projects by Peter Rockai and Enrico Zini, so that it can
 be maintained properly and in a single place.
 
 The library contains:
  * an exception hierarchy;
  * various useful mixin classes;
  * shortcuts for set operations;
  * a commandline parser that supports cvs-style subcommands;
  * improved macros for tut unit testing;
  * a non-intrusive polimorphic envelope.

%files devel
%defattr(-,root,root,-)
%{_bindir}/*
%{_libdir}/*
%{_includedir}/*
%{_libdir}/pkgconfig/libwibble*
%{_datadir}/aclocal/libwibble*.m4
%{_datadir}/*
%{_mandir}/*

%package doc
Summary: Library of various useful C++ code - documentation

%description doc
 libwibble collects the foundation code that has been used over time
 in various C++ projects by Peter Rockai and Enrico Zini, so that it can
 be maintained properly and in a single place.
 
 The library contains:
  * an exception hierarchy;
  * various useful mixin classes;
  * shortcuts for set operations;
  * a commandline parser that supports cvs-style subcommands;
  * improved macros for tut unit testing;
  * a non-intrusive polimorphic envelope.

%files doc
%doc doc/html/*


%changelog
* Wed Nov 13 2013 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 1.1-2
- Release 1.1

* Tue Sep 28 2010 root <ppatruno@arpa.emr.it> - 0.1.26-1
- release  0.1.26 for Fedora 12

* Wed Sep 19 2007 Enrico Zini <enrico@enricozini.org>
- Initial build.
