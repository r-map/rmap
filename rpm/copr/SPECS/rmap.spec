%define srcname rmap
%define version 16.1
%define release 2

Summary: rete monitoraggio ambientale partecipativo
Name: python3-%{srcname}
Version: %{version}
Release: %{release}
Source0: %{srcname}-%{version}.tar.gz
License: GNU GPL v2
Group: Development/Libraries
BuildArch: noarch
Vendor: Paolo Patruno <p.patruno@iperbole.bologna.it>
Url: https://github.com/r-map/rmap
BuildRequires: python3-devel (python3-django < 3.0.0) python3-configobj
BuildRequires: python3-django-imagekit python3-django-geojson
BuildRequires: python3-django-tagging gettext python3-django-appconf
BuildRequires: help2man python3-numpy python3-paho-mqtt
Requires: (python3-django < 3.0.0) python3-configobj
Requires: python3-django-imagekit python3-django-geojson
Requires: python3-django-tagging python3-django-hosts
Requires: python3-django-cookie-law python3-django-extensions
Requires: python3-dballe python3-gdal  python3-requests
Requires: gettext python3-django-appconf python3-paho-mqtt
Requires: python3-cairocffi python3-pyparsing python3-django-cors-headers
Requires: python3-sslpsk python3-django-hijack
#Requires:corsheaders

%description
RMAP: participative environmental monitoring net.


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
%doc README.rst
%{python3_sitelib}/*
%config(noreplace) %{_sysconfdir}/rmap/*
%{_bindir}/*
%{_datadir}/rmap/
%{_mandir}/man1/*
