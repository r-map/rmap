%define srcname django-classy-tags
%define version 0.8.0
%define release 3

Summary: Class based template tags for Django
Name: python-%{srcname}
Version: %{version}
Release: %{release}
Source0: %{srcname}-%{version}.tar.gz
License: UNKNOWN
Group: Development/Libraries
BuildArch: noarch
Vendor: Jonas Obrist <ojiidotch@gmail.com>
Url: http://github.com/ojii/django-classy-tags
BuildRequires: python2-devel python3-devel

%description
UNKNOWN

%package -n python3-%{srcname}
Summary:        %{summary}
#%{?python_provide:%python_provide python3-%{srcname}}

%description -n python3-%{srcname}
UNKNOWN

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
#%license LICENSE
#%doc README.rst
%{python2_sitelib}/*

%files -n python3-%{srcname}
#%license LICENSE
#%doc README.rst
%{python3_sitelib}/*

