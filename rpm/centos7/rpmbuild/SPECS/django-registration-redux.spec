%define name django-registration-redux
%define version 1.2
%define unmangled_version 1.2
%define unmangled_version 1.2
%define release 1

Summary: An extensible user-registration application for Django
Name: %{name}
Version: %{version}
Release: %{release}
Source0: %{name}-%{unmangled_version}.tar.gz
License: UNKNOWN
Group: Development/Libraries
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
Prefix: %{_prefix}
BuildArch: noarch
Vendor: Andrew Cutler <macropin@gmail.com>
Url: https://github.com/macropin/django-registration

%description
.. -*-restructuredtext-*-

.. image:: https://travis-ci.org/macropin/django-registration.png?branch=master
    :target: https://travis-ci.org/macropin/django-registration

.. image:: https://coveralls.io/repos/macropin/django-registration/badge.png?branch=master
    :target: https://coveralls.io/r/macropin/django-registration/

.. image:: https://badge.fury.io/py/django-registration-redux.svg
    :target: https://pypi.python.org/pypi/django-registration-redux/

.. image:: https://pypip.in/download/django-registration-redux/badge.svg
    :target: https://pypi.python.org/pypi/django-registration-redux/

If you have issues with the "django-registration-redux" package then please `raise them here`_.

This is a fairly simple user-registration application for Django,
designed to make allowing user signups as painless as possible. It
requires a functional installation of Django 1.4 or newer, but has no
other dependencies.

For installation instructions, see the file "INSTALL" in this
directory; for instructions on how to use this application, and on
what it provides, see the file "quickstart.rst" in the "docs/"
directory. Full documentation is also `available online`_

.. _`available online`: https://django-registration-redux.readthedocs.org/
.. _`raise them here`: https://github.com/macropin/django-registration/issues

%prep
%setup -n %{name}-%{unmangled_version} -n %{name}-%{unmangled_version}

%build
python setup.py build

%install
python setup.py install --single-version-externally-managed -O1 --root=$RPM_BUILD_ROOT --record=INSTALLED_FILES

%clean
rm -rf $RPM_BUILD_ROOT

%files -f INSTALLED_FILES
%defattr(-,root,root)
