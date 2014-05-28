Name:     mediamanager-extension
Version:  0.1
Release:  1
Summary:  Media Manager CrossWalk Tizen Extension
Group:    System/Libraries

License:    MPL-2.0
URL:      http://git.projects.genivi.org/?p=media-manager.git
Source0:  %{name}-%{version}.tar.gz

Requires: crosswalk jansson
BuildRequires: cmake git jansson-devel jansson

%description
Media Manager CrossWalk Tizen Extension. Interfaces with GENIVI MediaManager

%prep
%setup -q

%build
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=%{_prefix} ..

make %{?_smp_mflags}

%install
cd build
make install DESTDIR=%{buildroot} PREFIX=%{_prefix}

%files
%{_prefix}/lib/tizen-extensions-crosswalk/libmediamanager.so
