# Note 1: EPEL7 packaging is not yet possible due missing
# python-django-formtools in EPEL7 stable repository.
# See https://bugzilla.redhat.com/show_bug.cgi?id=1387376

%global pypi_name jsonfield

%global with_python2 0

Name:           python-django-%{pypi_name}
Version:        2.0.2
Release:        9%{?dist}
Summary:        A reusable Django field that allows you to store validated JSON in your model

License:        BSD
URL:            https://github.com/bradjasper/django-jsonfield
Source0:        https://files.pythonhosted.org/packages/source/%(n=%{pypi_name}; echo ${n:0:1})/%{pypi_name}/%{pypi_name}-%{version}.tar.gz

BuildArch:      noarch

%if 0%{?with_python2}
BuildRequires:  python2-devel
BuildRequires:  python2-django
BuildRequires:  python2-setuptools
BuildRequires:  python2-django-formtools
%endif

# for testing purposes
BuildRequires:  sqlite
BuildRequires:  python3-django
BuildRequires:  python3-devel
BuildRequires:  python3-setuptools
BuildRequires:  python3-django-formtools

%description
django-jsonfield is a reusable Django field that allows you to store validated
JSON in your model. It silently takes care of serialization. To use, simply
add the field to one of your models.

%if 0%{?with_python2}
%package -n python2-django-%{pypi_name}
Summary:        %{summary}
%{?python_provide:%python_provide python2-%{pypi_name}}
Requires:  python2-django
Requires:  python2-django-formtools
%description -n python2-django-%{pypi_name}
django-jsonfield is a reusable Django field that allows you to store validated
JSON in your model. It silently takes care of serialization. To use, simply
add the field to one of your models..
%endif

%package -n python3-django-%{pypi_name}
Summary:        %{summary}

Requires:  python3-django
Requires:  python3-django-formtools

%{?python_provide:%python_provide python3-%{pypi_name}}
%if 0%{?with_python2}
%else
Provides: python2-%{pypi_name}
%endif

%description -n python3-django-%{pypi_name}
django-jsonfield is a reusable Django field that allows you to store validated
JSON in your model. It silently takes care of serialization. To use, simply
add the field to one of your models.

%prep
%setup -q -n %{pypi_name}-%{version}

%build
%if 0%{?with_python2}
%py2_build
%endif
%py3_build

%install
%if 0%{?with_python2}
%py2_install
%endif
%py3_install

%check
sqlite3 FILETEMP
export DB_ENGINE=sqlite3
export DB_NAME="mydb"
%if 0%{?with_python2}
%{__python2} setup.py test
%endif
%{__python3} setup.py test

%if 0%{?with_python2}
%files -n python2-django-%{pypi_name}
%license LICENSE
%doc README.rst
%{python2_sitelib}/*
%endif

%files -n python3-django-%{pypi_name}
%license LICENSE
%doc README.rst
%{python3_sitelib}/*

%changelog
* Mon Aug 19 2019 Miro Hrončok <mhroncok@redhat.com> - 2.0.2-9
- Rebuilt for Python 3.8

* Fri Jul 26 2019 Fedora Release Engineering <releng@fedoraproject.org> - 2.0.2-8
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Sat Feb 02 2019 Fedora Release Engineering <releng@fedoraproject.org> - 2.0.2-7
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Sat Jul 14 2018 Fedora Release Engineering <releng@fedoraproject.org> - 2.0.2-6
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Tue Jun 19 2018 Miro Hrončok <mhroncok@redhat.com> - 2.0.2-5
- Rebuilt for Python 3.7

* Tue Mar 06 2018 Ralph Bean <rbean@redhat.com> - 2.0.2-4
- Make py3 package provide the old py2 name.

* Mon Mar 05 2018 Ralph Bean <rbean@redhat.com> - 2.0.2-3
- Disable python2 for https://fedoraproject.org/wiki/Changes/Django20

* Fri Feb 09 2018 Fedora Release Engineering <releng@fedoraproject.org> - 2.0.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Fri Jan 19 2018 Germano Massullo  <germano.massullo@gmail.com> - 2.0.2-1
- removed test.patch
- removes Source1 (LICENSE) because now it is in tar.gz
- 2.0.2 release

* Thu Jul 27 2017 Fedora Release Engineering <releng@fedoraproject.org> - 1.0.3-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Sat Feb 11 2017 Fedora Release Engineering <releng@fedoraproject.org> - 1.0.3-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_Mass_Rebuild

* Sat Jan 07 2017 Germano Massullo <germano.massullo@gmail.com> - 1.0.3-3
- rebuilt

* Mon Dec 19 2016 Miro Hrončok <mhroncok@redhat.com> - 1.0.3-2
- Rebuild for Python 3.6

* Wed Aug 10 2016 Germano Massullo <germano.massullo@gmail.com> - 1.0.3-1
- First commit on Fedora's Git
