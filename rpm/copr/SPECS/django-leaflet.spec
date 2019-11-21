%define srcname django-leaflet
%define version 0.26.0.dev0
%define release 2

Summary: Use Leaflet in your django projects
Name: python-%{srcname}
Version: %{version}
Release: %{release}
Source0: %{srcname}-%{version}.tar.gz
License: LPGL, see LICENSE file.
Group: Development/Libraries
BuildArch: noarch
Vendor: Mathieu Leplatre <mathieu.leplatre@makina-corpus.com>
Url: https://github.com/makinacorpus/django-leaflet
BuildRequires: python2-devel python3-devel

%description
django-leaflet allows you to use Leaflet in your Django projects.

It embeds Leaflet version *1.0.3*.

Main purposes of having a python package for the Leaflet Javascript library :

* Install and enjoy ;
* Do not embed Leaflet assets in every Django project ;
* Enjoy geometry edition with Leaflet form widget ;
* Control apparence and settings of maps from Django settings (e.g. at deployment) ;
* Reuse Leaflet map initialization code (e.g. local projections) ;

:note:

    *django-leaflet* is compatible with `django-geojson <https://github.com/makinacorpus/django-geojson.git>`_ fields, which
    allow handling geographic data without spatial database.

=========
TUTORIALS
=========

* `GeoDjango maps with Leaflet <http://blog.mathieu-leplatre.info/geodjango-maps-with-leaflet.html>`_

%package -n python3-%{srcname}
Summary:        %{summary}
%{?python_provide:%python_provide python3-%{srcname}}

%description -n python3-%{srcname}
django-leaflet allows you to use Leaflet in your Django projects.

It embeds Leaflet version *1.0.3*.

Main purposes of having a python package for the Leaflet Javascript library :

* Install and enjoy ;
* Do not embed Leaflet assets in every Django project ;
* Enjoy geometry edition with Leaflet form widget ;
* Control apparence and settings of maps from Django settings (e.g. at deployment) ;
* Reuse Leaflet map initialization code (e.g. local projections) ;

:note:

    *django-leaflet* is compatible with `django-geojson <https://github.com/makinacorpus/django-geojson.git>`_ fields, which
    allow handling geographic data without spatial database.

=========
TUTORIALS
=========

* `GeoDjango maps with Leaflet <http://blog.mathieu-leplatre.info/geodjango-maps-with-leaflet.html>`_


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

