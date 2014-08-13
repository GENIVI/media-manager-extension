Name:     mediamanager-extension
Version:  0.1
Release:  1
Summary:  Media Manager CrossWalk Tizen Extension
Group:    System/Libraries

License:    MPL-2.0
URL:      http://git.projects.genivi.org/?p=media-manager.git
Source0:  %{name}-%{version}.tar.gz

Requires: crosswalk jansson
BuildRequires: cmake
BuildRequires: git
BuildRequires: jansson-devel
BuildRequires: jansson
BuildRequires: CommonAPI
BuildRequires: CommonAPI-DBus
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: dbus-genivi

%description
Media Manager CrossWalk Tizen Extension. Interfaces with GENIVI MediaManager

%prep
%setup -q
tar xvfz interfaces.tar.gz

%build
export PKG_CONFIG_PATH=/opt/genivi/lib/pkgconfig/
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=%{_prefix} ..

make

%install
cd build
make install DESTDIR=%{buildroot} PREFIX=%{_prefix}

%files
%{_prefix}/lib/tizen-extensions-crosswalk/libmediamanager.so
