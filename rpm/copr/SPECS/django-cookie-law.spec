%define srcname django-cookie-law
%define version 1.0.13
%define release 2

Summary: Helps your Django project comply with EU cookie law regulations
Name: %{srcname}
Version: %{version}
Release: %{release}
Source0: %{srcname}-%{version}.tar.gz
License: BSD License
Group: Development/Libraries
BuildArch: noarch
Vendor: Piotr Kilczuk <piotr@tymaszweb.pl>
Url: https://github.com/TyMaszWeb/django-cookie-law
BuildRequires: python2-devel
BuildRequires: python3-devel



%description
django-cookie-law helps your Django project comply with the
EU cookie regulations <http://www.aboutcookies.org/default.aspx?page=3
by displaying a cookie information banner until it is dismissed by the user.

 warning: The app can be incompatible with your local cookie
             law regulations. Consult your lawyer when in doubt.

 Contributions and comments are welcome using Github at:
 http://github.com/TyMaszWeb/django-cookie-law

 Please note that django-cookie-law requires:

- Django >= 1.2
- django-classy-tags >= 0.3.0

%package -n python3-%{srcname}
Summary:        %{summary}
%{?python_provide:%python_provide python3-%{srcname}}

%description -n python3-%{srcname}
django-cookie-law helps your Django project comply with the
EU cookie regulations <http://www.aboutcookies.org/default.aspx?page=3
by displaying a cookie information banner until it is dismissed by the user.

 warning: The app can be incompatible with your local cookie
             law regulations. Consult your lawyer when in doubt.

 Contributions and comments are welcome using Github at:
 http://github.com/TyMaszWeb/django-cookie-law

 Please note that django-cookie-law requires:

- Django >= 1.2
- django-classy-tags >= 0.3.0

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
%license LICENSE.rst
%doc README.rst
%{python2_sitelib}/*

%files -n python3-%{srcname}
%license LICENSE.rst
%doc README.rst
%{python3_sitelib}/*
#%{_bindir}/*

