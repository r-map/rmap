%global srcname kivy

Name:           python-kivy
Version:        1.10.1
Release:        2%{?dist}
License:        LGPL-3.0
Summary:        Kivy - Multimedia / Multitouch framework in Python
Url:            http://kivy.org
Source0:        https://pypi.python.org/packages/source/K/Kivy/Kivy-%{version}.tar.gz
BuildRequires:  gcc
BuildRequires:  gcc-c++
BuildRequires:  python2-devel
BuildRequires:  python3-devel
BuildRequires:  python2-Cython
BuildRequires:  python3-Cython
BuildRequires:  SDL2-devel
BuildRequires:  SDL2_image-devel
BuildRequires:  SDL2_mixer-devel
BuildRequires:  SDL2_ttf-devel
%ifarch armel arm
BuildRequires:  pkgconfig(glesv2)
%else
BuildRequires:  python3-pkgconfig
%endif
BuildRequires:  mesa-libGL-devel

Requires:       python3-imaging
Requires:       python3-pygame
Requires:       python3-gstreamer1
Requires:       python3-enchant

Provides:       python3-kivy
Provides:       pythonegg(kivy)

%global _description \
Kivy is an open source library for developing multi-touch applications. It is \
completely cross platform (Linux/OSX/Win/Android) and released under the terms \
of the GNU LGPL V3. \
\
It comes with native support for many multi-touch input devices, a growing \
library of multi-touch aware widgets, hardware accelerated OpenGL drawing, and \
an architecture that is designed to let you focus on building custom and highly \
interactive applications as quickly and easily as possible. \
\
Kivy is a mixed Python library with Cython code, to take advantage of its \
highly dynamic nature and use any of the thousands of high quality and open \
source python libraries out there, with the speed of C code.

%description %{_description}

%package -n python2-%{srcname}
Summary:        %{summary}

%description -n python2-%{srcname} %{_description}


%package -n python3-%{srcname}
Summary:        %{summary}

%description -n python3-%{srcname} %{_description}

%package examples
Summary:        Hardware-accelerated multitouch application library - Documentation
Group:          Documentation/Other
Requires:       %{name} = %{version}
Provides:       python-kivy-examples

%description examples
Kivy is an open source software library for rapid development of applications
that make use of innovative user interfaces, such as multi-touch apps.

This package contains the examples

%package doc
Summary:        Documentation for %{name}
Group:          Documentation/Other
Requires:       %{name} = %{version}

%description doc
Kivy is an open source software library for rapid development of applications
that make use of innovative user interfaces, such as multi-touch apps.

This package contains the developer documentation

%prep
%setup -q -n Kivy-%{version}

%build
%py2_build
%py3_build
#cd doc && make html && rm -r build/html/.buildinfo # Build HTML documentation

%install
%py2_install
%py3_install


%files -n python2-%{srcname}
%doc AUTHORS
%license LICENSE
%{python2_sitearch}/kivy/
%{python2_sitearch}/Kivy-%{version}-py*.egg-info

%files -n python3-%{srcname}
%doc AUTHORS
%license LICENSE
%{python3_sitearch}/kivy/
%{python3_sitearch}/Kivy-%{version}-py*.egg-info

%files examples
%{_datadir}/kivy-examples

%files doc
#%doc doc/build/html


%changelog
* Wed Jan 25 2017 Martin Gansser <martinkg@fedoraproject.org> - 1.9.1-1
- Initial build
