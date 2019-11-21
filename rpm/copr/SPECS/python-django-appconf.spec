%global pypi_name django-appconf
%global with_tests 0

Name:           python-%{pypi_name}
Version:        1.0.3
Release:        1%{?dist}
Summary:        A helper class for handling configuration defaults of packaged apps gracefully

License:        BSD
URL:            http://pypi.python.org/pypi/django-appconf/
Source0:        https://github.com/django-compressor/%{pypi_name}/archive/%{version}.tar.gz#/%{name}-%{version}.tar.gz
BuildArch:      noarch


%global _description\
A helper class for handling configuration\
defaults of packaged Django\
apps gracefully.

%description %_description


%package -n python3-%{pypi_name}
Summary:        A helper class for handling configuration defaults of packaged apps gracefully

BuildRequires:  python3-coverage
BuildRequires:  python3-devel
BuildRequires:  python3-django
BuildRequires:  python3-setuptools
BuildRequires:  python3-sphinx
BuildRequires:  python3-six

Requires:   python3-django

Obsoletes:  python2-%{pypi_name} < 1.0.2-6
Obsoletes:  python-%{pypi_name} < 1.0.2-6

%{?python_provide:%python_provide python3-%{pypi_name}}

%description -n python3-%{pypi_name} %_description


%prep
%autosetup -n %{pypi_name}-%{version} -p1


%build
%py3_build

# generate html docs
sphinx-build-3 -b html docs html
# remove the sphinx-build leftovers
rm -rf html/.{doctrees,buildinfo}


%install
%py3_install


# checks fail in mock
%check
export PYTHONPATH=.:$PYTHONPATH
export DJANGO_SETTINGS_MODULE=tests.test_settings
%{__python3} setup.py test
# coverage3 run %{_bindir}/python3-django-admin test -v2 test
#                        ^ change to django-admin once that is Python 3


%files -n python3-%{pypi_name}
%doc html README.rst
%license LICENSE
%{python3_sitelib}/appconf
%{python3_sitelib}/django_appconf-%{version}-py%{python3_version}.egg-info


%changelog
* Thu Sep 05 2019 Matthias Runge <mrunge@redhat.com> - 1.0.3-1
- update to 1.0.3 and FTBFS (rhbz#1737272)
- skip coverage tests for now

* Mon Aug 19 2019 Miro Hrončok <mhroncok@redhat.com> - 1.0.2-13
- Rebuilt for Python 3.8

* Fri Jul 26 2019 Fedora Release Engineering <releng@fedoraproject.org> - 1.0.2-12
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Thu Jul 11 2019 Matthias Runge <mrunge@redhat.com> - 1.0.2-11
- fix FTBFS (rhbz#1707040)

* Sat Feb 02 2019 Fedora Release Engineering <releng@fedoraproject.org> - 1.0.2-10
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Sat Jul 14 2018 Fedora Release Engineering <releng@fedoraproject.org> - 1.0.2-9
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Tue Jun 19 2018 Miro Hrončok <mhroncok@redhat.com> - 1.0.2-8
- Rebuilt for Python 3.7

* Fri Feb 09 2018 Fedora Release Engineering <releng@fedoraproject.org> - 1.0.2-7
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Tue Jan 16 2018 Miro Hrončok <mhroncok@redhat.com> - 1.0.2-6
- Removed Python 2 subpackage for https://fedoraproject.org/wiki/Changes/Django20

* Sat Aug 19 2017 Zbigniew Jędrzejewski-Szmek <zbyszek@in.waw.pl> - 1.0.2-5
- Python 2 binary package renamed to python2-django-appconf
  See https://fedoraproject.org/wiki/FinalizingFedoraSwitchtoPython3

* Thu Jul 27 2017 Fedora Release Engineering <releng@fedoraproject.org> - 1.0.2-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Sat Feb 11 2017 Fedora Release Engineering <releng@fedoraproject.org> - 1.0.2-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_Mass_Rebuild

* Mon Dec 19 2016 Miro Hrončok <mhroncok@redhat.com> - 1.0.2-2
- Rebuild for Python 3.6

* Wed Nov 02 2016 Matthias Runge <mrunge@redhat.com> - 1.0.2-1
- update to 1.0.2 (rhbz#1328640)

* Tue Jul 19 2016 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.0.1-5
- https://fedoraproject.org/wiki/Changes/Automatic_Provides_for_Python_RPM_Packages

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 1.0.1-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Tue Nov 10 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.0.1-3
- Rebuilt for https://fedoraproject.org/wiki/Changes/python3.5

* Thu Aug 27 2015 Matthias Runge <mrunge@redhat.com> - 1.0.1-2
- update to 1.0.1
- python3 subpackage
- fix requirements py/py3

* Thu Jun 18 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.6-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Sat Jun 07 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.6-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Sun Aug 04 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.6-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_20_Mass_Rebuild

* Wed Mar 06 2013 Matthias Runge <mrunge@redhat.com> - 0.6-1
- update to appconf-0.6

* Thu Feb 14 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.5-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_19_Mass_Rebuild

* Wed Sep 26 2012 Matthias Runge <mrunge@redhat.com> - 0.5-2
- also add requirement: Django/python-django

* Tue Sep 11 2012 Matthias Runge <mrunge@redhat.com> - 0.5-1
- Initial package.
