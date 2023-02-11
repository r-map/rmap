%define srcname django-borinud
%define version 1.0
%define release 1

Summary: django web services to provide weather data
Name: %{srcname}
Version: %{version}
Release: %{release}
Source0: %{srcname}-%{version}.tar.gz
License: GPL2+
Group: Development/Libraries
BuildArch: noarch
Vendor: Paolo Patruno <p.patruno@iperbole.bologna.it>
Url: https://github.com/r-map/rmap
BuildRequires: python3-devel python3-setuptools

%description
 Django web services to provide weather data.

 Requires:

- Django >= 2.0

%package -n python3-%{srcname}
Summary:        %{summary}
%{?python_provide:%python_provide python3-%{srcname}}

%description -n python3-%{srcname}
 Django web services to provide weather data.

 Requires:

- Django >= 2.0

%prep
%autosetup -n %{srcname}-%{version}

%build
%py3_build

%install
%py3_install

#%check
#%{__python3} setup.py test


%files -n %{name}
%license LICENSE
%doc README.rst
%{python3_sitelib}/*

