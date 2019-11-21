%define srcname nominatim
%define version 0.3
%define release 3

Summary: OSM Nominatim API module
Name: python-%{srcname}
Version: %{version}
Release: %{release}
Source0: %{srcname}-%{version}.tar.gz
#Patch0: nominatim-python3.patch

License: UNKNOWN
Group: Development/Libraries
BuildArch: noarch
Vendor: Damian Braun <brunek5252@gmail.com>
Url: https://github.com/damianbraun/nominatim
BuildRequires: python2-devel
BuildRequires: python3-devel

%description
OSM Nominatim API module for python

%package -n python3-%{srcname}
Summary:        %{summary}
%{?python_provide:%python_provide python3-%{srcname}}

%description -n python3-%{srcname}
OSM Nominatim API module for python

%prep
%autosetup -n %{srcname}-%{version}


%build
%py2_build
%py3_build

%install
%py2_install
%py3_install

%check
%{__python2} setup.py test
%{__python3} setup.py test


%files -n %{name}
#%license LICENSE
#%doc README.rst
%{python2_sitelib}/*

%files -n python3-%{srcname}
#%license LICENSE
#%doc README.rst
%{python3_sitelib}/*

