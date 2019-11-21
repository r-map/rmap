%define srcname django-imagekit
%define version 4.0.1
%define release 4

Name: python3-%{srcname}
Version: %{version}
Release: %{release}
Summary: Automated image processing for Django models.
Source0: %{srcname}-%{version}.tar.gz
License: BSD
Group: Development/Libraries
BuildArch: noarch
Vendor: Bryan Veloso <bryan@revyver.com>
Url: http://github.com/matthewwithanm/django-imagekit/
#BuildRequires: python2-devel
BuildRequires: python3-devel
Requires: python3-pilkit

%description
ImageKit is a Django app for processing images. Need a thumbnail? A
black-and-white version of a user-uploaded image? ImageKit will make them for
you. If you need to programatically generate one image from another, you need
ImageKit.

ImageKit comes with a bunch of image processors for common tasks like resizing
and cropping, but you can also create your own. For an idea of what\'s possible,
check out the `Instakit`__ project.

**For the complete documentation on the latest stable version of ImageKit, see**
.. _`ImageKit on RTD`: http://django-imagekit.readthedocs.org
__ https://github.com/fish2000/instakit


%prep
%autosetup -n %{srcname}-%{version}

%build
%py3_build

%install
%py3_install

%check
#%{__python3} setup.py test

%clean
rm -rf $RPM_BUILD_ROOT


%files -n %{name}
%license LICENSE
%doc README.rst
%{python3_sitelib}/*


