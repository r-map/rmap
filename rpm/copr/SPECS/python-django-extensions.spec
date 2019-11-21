%global pypi_name django-extensions
%global summary Extensions for Django
Name:           python-%{pypi_name}
Version:        2.2.5
Release:        1%{?dist}
Summary:        %{summary}

License:        BSD
URL:            https://github.com/django-extensions/django-extensions
Source0:        https://files.pythonhosted.org/packages/source/d/%{pypi_name}/%{pypi_name}-%{version}.tar.gz
BuildArch:      noarch

%description
Django Extensions is a collection of custom extensions for the Django Framework.
These include management commands, additional database fields,
admin extensions and much more.

%package -n python2-%{pypi_name}
Summary:        %{summary} - Python 2 version
BuildRequires:  python2-devel

Requires:       python2-django
Requires:       python-openid


%description -n python2-%{pypi_name}
Django Extensions is a collection of custom extensions for the Django Framework.
These include management commands, additional database fields,
admin extensions and much more.
This package provides Python 2 build of %{pypi_name}.


%package -n python3-%{pypi_name}
Summary:        %{summary} - Python 3 version
BuildRequires:  python3-devel

Requires:       python3-django
Requires:       python3-openid


%description -n python3-%{pypi_name}
Django Extensions is a collection of custom extensions for the Django Framework.
These include management commands, additional database fields,
admin extensions and much more.
This package provides Python 3 build of %{pypi_name}.

%package -n python-%{pypi_name}-doc
Summary:        %{summary} - documentation

%description -n python-%{pypi_name}-doc
Django Extensions is a collection of custom extensions for the Django Framework.
These include management commands, additional database fields,
admin extensions and much more.
This package provides documentation build of %{pypi_name}.

%prep
%autosetup -n %{pypi_name}-%{version}

%build
%py2_build
%py3_build

# get rid of emtpy files (.tmpl templates)
rm -r build/lib/django_extensions/conf

%install
%py2_install
%py3_install
%find_lang django-extensions --all-name

%files -f django-extensions.lang -n python2-%{pypi_name}
%license LICENSE
%{python2_sitelib}/django_extensions
%{python2_sitelib}/django_extensions-%{version}-py?.?.egg-info
# find_lang will find both python2 and python3 locale files
%exclude %{python3_sitelib}/django_extensions/locale

%attr(755,-,-) %{python2_sitelib}/django_extensions/utils/dia2django.py
%attr(755,-,-) %{python2_sitelib}/django_extensions/management/modelviz.py
%attr(755,-,-) %{python2_sitelib}/django_extensions/management/commands/dumpscript.py
%attr(755,-,-) %{python2_sitelib}/django_extensions/management/commands/pipchecker.py

%files -f django-extensions.lang -n python3-%{pypi_name}
%license LICENSE
%{python3_sitelib}/django_extensions
%{python3_sitelib}/django_extensions-%{version}-py?.?.egg-info
# find_lang will find both python2 and python3 locale files
%exclude %{python2_sitelib}/django_extensions/locale

%attr(755,-,-) %{python3_sitelib}/django_extensions/utils/dia2django.py
%attr(755,-,-) %{python3_sitelib}/django_extensions/management/modelviz.py
%attr(755,-,-) %{python3_sitelib}/django_extensions/management/commands/dumpscript.py
%attr(755,-,-) %{python3_sitelib}/django_extensions/management/commands/pipchecker.py

%files -n python-%{pypi_name}-doc
%license LICENSE
%doc docs

%changelog
* Fri Feb 09 2018 Fedora Release Engineering <releng@fedoraproject.org> - 1.7.3-5
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Thu Jul 27 2017 Fedora Release Engineering <releng@fedoraproject.org> - 1.7.3-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_27_Mass_Rebuild

* Sat Feb 11 2017 Fedora Release Engineering <releng@fedoraproject.org> - 1.7.3-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_26_Mass_Rebuild

* Mon Dec 19 2016 Miro Hrončok <mhroncok@redhat.com> - 1.7.3-2
- Rebuild for Python 3.6

* Wed Aug 17 2016 Jan Beran <jberan@redhat.com> - 1.7.3-1
- update to version 1.7.3
- source update
- modernized specfile with Python 3 and doc packaging

* Tue Jul 19 2016 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.3.7-5
- https://fedoraproject.org/wiki/Changes/Automatic_Provides_for_Python_RPM_Packages

* Thu Feb 04 2016 Fedora Release Engineering <releng@fedoraproject.org> - 1.3.7-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_24_Mass_Rebuild

* Thu Jun 18 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.3.7-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Sat Jun 07 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.3.7-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Thu May 29 2014 Richard Marko <rmarko@fedoraproject.org> - 1.3.7-1
- New version

* Thu Oct 24 2013 Richard Marko <rmarko@fedoraproject.org> - 1.2.5-1
- New version

* Sun Aug 04 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.1.1-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_20_Mass_Rebuild

* Thu Apr 18 2013 Richard Marko <rmarko@fedoraproject.org> - 1.1.1-1
- New version

* Fri Feb 15 2013 Richard Marko <rmarko@fedoraproject.org> - 1.0.3-2
- Remove empty tests/urls.py file.

* Tue Jan 29 2013 Richard Marko <rmarko@fedoraproject.org> - 1.0.3-1
- Initial package.
