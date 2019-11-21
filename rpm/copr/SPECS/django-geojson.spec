%define srcname django-geojson
%define version 2.12.0.dev0
%define release 6

Summary: Serve vectorial map layers with Django
Name: python-%{srcname}
Version: %{version}
Release: %{release}
Source0: %{srcname}-%{version}.tar.gz
License: LPGL, see LICENSE file.
Group: Development/Libraries
BuildArch: noarch
Vendor: Mathieu Leplatre <mathieu.leplatre@makina-corpus.com>
Url: https://github.com/makinacorpus/django-geojson
BuildRequires: python2-devel python3-devel

%description
See the `documentation <https://django-geojson.readthedocs.io/en/latest/>`_ for more information.

*django-geojson* is a set of tools to manipulate GeoJSON with Django :

* (De)Serializer for (Geo)Django objects, querysets and lists
* Base views to serve GeoJSON map layers from models
* GeoJSON model and form fields to avoid spatial database backends
  (compatible with *django-leaflet* for map widgets)


%package -n python3-%{srcname}
Summary:        %{summary}
%{?python_provide:%python_provide python3-%{srcname}}
Requires: python3-django-jsonfield python3-django-leaflet

%description -n python3-%{srcname}
See the `documentation <https://django-geojson.readthedocs.io/en/latest/>`_ for more information.

*django-geojson* is a set of tools to manipulate GeoJSON with Django :

* (De)Serializer for (Geo)Django objects, querysets and lists
* Base views to serve GeoJSON map layers from models
* GeoJSON model and form fields to avoid spatial database backends
  (compatible with *django-leaflet* for map widgets)

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
%doc README.rst
%{python2_sitelib}/*

%files -n python3-%{srcname}
#%license LICENSE
%doc README.rst
%{python3_sitelib}/*
#%{_bindir}/*



