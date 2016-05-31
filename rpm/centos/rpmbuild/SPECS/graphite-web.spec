%if 0%{?rhel} && 0%{?rhel} <= 6
%{!?__python2: %global __python2 /usr/bin/python2}
%{!?python2_sitelib: %global python2_sitelib %(%{__python2} -c "from distutils.sysconfig import get_python_lib; print(get_python_lib())")}
%{!?python2_sitearch: %global python2_sitearch %(%{__python2} -c "from distutils.sysconfig import get_python_lib; print(get_python_lib(1))")}
%endif

%global commit aa992b96273e6042660111d594ee62108cea52ae
%global shortcommit %(c=%{commit}; echo ${c:0:7})

Name:           graphite-web
Version:        0.9.13
Release:        0.4.%{shortcommit}%{?dist}

Summary:        A Django web application for enterprise scalable realtime graphing
Group:          Applications/Internet
License:        ASL 2.0
URL:            https://github.com/graphite-project

Source0:        https://github.com/graphite-project/graphite-web/archive/%{commit}/%{name}-%{commit}.tar.gz
Source1:        graphite-web-vhost.conf
Source2:        graphite-web-README.fedora
Source10:       %{name}.logrotate
Patch0:         %{name}-0.9.13-Amend-default-filesystem-locations.patch
Patch1:         %{name}-0.9.13-Force-use-of-system-libraries.patch
Patch5:         %{name}-0.9.13-Fix-build-index.sh-variables.patch
Patch7:         %{name}-0.9.13-Disable-internal-log-rotation.patch

BuildArch:      noarch
BuildRequires:  python-devel

Requires:       dejavu-sans-fonts
Requires:       dejavu-serif-fonts
Requires:       django-tagging
Requires:       mod_wsgi
Requires:       pycairo
Requires:       pyparsing
Requires:       python-simplejson
Requires:       python-whisper >= %{version}
Requires:       pytz

%if 0%{?fedora} >= 18 || 0%{?rhel} >= 7
Requires:       python-django >= 1.3
%else
Requires:       Django >= 1.3
%endif

%if 0%{?el5}
Requires:       python-sqlite2
%endif

Obsoletes:      %{name}-selinux < 0.9.12-7


%description
Graphite consists of a storage backend and a web-based visualization frontend.
Client applications send streams of numeric time-series data to the Graphite
backend (called carbon), where it gets stored in fixed-size database files
similar in design to RRD. The web frontend provides user interfaces
for visualizing this data in graphs as well as a simple URL-based API for
direct graph generation.

Graphite's design is focused on providing simple interfaces (both to users and
applications), real-time visualization, high-availability, and enterprise
scalability.


%prep
%setup -q -n graphite-web-%{commit}
rm -rf webapp/graphite/thirdparty
find -type f -iname '*.swf' -delete

install -m0644 %{SOURCE2} README.fedora

# Amend default filesystem locations.
%patch0 -p1

# Force use of system libraries.
%patch1 -p1

# Fix filesystem locations for variables in build-index.sh.
%patch5 -p1

# Disable internal log rotation.
%patch7 -p1

%build
%{__python2} setup.py build

%install
%{__python2} setup.py install --optimize=1 --skip-build --root %{buildroot} \
    --install-lib=%{python_sitelib} \
    --install-data=%{_datadir}/graphite \
    --install-scripts=%{_bindir}

mkdir -p %{buildroot}%{_localstatedir}/lib/graphite-web
mkdir -p %{buildroot}%{_localstatedir}/log/graphite-web
mkdir -p %{buildroot}%{_sysconfdir}/graphite-web

install -Dp -m0644 webapp/graphite/local_settings.py.example \
    %{buildroot}%{_sysconfdir}/graphite-web/local_settings.py
ln -s %{_sysconfdir}/graphite-web/local_settings.py \
    %{buildroot}%{python_sitelib}/graphite/local_settings.py
install -Dp -m0644 conf/dashboard.conf.example  \
    %{buildroot}%{_sysconfdir}/graphite-web/dashboard.conf
install -Dp -m0644 %{SOURCE1} \
    %{buildroot}%{_sysconfdir}/httpd/conf.d/graphite-web.conf
install -Dp -m0644 conf/graphite.wsgi.example \
    %{buildroot}%{_datadir}/graphite/graphite-web.wsgi

# Configure django /media/ location.
sed -i 's|##PYTHON_SITELIB##|%{python_sitelib}|' \
    %{buildroot}%{_sysconfdir}/httpd/conf.d/graphite-web.conf

# Log rotation.
install -D -p -m0644 %{SOURCE10} %{buildroot}%{_sysconfdir}/logrotate.d/%{name}

# Remove unneeded binaries.
rm -f %{buildroot}%{_bindir}/run-graphite-devel-server.py

# Rename build-index.sh.
mv %{buildroot}%{_bindir}/build-index.sh %{buildroot}%{_bindir}/graphite-build-index

# Make manage.py available at an easier location.
ln -s %{python_sitelib}/graphite/manage.py \
    %{buildroot}%{_bindir}/graphite-manage

