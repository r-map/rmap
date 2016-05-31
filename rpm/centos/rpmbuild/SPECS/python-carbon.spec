%if 0%{?rhel} && 0%{?rhel} <= 6
%{!?__python2: %global __python2 /usr/bin/python2}
%{!?python2_sitelib: %global python2_sitelib %(%{__python2} -c "from distutils.sysconfig import get_python_lib; print(get_python_lib())")}
%{!?python2_sitearch: %global python2_sitearch %(%{__python2} -c "from distutils.sysconfig import get_python_lib; print(get_python_lib(1))")}
%endif

%global commit e69e1eb59aaade325d37e68d920d786f280d294a

# Switched to system log rotation on Fedora 21 and EPEL 7.
# systemd service files introduced in Fedora 21 and EPEL 7.
%if 0%{?fedora} >= 21 || 0%{?rhel} >= 7
%global with_system_logrotate 1
%global with_systemd 1
%else
%global with_system_logrotate 0
%global with_systemd 0
%endif

Name:           python-carbon
Version:        0.9.13
Release:        0.2.pre1%{?dist}

Summary:        Back-end data caching and persistence daemon for Graphite
Group:          System Environment/Daemons
License:        ASL 2.0
URL:            https://github.com/graphite-project

Source0:        https://github.com/graphite-project/carbon/archive/%{commit}/%{name}-%{commit}.tar.gz
Source10:       carbon-aggregator.1
Source11:       carbon-cache.1
Source12:       carbon-client.1
Source13:       carbon-relay.1
Source14:       validate-storage-schemas.1
Source20:       %{name}.logrotate

Source30:       carbon-aggregator.service
Source31:       carbon-cache.service
Source32:       carbon-relay.service

Source40:       %{name}-cache.init
Source41:       %{name}-relay.init
Source42:       %{name}-aggregator.init
Source43:       %{name}.sysconfig

Patch0:         %{name}-0.9.13-Do-not-install-upstream-init-scripts.patch
Patch1:         %{name}-0.9.13-Set-sane-defaults.patch
Patch2:         %{name}-0.9.13-Fix-path-to-storage-schemas.conf.patch

BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildArch:      noarch
BuildRequires:  python-devel
BuildRequires:  python-setuptools

Requires:       python-twisted-core >= 8.0
Requires:       python-whisper >= %{version}

Requires(pre):    shadow-utils
%if %{with_systemd}
BuildRequires:    systemd
Requires(post):   systemd
Requires(preun):  systemd
Requires(postun): systemd
%else
Requires(post):   chkconfig
Requires(preun):  chkconfig, initscripts
%endif


%description
Carbon is one of the components of Graphite, and is responsible for
receiving metrics over the network and writing them down to disk using
a storage back-end.


%prep
%setup -q -n carbon-%{commit}

# Install our own init scripts
%patch0 -p1

# Set sane default filesystem paths.
%patch1 -p1

# Fix path to storage-schemas.conf.
%patch2 -p1

%if %{with_system_logrotate}
# Disable internal log rotation.
sed -i -e 's/ENABLE_LOGROTATION.*/ENABLE_LOGROTATION = False/g' \
    conf/carbon.conf.example
%endif


%build
%{__python} setup.py build


%install
rm -rf %{buildroot}
%{__python} setup.py install \
    --optimize=1 --skip-build \
    --root=%{buildroot} \
    --install-data=%{_localstatedir}/lib/carbon \
    --install-lib=%{python_sitelib} \
    --install-scripts=%{_bindir}

