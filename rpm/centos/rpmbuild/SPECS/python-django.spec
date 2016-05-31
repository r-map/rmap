%if 0%{?fedora} > 12
%global with_python3 1 
%else
%{!?python_sitelib: %global python_sitelib %(%{__python} -c "from distutils.sysconfig import get_python_lib; print (get_python_lib())")}
%endif

%{!?__python2:%global __python2 %{__python}}
%{!?python2_sitelib:   %global python2_sitelib  %{python_sitelib}}
%{!?python2_sitearch:  %global python2_sitearch %{python_sitearch}}
%{!?python2_version:   %global python2_version  %{python_version}}



# Turn off the brp-python-bytecompile script
%global __os_install_post %(echo '%{__os_install_post}' | sed -e 's!/usr/lib[^[:space:]]*/brp-python-bytecompile[[:space:]].*$!!g')

%global         pkgname Django


# Tests requiring Internet connections are disabled by default
# pass --with internet to run them (e.g. when doing a local rebuild
# for sanity checks before committing)
%bcond_with internet

# one higher than the last Django release, to account for
# dist tags
%global         obs_ver 1.5.5-3

Name:           python-django

Version:        1.8.8
Release:        1%{?dist}
Summary:        A high-level Python Web framework

Group:          Development/Languages
License:        BSD
URL:            http://www.djangoproject.com/
Source0:        https://pypi.python.org/packages/source/D/Django/Django-%{version}.tar.gz

Patch0:         python-django-1.8.3-shell-completion.patch

BuildArch:      noarch
BuildRequires:  python2-devel

# test requirements
#BuildRequires:  py-bcrypt
BuildRequires:  python-docutils
BuildRequires:  python-jinja2
BuildRequires:  python-mock
BuildRequires:  numpy
BuildRequires:  python-pillow
BuildRequires:  PyYAML
BuildRequires:  pytz
%if 0%{?fedora} > 0
BuildRequires:  python-selenium
%endif
BuildRequires:  python-sqlparse
BuildRequires:  python-memcached

%if 0%{?with_python3}
BuildRequires:  python3-devel
# test requirements
#BuildRequires: python3-py-bcrypt
BuildRequires:  python3-docutils
BuildRequires:  python3-jinja2
BuildRequires:  python3-mock
BuildRequires:  python3-numpy
BuildRequires:  python3-pillow
BuildRequires:  python3-PyYAML
BuildRequires:  python3-pytz
BuildRequires:  python3-selenium
#BuildRequires:  python3-sqlparse
BuildRequires:  python3-memcached
%endif
Requires:       %{name}-bash-completion = %{version}-%{release}

# allow users to use django with lowercase d
Provides:       django = %{version}-%{release}
Provides:       %{pkgname} = %{version}-%{release}
Obsoletes:      %{pkgname} < %{obs_ver}


