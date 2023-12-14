
%define srcname django-hijack
%define version 3.4.2
%define release 2

Summary: With Django Hijack, admins can log in and work on behalf of other users without having to know their credentials.
Name: python3-%{srcname}
Version: %{version}
Release: %{release}
Source0: %{srcname}-%{version}.tar.gz
License: MIT
Group: Development/Libraries
BuildArch: noarch
Vendor: arteria GmbH <admin@arteria.ch>
Url: https://github.com/django-hijack/django-hijack

BuildRequires: python3-devel python3-setuptools gettext npm

%description
With Django Hijack, admins can log in and work on behalf of other users without having to know their credentials.

%prep
%autosetup -n %{srcname}-%{version}

%build
%py3_build

%install
%py3_install

#%check
#%{__python3} setup.py test


%files -n %{name}
#%license LICENSE
%doc README.md
%{python3_sitelib}/*
#%config(noreplace) %{_sysconfdir}/%{srcname}/*
#%{_bindir}/*
#%{_datadir}/%{srcname}/
