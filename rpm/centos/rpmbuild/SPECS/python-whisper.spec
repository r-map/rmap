%if 0%{?rhel} && 0%{?rhel} <= 6
%{!?__python2: %global __python2 /usr/bin/python2}
%{!?python2_sitelib: %global python2_sitelib %(%{__python2} -c "from distutils.sysconfig import get_python_lib; print(get_python_lib())")}
%{!?python2_sitearch: %global python2_sitearch %(%{__python2} -c "from distutils.sysconfig import get_python_lib; print(get_python_lib(1))")}
%endif

%global commit 142f5493972db05c5c63ef7691ff71a8aeb128a4

Name:           python-whisper
Version:        0.9.13
Release:        0.2.pre1%{?dist}
Summary:        Simple database library for storing time-series data

Group:          Development/Libraries
License:        ASL 2.0
URL:            https://github.com/graphite-project

Source0:        https://github.com/graphite-project/whisper/archive/%{commit}/%{name}-%{commit}.tar.gz
Source10:       rrd2whisper.1
Source11:       whisper-create.1
Source12:       whisper-dump.1
Source13:       whisper-fetch.1
Source14:       whisper-info.1
Source15:       whisper-merge.1
Source16:       whisper-resize.1
Source17:       whisper-set-aggregation-method.1
Source18:       whisper-update.1
Source19:       whisper-fill.1

BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildArch:      noarch
BuildRequires:  python-devel
BuildRequires:  python-setuptools


%description
Whisper is a fixed-size database, similar in design and purpose to RRD
(round-robin-database). It provides fast, reliable storage of numeric
data over time. Whisper allows for higher resolution (seconds per point)
of recent data to degrade into lower resolutions for long-term retention
of historical data.


%prep
%setup -q -n whisper-%{commit}


%build
%{__python} setup.py build


%install
rm -rf $RPM_BUILD_ROOT
%{__python} setup.py install -O1 --skip-build --root $RPM_BUILD_ROOT

# man pages
mkdir -p $RPM_BUILD_ROOT%{_mandir}/man1
install -D -p -m0644 %{SOURCE10} $RPM_BUILD_ROOT%{_mandir}/man1
install -D -p -m0644 %{SOURCE11} $RPM_BUILD_ROOT%{_mandir}/man1
install -D -p -m0644 %{SOURCE12} $RPM_BUILD_ROOT%{_mandir}/man1
install -D -p -m0644 %{SOURCE13} $RPM_BUILD_ROOT%{_mandir}/man1
install -D -p -m0644 %{SOURCE14} $RPM_BUILD_ROOT%{_mandir}/man1
install -D -p -m0644 %{SOURCE15} $RPM_BUILD_ROOT%{_mandir}/man1
install -D -p -m0644 %{SOURCE16} $RPM_BUILD_ROOT%{_mandir}/man1
install -D -p -m0644 %{SOURCE17} $RPM_BUILD_ROOT%{_mandir}/man1
install -D -p -m0644 %{SOURCE18} $RPM_BUILD_ROOT%{_mandir}/man1
install -D -p -m0644 %{SOURCE19} $RPM_BUILD_ROOT%{_mandir}/man1

# remove .py suffix
for i in $RPM_BUILD_ROOT%{_bindir}/*.py; do
    mv ${i} ${i%%.py}
done


%files
%doc LICENSE
%{_bindir}/rrd2whisper
%{_bindir}/whisper-create
%{_bindir}/whisper-dump
%{_bindir}/whisper-fetch
%{_bindir}/whisper-fill
%{_bindir}/whisper-info
%{_bindir}/whisper-merge
%{_bindir}/whisper-resize
%{_bindir}/whisper-set-aggregation-method
%{_bindir}/whisper-update
%{_mandir}/man1/rrd2whisper.1*
%{_mandir}/man1/whisper-create.1*
%{_mandir}/man1/whisper-dump.1*
%{_mandir}/man1/whisper-fetch.1*
%{_mandir}/man1/whisper-fill.1*
%{_mandir}/man1/whisper-info.1*
%{_mandir}/man1/whisper-merge.1*
%{_mandir}/man1/whisper-resize.1*
%{_mandir}/man1/whisper-set-aggregation-method.1*
%{_mandir}/man1/whisper-update.1*
%{python_sitelib}/whisper.py*

%if 0%{?fedora} || 0%{?rhel} > 5
%{python_sitelib}/whisper-*-py?.?.egg-info
%endif


%changelog
* Thu Jun 18 2015 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.9.13-0.2.pre1
- Rebuilt for https://fedoraproject.org/wiki/Fedora_23_Mass_Rebuild

* Mon Jan 19 2015 Piotr Popieluch <piotr1212@gmail.com> - 0.9.13-0.1.pre1
- update to 0.9.13-pre1

* Fri Nov 14 2014 Jamie Nguyen <jamielinux@fedoraproject.org> - 0.9.12-4
- conditionally define macros for EPEL 6 and below

* Wed Oct 01 2014 Jamie Nguyen <jamielinux@fedoraproject.org> - 0.9.12-3
- update URL
- improve description
- specify commit hash in Source URL
- include man pages from Debian
- include missing LICENSE file
- include python egg
- use loop to rename files
- be more explicit in %%files

* Sun Jun 08 2014 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.9.12-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_21_Mass_Rebuild

* Mon Sep 02 2013 Jonathan Steffan <jsteffan@fedoraproject.org> - 0.9.12-1
- Update to 0.9.12

* Sun Aug 04 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.9.10-4
- Rebuilt for https://fedoraproject.org/wiki/Fedora_20_Mass_Rebuild

* Thu Feb 14 2013 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 0.9.10-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_19_Mass_Rebuild

* Sun Sep 16 2012 Jonathan Steffan <jsteffan@fedoraproject.org> - 0.9.10-2
- Add group to be able to build against EPEL5

* Thu May 31 2012 Jonathan Steffan <jsteffan@fedoraproject.org> - 0.9.10-1
- Initial Package
