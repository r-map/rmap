%define short_name pika

Name:           python-%{short_name}
Version:        0.9.14
Release:        1%{?dist}
Summary:        AMQP 0-9-1 client library for Python

Group:          Development/Libraries
License:        MPLv1.1 or GPLv2
URL:            http://github.com/%{short_name}/%{short_name}
# The tarball comes from here:
# http://github.com/%{short_name}/%{short_name}/tarball/v%{version}
# GitHub has layers of redirection and renames that make this a troublesome
# URL to include directly.
Source0:        %{short_name}-%{version}.tar.gz
#Patch0:         blocking_connection.patch
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildArch:      noarch

BuildRequires:  python-setuptools
BuildRequires:  python-devel
Requires:       python

%description
Pika is a pure-Python implementation of the AMQP 0-9-1 protocol that
tries to stay fairly independent of the underlying network support
library.


%prep
%setup -q -n %{short_name}-%{version}
#%patch0 -p0


%build
%{__python} setup.py build

%install
%{__rm} -rf %{buildroot}
%{__python} setup.py install -O1 --skip-build --root %{buildroot}


%clean
%{__rm} -rf %{buildroot}


%files
%defattr(-,root,root,-)
%dir %{python_sitelib}/%{short_name}*
%{python_sitelib}/%{short_name}*/*

%changelog
* Thu Feb 14 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.9.5-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_19_Mass_Rebuild

* Sat Jul 21 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.9.5-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_18_Mass_Rebuild

* Sun May 13 2012 Ilia Cheishvili <ilia.cheishvili@gmail.com> - 0.9.5-3
- Bump pika release version to fix upgrade path for f17 -> f18

* Sun Feb 26 2012 Daniel Aharon <dan@danielaharon.com> - 0.9.5-2
- Patch pika/adapters/blocking_connection.py

* Sat Jan 14 2012 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.9.5-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_17_Mass_Rebuild

* Sun Apr 3 2011 Ilia Cheishvili <ilia.cheishvili@gmail.com> - 0.9.5-1
- Upgrade to version 0.9.5

* Sun Mar 6 2011 Ilia Cheishvili <ilia.cheishvili@gmail.com> - 0.9.4-1
- Upgrade to version 0.9.4

* Sat Feb 19 2011 Ilia Cheishvili <ilia.cheishvili@gmail.com> - 0.9.3-1
- Upgrade to version 0.9.3

* Sat Oct 2 2010 Ilia Cheishvili <ilia.cheishvili@gmail.com> - 0.5.2-1
- Initial Package

