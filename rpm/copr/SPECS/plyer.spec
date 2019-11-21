%define srcname plyer
%define version 1.3.2.dev0
%define release 2

Summary: Platform-independent wrapper for platform-dependent APIs
Name: %{srcname}
Version: %{version}
Release: %{release}
Source0: %{srcname}-%{version}.tar.gz
License: UNKNOWN
Group: Development/Libraries
BuildArch: noarch
Vendor: Kivy team <mat@kivy.org>
Url: https://plyer.readthedocs.org/en/latest/
BuildRequires: python2-devel python3-devel

%description
Plyer is a platform-independent api to use features commonly found on various
platforms, notably mobile ones, in Python.

%package -n python3-%{srcname}
Summary:        %{summary}
%{?python_provide:%python_provide python3-%{srcname}}

%description -n python3-%{srcname}
Plyer is a platform-independent api to use features commonly found on various
platforms, notably mobile ones, in Python.

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

