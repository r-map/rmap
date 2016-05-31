%{!?__python2:%global __python2 %{__python}}
%{!?python2_sitelib:   %global python2_sitelib  %{python_sitelib}}
%{!?python2_sitearch:  %global python2_sitearch %{python_sitearch}}
%{!?python2_version:   %global python2_version  %{python_version}}

%global pypi_name django-appconf

%if 0%{?fedora} > 12
%global with_python3 1
%else
%global with_python3 0
%endif

Name:           python-%{pypi_name}
Version:        1.0.1
Release:        4%{?dist}
Summary:        A helper class for handling configuration defaults of packaged apps gracefully

License:        BSD
URL:            http://pypi.python.org/pypi/django-appconf/
Source0:        http://pypi.python.org/packages/source/d/%{pypi_name}/%{pypi_name}-%{version}.tar.gz
BuildArch:      noarch

BuildRequires:  python2-devel
BuildRequires:  python-sphinx
BuildRequires:  python-setuptools

#BuildRequires:  python-django-discover-runner
#BuildRequires:  python-flake8
BuildRequires:  python-coverage
BuildRequires:  python-django

Requires:       python-django

%description
A helper class for handling configuration
defaults of packaged Django
apps gracefully.

%if 0%{?with_python3}
%package -n python3-%{pypi_name}
Summary:        A helper class for handling configuration defaults of packaged apps gracefully

BuildRequires:  python3-devel
BuildRequires:  python-sphinx
BuildRequires:  python3-setuptools

Requires:   python3-django

%description -n python3-%{pypi_name}
A helper class for handling configuration
defaults of packaged Django
apps gracefully.

%endif

%prep
%setup -qc
mv %{pypi_name}-%{version} python2

pushd python2

# copy license etc to top level dir
cp -a README.rst ..
cp -a LICENSE ..

# generate html docs
sphinx-build docs html
# remove the sphinx-build leftovers
rm -rf html/.{doctrees,buildinfo}

# move html up to get picked up for docs
mv html ..

popd

%if 0%{?with_python3}
cp -a python2 python3
%endif

%build
pushd python2
%{__python2} setup.py build
popd

%if 0%{?with_python3}
pushd python3
%{__python3} setup.py build
popd
%endif


%install
pushd python2
%{__python2} setup.py install --skip-build --root %{buildroot}
popd

%if 0%{?with_python3}
pushd python3
%{__python3} setup.py install --skip-build --root %{buildroot}
popd
%endif

# checks fail in mock
#%check
#pushd python2
#%{__python} setup.py test
#export PYTHONPATH=.:$PYTHONPATH
#export DJANGO_SETTINGS_MODULE=tests.test_settings
#coverage run %{_bindir}/django-admin test -v2 test
#popd

%files
%doc html README.rst
%license LICENSE
%{python2_sitelib}/appconf
%{python2_sitelib}/django_appconf-%{version}-py%{python2_version}.egg-info

%if 0%{?with_python3}
%files -n python3-%{pypi_name}
%doc html README.rst
%license LICENSE
%{python3_sitelib}/appconf
%{python3_sitelib}/django_appconf-%{version}-py%{python3_version}.egg-info
%endif

%changelog
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