rm -rf %{buildroot}%{_localstatedir}/lib/carbon/*
mkdir -p %{buildroot}%{_localstatedir}/lib/carbon/lists
mkdir -p %{buildroot}%{_localstatedir}/lib/carbon/rrd
mkdir -p %{buildroot}%{_localstatedir}/lib/carbon/whisper

# default config
mkdir -p %{buildroot}%{_sysconfdir}/carbon
install -D -p -m0644 conf/carbon.conf.example \
    %{buildroot}%{_sysconfdir}/carbon/carbon.conf
install -D -p -m0644 conf/storage-schemas.conf.example \
    %{buildroot}%{_sysconfdir}/carbon/storage-schemas.conf

# man pages
mkdir -p %{buildroot}%{_mandir}/man1
install -D -p -m0644 %{SOURCE10} %{buildroot}%{_mandir}/man1
install -D -p -m0644 %{SOURCE11} %{buildroot}%{_mandir}/man1
install -D -p -m0644 %{SOURCE12} %{buildroot}%{_mandir}/man1
install -D -p -m0644 %{SOURCE13} %{buildroot}%{_mandir}/man1
install -D -p -m0644 %{SOURCE14} %{buildroot}%{_mandir}/man1

# log files
mkdir -p %{buildroot}%{_localstatedir}/log/carbon
%if %{with_system_logrotate}
install -D -p -m0644 %{SOURCE20} \
    %{buildroot}%{_sysconfdir}/logrotate.d/%{name}
%endif

# init scripts
%if %{with_systemd}
install -D -p -m0644 %{SOURCE30} \
    %{buildroot}%{_unitdir}/carbon-aggregator.service
install -D -p -m0644 %{SOURCE31} \
    %{buildroot}%{_unitdir}/carbon-cache.service
install -D -p -m0644 %{SOURCE32} \
    %{buildroot}%{_unitdir}/carbon-relay.service
%else
mkdir -p %{buildroot}%{_localstatedir}/run/carbon
install -Dp -m0755 %{SOURCE40} \
    %{buildroot}%{_sysconfdir}/init.d/carbon-cache
install -Dp -m0755 %{SOURCE41} \
    %{buildroot}%{_sysconfdir}/init.d/carbon-relay
install -Dp -m0755 %{SOURCE42} \
    %{buildroot}%{_sysconfdir}/init.d/carbon-aggregator
install -Dp -m0644 %{SOURCE43} \
    %{buildroot}%{_sysconfdir}/sysconfig/carbon
%endif

# remove .py suffix
for i in %{buildroot}%{_bindir}/*.py; do
    mv ${i} ${i%%.py}
done

# fix permissions
chmod 755 %{buildroot}%{python_sitelib}/carbon/amqp_listener.py
chmod 755 %{buildroot}%{python_sitelib}/carbon/amqp_publisher.py


%pre
getent group carbon >/dev/null || groupadd -r carbon
getent passwd carbon >/dev/null || \
    useradd -r -g carbon -d %{_localstatedir}/lib/carbon \
    -s /sbin/nologin -c "Carbon cache daemon" carbon

%post
%if %{with_systemd}
%systemd_post carbon-aggregator.service
%systemd_post carbon-cache.service
%systemd_post carbon-relay.service
%else
/sbin/chkconfig --add carbon-cache
/sbin/chkconfig --add carbon-relay
/sbin/chkconfig --add carbon-aggregator
%endif

%preun
%if %{with_systemd}
%systemd_preun carbon-aggregator.service
%systemd_preun carbon-cache.service
%systemd_preun carbon-relay.service
%else
if [ $1 -eq 0 ]; then
    /sbin/service carbon-cache stop >/dev/null 2>&1
    /sbin/chkconfig --del carbon-cache
    /sbin/service carbon-relay stop >/dev/null 2>&1
    /sbin/chkconfig --del carbon-relay
    /sbin/service carbon-aggregator stop >/dev/null 2>&1
    /sbin/chkconfig --del carbon-aggregator
fi
%endif

%if %{with_systemd}
%postun
%systemd_postun_with_restart carbon-aggregator.service
%systemd_postun_with_restart carbon-cache.service
%systemd_postun_with_restart carbon-relay.service
%endif


%files
%doc LICENSE README.md
%doc conf/ examples/

%dir %{_sysconfdir}/carbon
%config(noreplace) %{_sysconfdir}/carbon/carbon.conf
%config(noreplace) %{_sysconfdir}/carbon/storage-schemas.conf

%if %{with_system_logrotate}
%config(noreplace) %{_sysconfdir}/logrotate.d/%{name}
%endif

%attr(0755,carbon,carbon) %dir %{_localstatedir}/lib/carbon
%attr(0755,carbon,carbon) %dir %{_localstatedir}/lib/carbon/lists
%attr(0755,carbon,carbon) %dir %{_localstatedir}/lib/carbon/rrd
%attr(0755,carbon,carbon) %dir %{_localstatedir}/lib/carbon/whisper
%attr(0755,carbon,carbon) %dir %{_localstatedir}/log/carbon

%{_bindir}/carbon-aggregator
%{_bindir}/carbon-cache
%{_bindir}/carbon-client
%{_bindir}/carbon-relay
%{_bindir}/validate-storage-schemas

%{_mandir}/man1/carbon-aggregator.1*
%{_mandir}/man1/carbon-cache.1*
%{_mandir}/man1/carbon-client.1*
%{_mandir}/man1/carbon-relay.1*
%{_mandir}/man1/validate-storage-schemas.1*

%{python_sitelib}/carbon
%{python_sitelib}/twisted

%if 0%{?fedora} || 0%{?rhel} > 5
%{python_sitelib}/carbon-*-py?.?.egg-info
%endif

%if %{with_systemd}
%{_unitdir}/carbon-aggregator.service
%{_unitdir}/carbon-cache.service
%{_unitdir}/carbon-relay.service
%else
%dir %{_localstatedir}/run/carbon
%{_sysconfdir}/init.d/carbon-*
%config(noreplace) %{_sysconfdir}/sysconfig/carbon
%endif


%changelog
* Thu Jun 18 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.9.13-0.2.pre1
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Mon Jan 19 2015 Piotr Popieluch <piotr1212@gmail.com> - 0.9.13-0.1.pre1
- update to 0.9.13-pre1

* Mon Nov 24 2014 Jamie Nguyen <jamielinux@fedoraproject.org> - 0.9.12-7
- patch setup.py to prevent installation of upstream init scripts

* Fri Nov 14 2014 Jamie Nguyen <jamielinux@fedoraproject.org> - 0.9.12-6
- conditionally define macros for EPEL 6 and below

* Wed Oct 01 2014 Jamie Nguyen <jamielinux@fedoraproject.org> - 0.9.12-5
- update URL
- improve description
- use commit hash for Source URL
- use loop to rename files
- include README.md and examples/
- amend patch for filesystem default paths
- fix path to storage-schemas.conf
- add man pages from Debian
- disable internal log rotation and include logrotate configuration
  for Fedora >= 21 and EPEL >= 7
- be more explicit in %%files
- include python egg
- migrate to systemd on Fedora >= 21 and EPEL >= 7

* Sat Jun 07 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.9.12-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Mon Sep 30 2013 Jonathan Steffan <jsteffan@fedoraproject.org> - 0.9.12-3
- Update default runtime user to carbon for carbon-aggregator and
  carbon-relay (RHBZ#1013813)

* Tue Sep 24 2013 Jonathan Steffan <jsteffan@fedoraproject.org> - 0.9.12-2
- Add strict python-whisper Requires (RHBZ#1010432)
- Don't cleanup user and user data on package remove (RHBZ#1010430)

* Mon Sep 02 2013 Jonathan Steffan <jsteffan@fedoraproject.org> - 0.9.12-1
- Update to 0.9.12

* Sun Aug 04 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.9.10-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_20_Mass_Rebuild

* Thu Feb 14 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.9.10-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_19_Mass_Rebuild

* Sat Nov 24 2012 Jonathan Steffan <jsteffan@fedoraproject.org> - 0.9.10-2
- Update spec to build on el5
- Fix python_sitelib definition

* Wed May 30 2012 Jonathan Steffan <jsteffan@fedoraproject.org> - 0.9.10-1
- Initial Package
