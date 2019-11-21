%define name rmap
%define version 6.14
%define unmangled_version 6.14
%define release 2

Summary: rete monitoraggio ambientale partecipativo
Name: %{name}
Version: %{version}
Release: %{release}
Source0: %{name}-%{unmangled_version}.tar.gz
License: GNU GPL v2
Group: Development/Libraries
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
Prefix: %{_prefix}
BuildArch: noarch
Vendor: Paolo Patruno <p.patruno@iperbole.bologna.it>
Url: https://github.com/r-map/rmap
Requires: python-django python-pika django-leaflet jsonfield django-geojson pilkit django-imagekit python-django-appconf nominatim
Requires: dballe

%description
R-map: participative environmental monitoring net.


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
