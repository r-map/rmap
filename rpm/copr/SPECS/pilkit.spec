%define srcname pilkit
%define version 1.1.13
%define release 3

Summary: A collection of utilities and processors for the Python Imaging Libary.
Name: python-%{srcname}
Version: %{version}
Release: %{release}
Source0: %{srcname}-%{version}.tar.gz
License: BSD
Group: Development/Libraries
BuildArch: noarch
Vendor: Matthew Tretter <m@tthewwithanm.com>
Url: http://github.com/matthewwithanm/pilkit/
BuildRequires: python2-devel python3-devel
Requires: python2-pillow

%description
PILKit is a collection of utilities for working with PIL (the Python Imaging
Library).

One of its main features is a set of **processors** which expose a simple
interface for performing manipulations on PIL images.

Looking for more advanced processors? Check out `Instakit`_!

**For the complete documentation on the latest stable version of PILKit, see**
`PILKit on RTD`_.
.. _`PILKit on RTD`: http://pilkit.readthedocs.org
.. _`Instakit`: https://github.com/fish2000/instakit



%package -n python3-%{srcname}
Summary:        %{summary}
%{?python_provide:%python_provide python3-%{srcname}}
Requires: python3-pillow

%description -n python3-%{srcname}
PILKit is a collection of utilities for working with PIL (the Python Imaging
Library).

One of its main features is a set of **processors** which expose a simple
interface for performing manipulations on PIL images.

Looking for more advanced processors? Check out `Instakit`_!

**For the complete documentation on the latest stable version of PILKit, see**
`PILKit on RTD`_.
.. _`PILKit on RTD`: http://pilkit.readthedocs.org
.. _`Instakit`: https://github.com/fish2000/instakit

%prep
%autosetup -n %{srcname}-%{version}

%build
%py2_build
%py3_build

%install
%py2_install
%py3_install

#%check
#%{__python2} setup.py test
#%{__python3} setup.py test


%files -n %{name}
%license LICENSE
%doc README.rst
%{python2_sitelib}/*

%files -n python3-%{srcname}
%license LICENSE
%doc README.rst
%{python3_sitelib}/*
#%{_bindir}/*