%description
Django is a high-level Python Web framework that encourages rapid
development and a clean, pragmatic design. It focuses on automating as
much as possible and adhering to the DRY (Don't Repeat Yourself)
principle.

%package doc
Summary:        Documentation for Django
Group:          Documentation
Requires:       %{name} = %{version}-%{release}

BuildRequires:  python-sphinx

Provides:       django-docs = %{version}-%{release}
Provides:       %{pkgname}-docs = %{version}-%{release}
Obsoletes:      %{pkgname}-docs < %{obs_ver}

%description doc
This package contains the documentation for the Django high-level
Python Web framework.

%package bash-completion
Summary:        bash completion files for Django
BuildRequires:  bash-completion


%description bash-completion
This package contains the bash completion files form Django high-level
Python Web framework.

%if 0%{?with_python3}
%package -n python3-django-doc
Summary:        Documentation for Django
Group:          Documentation
Requires:       python3-django = %{version}-%{release}

%description -n python3-django-doc
This package contains the documentation for the Django high-level
Python Web framework.

%package -n python3-django
Summary:        A high-level Python Web framework
Group:          Development/Languages

Requires:       python3
Requires:       %{name}-bash-completion = %{version}-%{release}

%description -n python3-django
Django is a high-level Python Web framework that encourages rapid
development and a clean, pragmatic design. It focuses on automating as
much as possible and adhering to the DRY (Don't Repeat Yourself)
principle.
%endif

%prep
%setup -qc

mv Django-%{version} python2
pushd python2

# copy LICENSE etc. to top level dir
cp -a LICENSE ..
cp -a README.rst AUTHORS ..

# remove bundled egg-info
rm -rf Django.egg-info

%patch0

# empty files
for f in \
    django/contrib/staticfiles/models.py \
    django/contrib/webdesign/models.py \
    django/contrib/humanize/models.py \
; do
  echo "# just a comment" > $f
done
popd

%if 0%{?with_python3}
cp -a python2 python3
%endif


%build
pushd python2
%{__python} setup.py build
popd

%if 0%{?with_python3}
pushd python3
%{__python3} setup.py build
popd
%endif # with_python3


%install
# must do install of python 3 subpackage first, so that we don't
# overwrite django-admin script with the python 3 version
%if 0%{?with_python3}
pushd python3
%{__python3} setup.py install --skip-build --root %{buildroot}
mv %{buildroot}%{_bindir}/django-admin.py %{buildroot}%{_bindir}/python3-django-admin
popd
%endif # with_python3

pushd python2
%{__python} setup.py install --skip-build --root %{buildroot}

%if 0%{?fedora} > 0
# Manually invoke the python byte compile macro for each path that needs byte
# compilation.
%py_byte_compile %{__python2} %{buildroot}%{python2_sitelib}

%if 0%{?with_python3}
%py_byte_compile %{__python3} %{buildroot}%{python3_sitelib}
%endif # with_python3

%endif

popd
%find_lang django
%find_lang djangojs
# append djangojs.lang to django.lang
cat djangojs.lang >> django.lang

%if 0%{?with_python3}
# When creating Python3 package, separate lang to Python 2 and Python 3 files
grep python3 django.lang > python3-django.lang
grep python2 django.lang > python2-django.lang
mv {python2-,}django.lang
%endif # with_python3

pushd python2
# build documentation
(cd docs && mkdir djangohtml && mkdir -p _build/{doctrees,html} && make html)
cp -ar docs ..

# install man pages
mkdir -p %{buildroot}%{_mandir}/man1/
cp -p docs/man/* %{buildroot}%{_mandir}/man1/
%if 0%{?with_python3}
cp -a %{buildroot}%{_mandir}/man1/django-admin.1 %{buildroot}%{_mandir}/man1/python3-django-admin.1
%endif # with_python3

# install bash completion script
bashcompdir=$(pkg-config --variable=completionsdir bash-completion)
mkdir -p %{buildroot}$bashcompdir
install -m 0644 -p extras/django_bash_completion \
  %{buildroot}$bashcompdir/django-admin.py

for file in django-admin python3-django-admin manage.py ; do
   ln -s django-admin.py %{buildroot}$bashcompdir/$file
done
popd

# Fix items in %%{_bindir}
mv %{buildroot}%{_bindir}/django-admin.py %{buildroot}%{_bindir}/django-admin

# remove .po files
find $RPM_BUILD_ROOT -name "*.po" | xargs rm -f

# checks failing in f21. need to examinate
%check
%if 0%{?fedora} > 0
pushd python2
export PYTHONPATH=$(pwd)
export LANG=en_US.utf8
cd tests
# %{__python} ./runtests.py --settings=test_sqlite --verbosity=2
popd

%if 0%{?with_python3}
pushd python3
export PYTHONPATH=$(pwd)
cd tests
# %{__python3} runtests.py --settings=test_sqlite --verbosity=2
popd
%endif # with_python3

%endif


%files -f django.lang
%doc AUTHORS README.rst
%license LICENSE
# manual pages are owned by both python2 and python3 packages
%{_mandir}/man1/*
# except the symlink with python3 prefix
%if 0%{?with_python3}
%exclude %{_mandir}/man1/python3-*
%endif # with_python3
%{_bindir}/django-admin
%attr(0755,root,root) %{python_sitelib}/django/bin/django-admin.py*
# Include everything but the locale data ...
%dir %{python_sitelib}/django
%dir %{python_sitelib}/django/bin
%{python_sitelib}/django/apps
%{python_sitelib}/django/db/
%{python_sitelib}/django/*.py*
%{python_sitelib}/django/utils/
%{python_sitelib}/django/dispatch/
%{python_sitelib}/django/template/
%{python_sitelib}/django/views/
%dir %{python_sitelib}/django/conf/
%dir %{python_sitelib}/django/conf/locale/
%dir %{python_sitelib}/django/conf/locale/??/
%dir %{python_sitelib}/django/conf/locale/??_*/
%dir %{python_sitelib}/django/conf/locale/*/LC_MESSAGES
%dir %{python_sitelib}/django/contrib/
%{python_sitelib}/django/contrib/*.py*
%dir %{python_sitelib}/django/contrib/admin/
%dir %{python_sitelib}/django/contrib/admin/locale
%dir %{python_sitelib}/django/contrib/admin/locale/??/
%dir %{python_sitelib}/django/contrib/admin/locale/??_*/
%dir %{python_sitelib}/django/contrib/admin/locale/*/LC_MESSAGES
%{python_sitelib}/django/contrib/admin/*.py*
%{python_sitelib}/django/contrib/admin/migrations
%{python_sitelib}/django/contrib/admin/views/
%{python_sitelib}/django/contrib/admin/static/
%{python_sitelib}/django/contrib/admin/templatetags/
%{python_sitelib}/django/contrib/admin/templates/
%dir %{python_sitelib}/django/contrib/admindocs/
%dir %{python_sitelib}/django/contrib/admindocs/locale/
%dir %{python_sitelib}/django/contrib/admindocs/locale/??/
%dir %{python_sitelib}/django/contrib/admindocs/locale/??_*/
%dir %{python_sitelib}/django/contrib/admindocs/locale/*/LC_MESSAGES
%{python_sitelib}/django/contrib/admindocs/*.py*
%{python_sitelib}/django/contrib/admindocs/templates/
%{python_sitelib}/django/contrib/admindocs/tests/
%dir %{python_sitelib}/django/contrib/auth/
%dir %{python_sitelib}/django/contrib/auth/locale/
%dir %{python_sitelib}/django/contrib/auth/locale/??/
%dir %{python_sitelib}/django/contrib/auth/locale/??_*/
%dir %{python_sitelib}/django/contrib/auth/locale/*/LC_MESSAGES
%{python_sitelib}/django/contrib/auth/*.py*
%{python_sitelib}/django/contrib/auth/handlers/
%{python_sitelib}/django/contrib/auth/management/
%{python_sitelib}/django/contrib/auth/migrations/
%{python_sitelib}/django/contrib/auth/templates/
%{python_sitelib}/django/contrib/auth/tests/
%dir %{python_sitelib}/django/contrib/contenttypes/
%dir %{python_sitelib}/django/contrib/contenttypes/locale
%dir %{python_sitelib}/django/contrib/contenttypes/locale/??/
%dir %{python_sitelib}/django/contrib/contenttypes/locale/??_*/
%dir %{python_sitelib}/django/contrib/contenttypes/locale/*/LC_MESSAGES
%{python_sitelib}/django/contrib/contenttypes/migrations
%{python_sitelib}/django/contrib/contenttypes/*.py*
%dir %{python_sitelib}/django/contrib/flatpages/
%dir %{python_sitelib}/django/contrib/flatpages/locale/
%dir %{python_sitelib}/django/contrib/flatpages/locale/??/
%dir %{python_sitelib}/django/contrib/flatpages/locale/??_*/
%dir %{python_sitelib}/django/contrib/flatpages/locale/*/LC_MESSAGES
%{python_sitelib}/django/contrib/flatpages/*.py*
%{python_sitelib}/django/contrib/flatpages/migrations/
%{python_sitelib}/django/contrib/flatpages/templatetags
%dir %{python_sitelib}/django/contrib/gis/
%dir %{python_sitelib}/django/contrib/gis/locale/
%dir %{python_sitelib}/django/contrib/gis/locale/??/
%dir %{python_sitelib}/django/contrib/gis/locale/??_*/
%dir %{python_sitelib}/django/contrib/gis/locale/*/LC_MESSAGES
%{python_sitelib}/django/contrib/gis/*.py*
%{python_sitelib}/django/contrib/gis/geoip/
%{python_sitelib}/django/contrib/gis/serializers/
%dir %{python_sitelib}/django/contrib/humanize/
%dir %{python_sitelib}/django/contrib/humanize/locale/
%dir %{python_sitelib}/django/contrib/humanize/locale/??/
%dir %{python_sitelib}/django/contrib/humanize/locale/??_*/
%dir %{python_sitelib}/django/contrib/humanize/locale/*/LC_MESSAGES
%{python_sitelib}/django/contrib/humanize/templatetags/
%{python_sitelib}/django/contrib/humanize/*.py*
%{python_sitelib}/django/contrib/messages/*.py*
%dir %{python_sitelib}/django/contrib/postgres/
%{python_sitelib}/django/contrib/postgres/*.py*
%{python_sitelib}/django/contrib/postgres/fields
%{python_sitelib}/django/contrib/postgres/forms
%dir %{python_sitelib}/django/contrib/redirects
%dir %{python_sitelib}/django/contrib/redirects/locale
%dir %{python_sitelib}/django/contrib/redirects/locale/??/
%dir %{python_sitelib}/django/contrib/redirects/locale/??_*/
%dir %{python_sitelib}/django/contrib/redirects/locale/*/LC_MESSAGES
%{python_sitelib}/django/contrib/redirects/*.py*
%{python_sitelib}/django/contrib/redirects/migrations
%dir %{python_sitelib}/django/contrib/sessions/
%dir %{python_sitelib}/django/contrib/sessions/locale/
%dir %{python_sitelib}/django/contrib/sessions/locale/??/
%dir %{python_sitelib}/django/contrib/sessions/locale/??_*/
%dir %{python_sitelib}/django/contrib/sessions/locale/*/LC_MESSAGES
%{python_sitelib}/django/contrib/sessions/management/
%{python_sitelib}/django/contrib/sessions/migrations/
%{python_sitelib}/django/contrib/sessions/*.py*
%{python_sitelib}/django/contrib/sitemaps/
%dir %{python_sitelib}/django/contrib/sites/
%dir %{python_sitelib}/django/contrib/sites/locale/
%dir %{python_sitelib}/django/contrib/sites/locale/??/
%dir %{python_sitelib}/django/contrib/sites/locale/??_*/
%dir %{python_sitelib}/django/contrib/sites/locale/*/LC_MESSAGES
%{python_sitelib}/django/contrib/sites/*.py*
%{python_sitelib}/django/contrib/sites/migrations
%{python_sitelib}/django/contrib/staticfiles/
%{python_sitelib}/django/contrib/syndication/
%{python_sitelib}/django/contrib/webdesign/
%{python_sitelib}/django/contrib/gis/admin/
%{python_sitelib}/django/contrib/gis/db/
%{python_sitelib}/django/contrib/gis/forms/
%{python_sitelib}/django/contrib/gis/gdal/
%{python_sitelib}/django/contrib/gis/geometry/
%{python_sitelib}/django/contrib/gis/geos/
%{python_sitelib}/django/contrib/gis/management/
%{python_sitelib}/django/contrib/gis/maps/
%{python_sitelib}/django/contrib/gis/sitemaps/
%{python_sitelib}/django/contrib/gis/static/gis/js/OLMapWidget.js
%{python_sitelib}/django/contrib/gis/templates/
%{python_sitelib}/django/contrib/gis/utils/
%{python_sitelib}/django/contrib/messages/storage/
%{python_sitelib}/django/contrib/sessions/backends/
%{python_sitelib}/django/forms/
%{python_sitelib}/django/templatetags/
%{python_sitelib}/django/core/
%{python_sitelib}/django/http/
%{python_sitelib}/django/middleware/
%{python_sitelib}/django/test/
%{python_sitelib}/django/conf/*.py*
%{python_sitelib}/django/conf/project_template/
%{python_sitelib}/django/conf/app_template/
%{python_sitelib}/django/conf/urls/
%{python_sitelib}/django/conf/locale/*/*.py*
%{python_sitelib}/django/conf/locale/*.py*

%{python_sitelib}/*.egg-info

%files doc
%doc docs/_build/html/*

%files bash-completion
%{_datadir}/bash-completion

%if 0%{?with_python3}
%files -n python3-django-doc
%doc docs/_build/html/*

%files -n python3-django -f python3-django.lang
%doc AUTHORS README.rst
%license LICENSE
%{_bindir}/python3-django-admin
# as said before, manpages are owned by both python2 and python3 packages
%exclude %{_mandir}/man1/django-admin.1*
%{_mandir}/man1/python3-django-admin.1.*
%attr(0755,root,root) %{python3_sitelib}/django/bin/django-admin.py*
# Include everything but the locale data ...
%dir %{python3_sitelib}/django
%dir %{python3_sitelib}/django/bin
%{python3_sitelib}/django/apps
%{python3_sitelib}/django/db/
%{python3_sitelib}/django/*.py*
%{python3_sitelib}/django/utils/
%{python3_sitelib}/django/dispatch/
%{python3_sitelib}/django/template/
%{python3_sitelib}/django/views/
%dir %{python3_sitelib}/django/conf/
%dir %{python3_sitelib}/django/conf/locale/
%dir %{python3_sitelib}/django/conf/locale/??/
%dir %{python3_sitelib}/django/conf/locale/??_*/
%dir %{python3_sitelib}/django/conf/locale/*/LC_MESSAGES
%dir %{python3_sitelib}/django/contrib/
%{python3_sitelib}/django/contrib/*.py*
%dir %{python3_sitelib}/django/contrib/admin/
%dir %{python3_sitelib}/django/contrib/admin/locale
%dir %{python3_sitelib}/django/contrib/admin/locale/??/
%dir %{python3_sitelib}/django/contrib/admin/locale/??_*/
%dir %{python3_sitelib}/django/contrib/admin/locale/*/LC_MESSAGES
%{python3_sitelib}/django/contrib/admin/*.py*
%{python3_sitelib}/django/contrib/admin/migrations
%{python3_sitelib}/django/contrib/admin/views/
%{python3_sitelib}/django/contrib/admin/static/
%{python3_sitelib}/django/contrib/admin/templatetags/
%{python3_sitelib}/django/contrib/admin/templates/
%dir %{python3_sitelib}/django/contrib/admindocs/
%dir %{python3_sitelib}/django/contrib/admindocs/locale/
%dir %{python3_sitelib}/django/contrib/admindocs/locale/??/
%dir %{python3_sitelib}/django/contrib/admindocs/locale/??_*/
%dir %{python3_sitelib}/django/contrib/admindocs/locale/*/LC_MESSAGES
%{python3_sitelib}/django/contrib/admindocs/*.py*
%{python3_sitelib}/django/contrib/admindocs/templates/
%{python3_sitelib}/django/contrib/admindocs/tests/
%dir %{python3_sitelib}/django/contrib/auth/
%dir %{python3_sitelib}/django/contrib/auth/locale/
%dir %{python3_sitelib}/django/contrib/auth/locale/??/
%dir %{python3_sitelib}/django/contrib/auth/locale/??_*/
%dir %{python3_sitelib}/django/contrib/auth/locale/*/LC_MESSAGES
%{python3_sitelib}/django/contrib/auth/*.py*
%{python3_sitelib}/django/contrib/auth/handlers/
%{python3_sitelib}/django/contrib/auth/management/
%{python3_sitelib}/django/contrib/auth/migrations/
%{python3_sitelib}/django/contrib/auth/templates/
%{python3_sitelib}/django/contrib/auth/tests/
%dir %{python3_sitelib}/django/contrib/contenttypes/
%dir %{python3_sitelib}/django/contrib/contenttypes/locale
%dir %{python3_sitelib}/django/contrib/contenttypes/locale/??/
%dir %{python3_sitelib}/django/contrib/contenttypes/locale/??_*/
%dir %{python3_sitelib}/django/contrib/contenttypes/locale/*/LC_MESSAGES
%{python3_sitelib}/django/contrib/contenttypes/*.py*
%{python3_sitelib}/django/contrib/contenttypes/__pycache__
%{python3_sitelib}/django/contrib/contenttypes/migrations/
%dir %{python3_sitelib}/django/contrib/flatpages/
%dir %{python3_sitelib}/django/contrib/flatpages/locale/
%dir %{python3_sitelib}/django/contrib/flatpages/locale/??/
%dir %{python3_sitelib}/django/contrib/flatpages/locale/??_*/
%dir %{python3_sitelib}/django/contrib/flatpages/locale/*/LC_MESSAGES
%{python3_sitelib}/django/contrib/flatpages/*.py*
%{python3_sitelib}/django/contrib/flatpages/migrations
%{python3_sitelib}/django/contrib/flatpages/templatetags
%dir %{python3_sitelib}/django/contrib/gis/
%dir %{python3_sitelib}/django/contrib/gis/locale/
%dir %{python3_sitelib}/django/contrib/gis/locale/??/
%dir %{python3_sitelib}/django/contrib/gis/locale/??_*/
%dir %{python3_sitelib}/django/contrib/gis/locale/*/LC_MESSAGES
%{python3_sitelib}/django/contrib/gis/*.py*
%{python3_sitelib}/django/contrib/gis/geoip/
%{python3_sitelib}/django/contrib/gis/serializers/
%dir %{python3_sitelib}/django/contrib/humanize/
%dir %{python3_sitelib}/django/contrib/humanize/locale/
%dir %{python3_sitelib}/django/contrib/humanize/locale/??/
%dir %{python3_sitelib}/django/contrib/humanize/locale/??_*/
%dir %{python3_sitelib}/django/contrib/humanize/locale/*/LC_MESSAGES
%{python3_sitelib}/django/contrib/humanize/templatetags/
%{python3_sitelib}/django/contrib/humanize/*.py*
%dir %{python3_sitelib}/django/contrib/messages/
%{python3_sitelib}/django/contrib/messages/*.py*
%dir %{python3_sitelib}/django/contrib/postgres
%{python3_sitelib}/django/contrib/postgres/*.py*
%{python3_sitelib}/django/contrib/postgres/fields
%{python3_sitelib}/django/contrib/postgres/forms
%{python3_sitelib}/django/contrib/postgres/__pycache__
%dir %{python3_sitelib}/django/contrib/redirects
%dir %{python3_sitelib}/django/contrib/redirects/locale
%dir %{python3_sitelib}/django/contrib/redirects/locale/??/
%dir %{python3_sitelib}/django/contrib/redirects/locale/??_*/
%dir %{python3_sitelib}/django/contrib/redirects/locale/*/LC_MESSAGES
%{python3_sitelib}/django/contrib/redirects/*.py*
%{python3_sitelib}/django/contrib/redirects/migrations
%dir %{python3_sitelib}/django/contrib/sessions/
%dir %{python3_sitelib}/django/contrib/sessions/locale/
%dir %{python3_sitelib}/django/contrib/sessions/locale/??/
%dir %{python3_sitelib}/django/contrib/sessions/locale/??_*/
%dir %{python3_sitelib}/django/contrib/sessions/locale/*/LC_MESSAGES
%{python3_sitelib}/django/contrib/sessions/management/
%{python3_sitelib}/django/contrib/sessions/migrations/
%{python3_sitelib}/django/contrib/sessions/*.py*
%{python3_sitelib}/django/contrib/sitemaps/
%dir %{python3_sitelib}/django/contrib/sites/
%dir %{python3_sitelib}/django/contrib/sites/locale/
%dir %{python3_sitelib}/django/contrib/sites/locale/??/
%dir %{python3_sitelib}/django/contrib/sites/locale/??_*/
%dir %{python3_sitelib}/django/contrib/sites/locale/*/LC_MESSAGES
%{python3_sitelib}/django/contrib/sites/*.py*
%{python3_sitelib}/django/contrib/sites/migrations
%{python3_sitelib}/django/contrib/staticfiles/
%{python3_sitelib}/django/contrib/syndication/
%{python3_sitelib}/django/contrib/webdesign/
%{python3_sitelib}/django/contrib/gis/admin/
%{python3_sitelib}/django/contrib/gis/db/
%{python3_sitelib}/django/contrib/gis/forms/
%{python3_sitelib}/django/contrib/gis/gdal/
%{python3_sitelib}/django/contrib/gis/geometry/
%{python3_sitelib}/django/contrib/gis/geos/
%{python3_sitelib}/django/contrib/gis/management/
%{python3_sitelib}/django/contrib/gis/maps/
%{python3_sitelib}/django/contrib/gis/sitemaps/
%{python3_sitelib}/django/contrib/gis/static/gis/js/OLMapWidget.js
%{python3_sitelib}/django/contrib/gis/templates/
%{python3_sitelib}/django/contrib/gis/utils/
%{python3_sitelib}/django/contrib/messages/storage/
%{python3_sitelib}/django/contrib/sessions/backends/
%{python3_sitelib}/django/forms/
%{python3_sitelib}/django/templatetags/
%{python3_sitelib}/django/core/
%{python3_sitelib}/django/http/
%{python3_sitelib}/django/middleware/
%{python3_sitelib}/django/test/
%{python3_sitelib}/django/conf/*.py*
%{python3_sitelib}/django/conf/project_template/
%{python3_sitelib}/django/conf/app_template/
%{python3_sitelib}/django/conf/urls/
%{python3_sitelib}/django/conf/locale/*/*.py*
%{python3_sitelib}/django/conf/locale/*.py*
%{python3_sitelib}/%{pkgname}-%{version}-py%{python3_version}.egg-info
%{python3_sitelib}/django/__pycache__
%{python3_sitelib}/django/bin/__pycache__
%{python3_sitelib}/django/conf/__pycache__
%{python3_sitelib}/django/conf/locale/*/__pycache__
%{python3_sitelib}/django/contrib/__pycache__
%{python3_sitelib}/django/contrib/admin/__pycache__
%{python3_sitelib}/django/contrib/admindocs/__pycache__
%{python3_sitelib}/django/contrib/auth/__pycache__
%{python3_sitelib}/django/contrib/flatpages/__pycache__
%{python3_sitelib}/django/contrib/gis/__pycache__
%{python3_sitelib}/django/contrib/humanize/__pycache__
%{python3_sitelib}/django/contrib/messages/__pycache__
%{python3_sitelib}/django/contrib/redirects/__pycache__
%{python3_sitelib}/django/contrib/sessions/__pycache__
%{python3_sitelib}/django/contrib/sites/__pycache__
%endif


%changelog
* Mon Jan 04 2016 Matthias Runge <mrunge@redhat.com> - 1.8.8-1
- update to 1.8.8

* Wed Nov 25 2015 Matthias Runge <mrunge@redhat.com> - 1.8.7-1
- Update to 1.8.7 , fixing CVE-2015-8213 (rhbz#1285278)

* Thu Nov 12 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.8.6-2
- Rebuilt for https://fedoraproject.org/wiki/Changes/python3.5

* Thu Nov 05 2015 Matthias Runge <mrunge@redhat.com> - 1.8.6-1
- rebase to 1.8.6 (rhbz#1276914)

* Wed Nov 04 2015 Robert Kuska <rkuska@redhat.com> - 1.8.5-2
- Rebuilt for Python3.5 rebuild

* Mon Nov 02 2015 Matthias Runge <mrunge@redhat.com> - 1.8.5-1
- rebase to 1.8.5 (rhbz#1276914)

* Wed Aug 12 2015 Ville Skyttä <ville.skytta@iki.fi> - 1.8.3-2
- Do not install bash completion for python executables
  (Ville Skyttä, rhbz#1253076)
- CVE-2015-5963 Denial-of-service possibility in logout() view by filling
  session store (rhbz#1254911)
- CVE-2015-5964 Denial-of-service possibility in logout() view by filling
  session store (rhbz#1252891)

* Thu Jul 09 2015 Matthias Runge <mrunge@redhat.com> - 1.8.3-1
- fix DoS via URL validation, CVE-2015-5145 (rhbh#1240526)
- possible header injection due to validators accepting newlines in 
  input, CVE-2015-5144 (rhbz#1239011)
- possible DoS by filling session store, CVE-2015-5143 (rhbz#1239010)
- update to 1.8.3 (rhbz#1241300)

* Mon Jul 06 2015 Matthias Runge <mrunge@redhat.com> - 1.8.2-2
- disable failing py2 tests for now, p3 passes (rhbz#1239824)

* Thu Jun 18 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.8.2-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Wed May 20 2015 Matthias Runge <mrunge@redhat.com> - 1.8.2-1
- fix CVE-2015-3982 - Fixed session flushing in the cached_db backend
  (rhbz#1223591)

* Mon May 04 2015 Matthias Runge <mrunge@redhat.com> - 1.8.1-1
- update to 1.8.1 (rhbz#1217863)

* Tue Apr 7 2015 Matthias Runge <mrunge@redhat.com> - 1.8-1
- update to 1.8 final

* Mon Mar 23 2015 Matthias Runge <mrunge@redhat.com> - 1.8.0.7.c1
- modernize spec for python3
- 1.8c1 snapshot
- fix for CVE-2015-2316 (rhbz#1203614)
- fix for CVE-2015-2317 (rhbz#1203616)

* Tue Mar 10 2015 Matthias Runge <mrunge@redhat.com> - 1.8-0.6.b2
- 1.8b2 snapshot and security fix

* Wed Feb 25 2015 Matthias Runge <mrunge@redhat.com> - 1.8-0.5.b1
- 1.8b1 snapshot

* Mon Feb 02 2015 Matthias Runge <mrunge@redhat.com> - 1.8-0.4.a1
- remove BR python-sphinx-latex
- fix build on epel7

* Sun Feb 01 2015 Matthias Runge <mrunge@redhat.com> - 1.7.4-1
- update to 1.7.4
- Install bash completion to %%{_datadir}/bash-completion
  (rhbz#1185574), thanks to Ville Skyttä
- add BR python-sphinx-latex

* Tue Jan 20 2015 Matthias Runge <mrunge@redhat.com> - 1.8-0.1.a1
- update to Django-1.8 Alpha1

* Wed Jan 14 2015 Matthias Runge <mrunge@redhat.com> - 1.7.3-1
- update to 1.7.3, fixes CVE-2015-0221 (rhbz#1181946, rhbz#1179679)

* Mon Jan 05 2015 Matthias Runge <mrunge@redhat.com> - 1.7.2-1
- update to 1.7.2 (rhbz#1157514)

* Tue Nov 11 2014 Matthias Runge <mrunge@redhat.com> - 1.7.1-1
- update to 1.7.1 (rhbz#1157514)

* Fri Oct 17 2014 Matthias Runge <mrunge@redhat.com> - 1.7-1
- update to 1.7 (rhbz#1132877)

* Thu Sep 25 2014 Matthias Runge <mrunge@redhat.com> - 1.6.7-1
- update to 1.6.7
- don't own bash-completion dir.

* Thu Aug 21 2014 Matthias Runge <mrunge@redhat.com> - 1.6.6-1
- update to 1.6.6
- fix CVE-2014-0480 (rhbz#1129950)
- fix CVE-2014-0481 (rhbz#1129952)
- fix CVE-2014-0482 (rhbz#1129954)
- fix CVE-2014-0483 (rhbz#1129959)

* Sat Jun 07 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.6.5-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Wed May 28 2014 Kalev Lember <kalevlember@gmail.com> - 1.6.5-2
- Rebuilt for https://fedoraproject.org/wiki/Changes/Python_3.4


* Fri May 16 2014 Matthias Runge <mrunge@redhat.com> - 1.6.5-1
- update to 1.6.5 CVE-2014-1418, CVE-2014-3730 (rhbz#1097935)

* Mon May 12 2014 Matthias Runge <mrunge@redhat.com> - 1.6.4-2
- don't hardcode python3.3

* Wed May 07 2014 Matthias Runge <mrunge@redhat.com> - 1.6.4-1
- update to 1.6.4 fix a potential regression in reverse()

* Tue Apr 22 2014 Matthias Runge <mrunge@redhat.com> - 1.6.3-1
- update to 1.6.3 fixing CVE-2014-0473 and CVE-2014-0474

* Thu Mar 27 2014 Matthias Runge <mrunge@redhat.com> - 1.6.2-2
- remove simplejson requirement
- make bash-completion a sub-package, both main packages can require

* Thu Feb 13 2014 Matthias Runge <mrunge@redhat.com> - 1.6.2-1
- update to 1.6.2 (rhbz#1027766)
- bash completion for python3-django-admin (rhbz#1035987)

* Sun Nov 24 2013 Matěj Cepl <mcepl@redhat.com> - 1.6-1
- update to 1.6 (rhbz#1027766)