# Fix permissions.
chmod 0644 conf/graphite.wsgi.example
chmod 0755 %{buildroot}%{python_sitelib}/graphite/manage.py
chmod 0644 %{buildroot}%{_datadir}/graphite/webapp/content/js/window/*


%files
%doc README.fedora LICENSE conf/* examples/*

%dir %{_sysconfdir}/graphite-web
%config(noreplace) %{_sysconfdir}/httpd/conf.d/graphite-web.conf
%config(noreplace) %{_sysconfdir}/graphite-web/dashboard.conf
%config(noreplace) %{_sysconfdir}/graphite-web/local_settings.py*
%config(noreplace) %{_sysconfdir}/logrotate.d/%{name}

%{_bindir}/graphite-build-index
%{_bindir}/graphite-manage
%{_datadir}/graphite
%attr(0755,apache,apache) %dir %{_localstatedir}/lib/graphite-web
%attr(0755,apache,apache) %dir %{_localstatedir}/log/graphite-web

%{python_sitelib}/graphite/
%{python_sitelib}/graphite_web-*-py?.?.egg-info


%changelog
* Wed Jun 17 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.9.13-0.4.aa992b9
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Tue Feb 17 2015 Piotr Popieluch <piotr1212@gmail.com> - 0.9.13-0.3.aa992b9
- fix IE 10 javascript issues

* Thu Feb  5 2015 Piotr Popieluch <piotr1212@gmail.com> - 0.9.13-0.2.094cf54
- update to later commit to fix XSS

* Mon Jan 19 2015 Piotr Popieluch <piotr1212@gmail.com> - 0.9.13-0.1.pre1
- update to upstream pre-release

* Fri Nov 14 2014 Jamie Nguyen <jamielinux@fedoraproject.org> - 0.9.12-8
- obsolete hacky graphite-web-selinux subpackage
- remove EPEL 5 related packaging things

* Wed Oct 01 2014 Jamie Nguyen <jamielinux@fedoraproject.org> - 0.9.12-7
- update URL
- use commit hash for Source URL
- package should own /etc/graphite-web
- do not ghost .pyc and .pyo files
- remove thirdparty libs and .swf files in %%prep
- split fhs+thirdparty patch into two discrete patches
- be more explicit in %%files
- include python egg
- include build-index.sh script (renamed to /usr/bin/graphite-build-index)
- make manage.py available at /usr/bin/graphite-manage
- patch for Django 1.5
- disable internal log rotation and use system logrotate
- apache needs httpd_sys_rw_content_t permissions instead of httpd_sys_content_t
- improve vhost configuration (including a fix for #1141701)

* Sat Jun 07 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.9.12-6
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Tue Oct 01 2013 Jonathan Steffan <jsteffan@fedoraproject.org> - 0.9.12-5
- Patch for fix loading dashboards by name (RHBZ#1014349)
- Patch for log name of metric that throws exception for CarbonLink (RHBZ#1014349)
- Add deque to the PICKLE_SAFE filter (RHBZ#1014356)
- Merge in EL5 conditionals for single spec

* Mon Sep 30 2013 Jonathan Steffan <jsteffan@fedoraproject.org> - 0.9.12-4
- Remove logrotate configuration as it conflicts with internal
  log rotation (RHBZ#1008616)

* Tue Sep 24 2013 Jonathan Steffan <jsteffan@fedoraproject.org> - 0.9.12-3
- Reorder Requires conditionals to fix amzn1 issues (RHBZ#1007300)
- Ensure python-whisper is also updated

* Tue Sep 17 2013 Jonathan Steffan <jsteffan@fedoraproject.org> - 0.9.12-2
- Don't ship js/ext/resources/*.swf (RHBZ#1000253)

* Mon Sep 02 2013 Jonathan Steffan <jsteffan@fedoraproject.org> - 0.9.12-1
- Update to 0.9.12
- Require Django >= 1.3
- Add EL5 conditional for SELinux policycoreutils

* Sat Aug 03 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.9.10-8
- Rebuilt for https://fedoraproject.org/wiki/Fedora_20_Mass_Rebuild

* Wed Mar 13 2013 Jonathan Steffan <jsteffan@fedoraproject.org> - 0.9.10-7
- Update required fonts to actually include fonts (RHBZ#917361)

* Thu Feb 14 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.9.10-6
- Rebuilt for https://fedoraproject.org/wiki/Fedora_19_Mass_Rebuild

* Sun Dec 30 2012 Jonathan Steffan <jsteffan@fedoraproject.org> - 0.9.10-5
- Conditionally require python-sqlite2
- Conditionally require new Django namespace

* Sat Dec 29 2012 Jonathan Steffan <jsteffan@fedoraproject.org> - 0.9.10-4
- Update to use mod_wsgi
- Update vhost configuration file to correctly work on multiple python
  versions

* Sat Nov 24 2012 Jonathan Steffan <jsteffan@fedoraproject.org> - 0.9.10-3
- Address all rpmlint errors
- Add SELinux subpackage README
- Patch out thirdparty code, Require it instead

* Fri Nov 09 2012 Jonathan Steffan <jsteffan@fedoraproject.org> - 0.9.10-2
- Add logrotate

* Thu May 31 2012 Jonathan Steffan <jsteffan@fedoraproject.org> - 0.9.10-1
- Initial Package
