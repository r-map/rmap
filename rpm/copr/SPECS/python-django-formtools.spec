%global pypi_name django-formtools

# skip test until test suite supports later django
%global skip_tests 1

Name:           python-%{pypi_name}
Version:        2.1
Release:        11%{?dist}
Summary:        A set of high-level abstractions for Django forms

License:        BSD
URL:            http://django-formtools.readthedocs.org/en/latest/
Source0:        https://pypi.io/packages/source/d/%{pypi_name}/%{pypi_name}-%{version}.tar.gz
BuildArch:      noarch

%description
Django's "formtools" is a set of high-level abstractions for Django forms.
Currently for form previews and multi-step forms.


%package -n python3-%{pypi_name}
Summary:        A set of high-level abstractions for Django forms
%{?python_provide:%python_provide python3-%{pypi_name}}

BuildRequires:  python3-devel
BuildRequires:  python3-sphinx
BuildRequires:  python3-django >= 1.7
# Required for testing
BuildRequires:  python3-flake8
BuildRequires:  python3-coverage

Requires:       python3-django >= 1.7

Obsoletes:      python-%{pypi_name} < 2.1-5
Obsoletes:      python2-%{pypi_name} < 2.1-5

%description -n python3-%{pypi_name}
Django's "formtools" is a set of high-level abstractions for Django forms.
Currently for form previews and multi-step forms.

%package -n python3-%{pypi_name}-doc
Summary:        A set of high-level abstractions for Django forms - documentation
%{?python_provide:%python_provide python3-%{pypi_name}-doc}

Requires:       python3-%{pypi_name} = %{version}-%{release}

Obsoletes:      python-%{pypi_name}-doc < 2.1-5
Obsoletes:      python2-%{pypi_name}-doc < 2.1-5

%description -n python3-%{pypi_name}-doc
Django's "formtools" is a set of high-level abstractions for Django forms.

This is the associated documentation.

%prep
%setup -q -n %{pypi_name}-%{version}

%build
%{py3_build}

%if 0%{?skip_tests} == 0
%check
PYTHONPATH=. DJANGO_SETTINGS_MODULE=tests.settings python3-coverage run %{python3_sitelib}/django/bin/django-admin.py test tests
%endif

%install
%{py3_install}
%find_lang django py3lang
# generate html docs
sphinx-build-3 docs html
# remove the sphinx-build leftovers
rm -rf html/.{doctrees,buildinfo}


%files -n python3-%{pypi_name} -f py3lang
%doc README.rst
%license LICENSE
%{python3_sitelib}/formtools
%{python3_sitelib}/django_formtools-%{version}-py?.?.egg-info

%files -n python3-%{pypi_name}-doc
%doc html
%license LICENSE


%changelog
* Thu Oct 03 2019 Miro Hrončok <mhroncok@redhat.com> - 2.1-11
- Rebuilt for Python 3.8.0rc1 (#1748018)

* Mon Aug 19 2019 Miro Hrončok <mhroncok@redhat.com> - 2.1-10
- Rebuilt for Python 3.8

* Fri Jul 26 2019 Fedora Release Engineering <releng@fedoraproject.org> - 2.1-9
- Rebuilt for https://fedoraproject.org/wiki/Fedora_31_Mass_Rebuild

* Sat Feb 02 2019 Fedora Release Engineering <releng@fedoraproject.org> - 2.1-8
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Sat Jul 14 2018 Fedora Release Engineering <releng@fedoraproject.org> - 2.1-7
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Tue Jun 19 2018 Miro Hrončok <mhroncok@redhat.com> - 2.1-6
- Rebuilt for Python 3.7

* Mon Feb 12 2018 Miro Hrončok <mhroncok@redhat.com> - 2.1-5
- Removed Python 2 subpackage for https://fedoraproject.org/wiki/Changes/Django20

* Fri Feb 09 2018 Fedora Release Engineering <releng@fedoraproject.org> - 2.1-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Wed Jan 24 2018 Matthias Runge <mrunge@redhat.com> - 2.1-3
- fix python2-django1.11 requirements

* Fri Jan 19 2018 Matthias Runge <mrunge@redhat.com> - 2.1-2
- fix python2 requirements

* Fri Oct 20 2017 Javier Peña <jpena@redhat.com> - 2.1-1
- Updated to upstream release 2.1
- Fixed Source0 URL

* Thu Jul 27 2017 Fedora Release Engineering <releng@fedoraproject.org> - 1.0-11
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Sat Feb 11 2017 Fedora Release Engineering <releng@fedoraproject.org> - 1.0-10
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_Mass_Rebuild

* Mon Dec 19 2016 Miro Hrončok <mhroncok@redhat.com> - 1.0-9
- Rebuild for Python 3.6

* Tue Jul 19 2016 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.0-8
- https://fedoraproject.org/wiki/Changes/Automatic_Provides_for_Python_RPM_Packages

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 1.0-7
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Thu Nov 26 2015 jpena <jpena@redhat.com> - 1.0-6
- Fixed tests

* Tue Nov 10 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.0-5
- Rebuilt for https://fedoraproject.org/wiki/Changes/python3.5

* Thu Sep 10 2015 jpena <jpena@redhat.com> - 1.0-4
- Moved documentation into a subpackage
* Wed Sep 09 2015 jpena <jpena@redhat.com> - 1.0-3
- Handle locale files properly
* Wed Sep 09 2015 jpena <jpena@redhat.com> - 1.0-2
- Fixed python3 conditional in files section
- Added checks
- Moved sphinx-build to install section
* Tue Sep 08 2015 jpena <jpena@redhat.com> - 1.0-1
- Initial package.
