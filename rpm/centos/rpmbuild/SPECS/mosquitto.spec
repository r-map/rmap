Name:       mosquitto
Version:    1.4.7
Release:    1

License:    BSD
Group:      Productivity/Networking/Other

BuildRoot:  %{_tmppath}/%{name}-%{version}-build

URL:        http://mosquitto.org
Source:     http://mosquitto.org/files/source/mosquitto-%{version}.tar.gz
%if %{defined suse_version}
Source1:    mosquitto.init.d.suse
%else
Source1:    mosquitto.init.d
%endif
Source2:    mosquitto.fw
Source3:    mosquitto.apparmor
Source4:    README-conf-d
Source5:    README-ca_certificates
Source6:    README-certs
Source7:    mosquitto.conf

Summary:    MQTT version 3.1/3.1.1 compatible message broker

%define fedora_version 20

%define SRV %nil
%define ECDH %nil
%define UUID %nil

%if "%_lib" == "lib64"
%define LIB_SUFFIX 64
%else
%define LIB_SUFFIX %nil
%endif

%if %{defined suse_version}
Requires:  tcpd, libopenssl1_0_0, libcares2, libuuid1
BuildRequires:  tcpd-devel, gcc-c++, python, pwdutils, libopenssl-devel, libcares-devel, libuuid-devel, c-ares-devel, tcp_wrappers-devel, e2fsprogs-devel

%define _fwdefdir /etc/sysconfig/SuSEfirewall2.d/services
%py_requires
PreReq: %insserv_prereq %fillup_prereq

%endif

%if %{defined rhel_version}

Requires:   tcp_wrappers, openssl, uuid
BuildRequires:  tcp_wrappers-devel, gcc-c++, python, openssl-devel, libuuid-devel
%define SRV WITH_SRV=no
#%define ECDH WITH_EC=no
%endif


%if %{defined centos_version}
Requires:   tcp_wrappers, openssl, uuid
BuildRequires:  tcp_wrappers-devel, gcc-c++, python, openssl-devel, libuuid-devel
%define SRV WITH_SRV=no
#%define ECDH WITH_EC=no
%endif

%if %{defined fedora_version}
Requires:   tcp_wrappers, openssl, c-ares, uuid
Requires(post): chkconfig
Requires(postun): initscripts
Requires(preun): chkconfig
Requires(preun): initscripts
BuildRequires:  tcp_wrappers-devel, gcc-c++, python, python-devel, openssl-devel, c-ares-devel, uuid-devel
%define UUID WITH_UUID=no
%endif

%if %{defined mdkversion}
Requires:  libwrap0, libopenssl1_0_0, c-ares
BuildRequires:  libwrap-devel, gcc-c++, python, python-devel, libopenssl-devel, c-ares-devel
%endif

%description
A message broker that supports version 3.1 of the MQ Telemetry Transport  
protocol. MQTT provides a method of carrying out messaging using a   
publish/subscribe model. It is lightweight, both in terms of bandwidth   
usage and ease of implementation. This makes it particularly useful at  
the edge of the network where simple embedded devices are in use, such   
as an arduino implementing a sensor.  

%package clients
Summary: Mosquitto command line publish/subscribe clients
Group: Productivity/Networking/Other

%description clients
This is two MQTT version 3.1 command line clients. mosquitto_pub can be used
to publish messages to a broker and mosquitto_sub can be used to subscribe to
a topic to receive messages.

%package -n libmosquitto1
Summary: MQTT C client library
Group: Development/Libraries/C and C++

%description -n libmosquitto1
This is a library that provides a means of implementing MQTT version 3.1
clients. MQTT provides a lightweight method of carrying out messaging using a
publish/subscribe model.

%package -n libmosquitto-devel
Summary: MQTT C client library development files
Requires: libmosquitto1
Group: Development/Libraries/C and C++

%description -n libmosquitto-devel
This is a library that provides a means of implementing MQTT version 3.1
clients. MQTT provides a lightweight method of carrying out messaging using a
publish/subscribe model.

%package -n libmosquittopp1
Summary: MQTT C++ client library
Group: Development/Libraries/C and C++

%description -n libmosquittopp1
This is a library that provides a means of implementing MQTT version 3.1
clients. MQTT provides a lightweight method of carrying out messaging using a
publish/subscribe model.

%package -n libmosquittopp-devel
Summary: MQTT C++ client library development files
Group: Development/Libraries/C and C++

%description -n libmosquittopp-devel
This is a library that provides a means of implementing MQTT version 3.1
clients. MQTT provides a lightweight method of carrying out messaging using a
publish/subscribe model.

%prep
%setup -q

%build
%if 0%{?sles_version} == 10 || 0%{?sles_version} == 11 || 0%{?rhel_version} == 406 || 0%{?rhel_version} == 505 || 0%{?centos_version} == 505 
make WITH_WRAP=yes WITH_TLS_PSK=no
%else
make WITH_WRAP=yes %SRV %ECDH %UUID
%endif
#make test


%install
make install DESTDIR=$RPM_BUILD_ROOT prefix=/usr LIB_SUFFIX=%{LIB_SUFFIX}
%if %{defined fedora_version}
install -d $RPM_BUILD_ROOT/etc/event.d
cp service/upstart/mosquitto.conf $RPM_BUILD_ROOT/etc/event.d/mosquitto
%endif

