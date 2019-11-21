%define srcname django-cors-headers
%define version 3.2.0
%define release 1

Summary: django-cors-headers is a Django application for handling the server headers required for Cross-Origin Resource Sharing (CORS).
Name: python3-%{srcname}
Version: %{version}
Release: %{release}
Source0: %{srcname}-%{version}.tar.gz
License: MIT License
Group: Development/Libraries
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
Prefix: %{_prefix}
BuildArch: noarch
Vendor: Adam Johnson <me@adamj.eu>
Url: https://github.com/adamchainz/django-cors-headers
BuildRequires: python3-devel python3-pytest

%description
django-cors-headers
===================

A Django App that adds Cross-Origin Resource Sharing (CORS) headers to
responses. This allows in-browser requests to your Django application from
other origins.

About CORS
----------

Adding CORS headers allows your resources to be accessed on other domains. It's
important you understand the implications before adding the headers, since you
could be unintentionally opening up your site's private data to others.

Some good resources to read on the subject are:

* The `Wikipedia Page <https://en.m.wikipedia.org/wiki/Cross-origin_resource_sharing>`_
* The `MDN Article <https://developer.mozilla.org/en-US/docs/Web/HTTP/CORS>`_
* The `HTML5 Rocks Tutorial <https://www.html5rocks.com/en/tutorials/cors/>`_

Requirements
------------

Python 3.5-3.8 supported.
Django 1.11-3.0 suppported.


%prep
%autosetup -n %{srcname}-%{version}

%build
%py3_build

%install
%py3_install

%check
#%{__python3} setup.py test


%files -n %{name}
#%license LICENSE
#%doc README.rst
%{python3_sitelib}/*