install -d $RPM_BUILD_ROOT/etc/init.d
%if %{defined suse_version}
install -d $RPM_BUILD_ROOT/etc/apparmor.d
#install firewall definitions format is described here:
#/usr/share/SuSEfirewall2/services/TEMPLATE
mkdir -p $RPM_BUILD_ROOT/%{_fwdefdir}
install -m 644 %SOURCE2 $RPM_BUILD_ROOT/%{_fwdefdir}/mosquitto
install -m 644 %SOURCE3 $RPM_BUILD_ROOT/etc/apparmor.d/usr.sbin.mosquitto
%endif

install -d $RPM_BUILD_ROOT/etc/mosquitto/conf.d/
install -m 644 %SOURCE4 $RPM_BUILD_ROOT/etc/mosquitto/conf.d/README
install -d $RPM_BUILD_ROOT/etc/mosquitto/ca_certificates/
install -m 644 %SOURCE5 $RPM_BUILD_ROOT/etc/mosquitto/ca_certificates/README
install -d $RPM_BUILD_ROOT/etc/mosquitto/certs/
install -m 644 %SOURCE6 $RPM_BUILD_ROOT/etc/mosquitto/certs/README
install -m 644 %SOURCE7 $RPM_BUILD_ROOT/etc/mosquitto/mosquitto.conf


install -d $RPM_BUILD_ROOT/var/lib/mosquitto
install -m 0755 %SOURCE1 $RPM_BUILD_ROOT/etc/init.d/mosquitto
ln -sf /etc/init.d/mosquitto $RPM_BUILD_ROOT/usr/sbin/rcmosquitto

%clean
rm -rf $RPM_BUILD_ROOT

%pre
# User needs to be present before install so permissions can be set.
getent group mosquitto >/dev/null || /usr/sbin/groupadd -r mosquitto  
getent passwd mosquitto >/dev/null || /usr/sbin/useradd -r -g mosquitto -d /var/lib/mosquitto -s /bin/false -c "Mosquitto broker" mosquitto  

%post
%if %{defined suse_version}
%fillup_and_insserv mosquitto
%restart_on_update mosquitto
%endif

%if %{defined fedora_version}
/sbin/chkconfig --add mosquitto
%endif

%preun
%if %{defined suse_version}
%stop_on_removal mosquitto
%endif

%if %{defined fedora_version}
if [ $1 = 0 ] ; then
    /sbin/service mosquitto stop >/dev/null 2>&1
    /sbin/chkconfig --del mosquitto
fi
%endif

%postun
%if %{defined suse_version}
%insserv_cleanup
%endif


if [ $1 -eq 0 ]
then
    userdel -r mosquitto
fi

%post -n libmosquitto1
/sbin/ldconfig

%postun -n libmosquitto1
/sbin/ldconfig

%post -n libmosquittopp1
/sbin/ldconfig

%postun -n libmosquittopp1
/sbin/ldconfig

%files
%defattr(-,root,root,-)
/usr/sbin/mosquitto
/usr/bin/mosquitto_passwd
/etc/init.d/mosquitto  
/usr/sbin/rcmosquitto
/etc/mosquitto
/etc/mosquitto/conf.d/README
/etc/mosquitto/ca_certificates/README
/etc/mosquitto/certs/README
%config /etc/mosquitto/mosquitto.conf
%config /etc/mosquitto/mosquitto.conf.example
%config /etc/mosquitto/pwfile.example
%config /etc/mosquitto/aclfile.example
/usr/include/mosquitto_plugin.h
%if %{defined fedora_version}
/etc/event.d/mosquitto
%endif
%if %{defined suse_version}
/etc/apparmor.d  
/etc/apparmor.d/usr.sbin.mosquitto  
%config %{_fwdefdir}/mosquitto  
%endif
%doc /usr/share/man/man5/mosquitto.conf.5.*
%doc /usr/share/man/man7/mqtt.7.*
%doc /usr/share/man/man7/mosquitto-tls.7.*
%doc /usr/share/man/man1/mosquitto_passwd.1.*
%doc /usr/share/man/man8/mosquitto.8.*
%attr(755,mosquitto,mosquitto) /var/lib/mosquitto/

%files clients
%defattr(-,root,root,-)
/usr/bin/mosquitto_pub
/usr/bin/mosquitto_sub
%doc /usr/share/man/man1/mosquitto_pub.1.*
%doc /usr/share/man/man1/mosquitto_sub.1.*

%files -n libmosquitto1
%defattr(-,root,root,-)
%{_libdir}/libmosquitto.so.*
%doc /usr/share/man/man3/libmosquitto.3.*

%files -n libmosquitto-devel
%defattr(-,root,root,-)
/usr/include/mosquitto.h
%{_libdir}/libmosquitto.so

%files -n libmosquittopp1
%defattr(-,root,root,-)
%{_libdir}/libmosquittopp.so.*

%files -n libmosquittopp-devel
%defattr(-,root,root,-)
/usr/include/mosquittopp.h
%{_libdir}/libmosquittopp.so

%changelog
