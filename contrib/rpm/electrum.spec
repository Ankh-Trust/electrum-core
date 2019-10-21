%define bdbv 4.8.30
%global selinux_variants mls strict targeted

%if 0%{?_no_gui:1}
%define _buildqt 0
%define buildargs --with-gui=no
%else
%define _buildqt 1
%if 0%{?_use_qt4}
%define buildargs --with-qrencode --with-gui=qt4
%else
%define buildargs --with-qrencode --with-gui=qt5
%endif
%endif

Name:		electrum
Version:	0.12.0
Release:	2%{?dist}
Summary:	Peer to Peer Cryptographic Currency

Group:		Applications/System
License:	MIT
URL:		https://electrum.org/
Source0:	https://electrum.org/bin/electrum-core-%{version}/electrum-%{version}.tar.gz
Source1:	http://download.oracle.com/berkeley-db/db-%{bdbv}.NC.tar.gz

Source10:	https://raw.githubusercontent.com/electrum/electrum/v%{version}/contrib/debian/examples/electrum.conf

#man pages
Source20:	https://raw.githubusercontent.com/electrum/electrum/v%{version}/contrib/debian/manpages/electrumd.1
Source21:	https://raw.githubusercontent.com/electrum/electrum/v%{version}/contrib/debian/manpages/electrum-cli.1
Source22:	https://raw.githubusercontent.com/electrum/electrum/v%{version}/contrib/debian/manpages/electrum-qt.1
Source23:	https://raw.githubusercontent.com/electrum/electrum/v%{version}/contrib/debian/manpages/electrum.conf.5

#selinux
Source30:	https://raw.githubusercontent.com/electrum/electrum/v%{version}/contrib/rpm/electrum.te
# Source31 - what about electrum-tx and bench_electrum ???
Source31:	https://raw.githubusercontent.com/electrum/electrum/v%{version}/contrib/rpm/electrum.fc
Source32:	https://raw.githubusercontent.com/electrum/electrum/v%{version}/contrib/rpm/electrum.if

Source100:	https://upload.wikimedia.org/wikipedia/commons/4/46/Electrum.svg

%if 0%{?_use_libressl:1}
BuildRequires:	libressl-devel
%else
BuildRequires:	openssl-devel
%endif
BuildRequires:	boost-devel
BuildRequires:	miniupnpc-devel
BuildRequires:	autoconf automake libtool
BuildRequires:	libevent-devel


Patch0:		electrum-0.12.0-libressl.patch


%description
Electrum is a digital cryptographic currency that uses peer-to-peer technology to
operate with no central authority or banks; managing transactions and the
issuing of electrums is carried out collectively by the network.

%if %{_buildqt}
%package core
Summary:	Peer to Peer Cryptographic Currency
Group:		Applications/System
Obsoletes:	%{name} < %{version}-%{release}
Provides:	%{name} = %{version}-%{release}
%if 0%{?_use_qt4}
BuildRequires:	qt-devel
%else
BuildRequires:	qt5-qtbase-devel
# for /usr/bin/lrelease-qt5
BuildRequires:	qt5-linguist
%endif
BuildRequires:	protobuf-devel
BuildRequires:	qrencode-devel
BuildRequires:	%{_bindir}/desktop-file-validate
# for icon generation from SVG
BuildRequires:	%{_bindir}/inkscape
BuildRequires:	%{_bindir}/convert

%description core
Electrum is a digital cryptographic currency that uses peer-to-peer technology to
operate with no central authority or banks; managing transactions and the
issuing of electrums is carried out collectively by the network.

This package contains the Qt based graphical client and node. If you are looking
to run a Electrum wallet, this is probably the package you want.
%endif


%package libs
Summary:	Electrum shared libraries
Group:		System Environment/Libraries

%description libs
This package provides the electrumconsensus shared libraries. These libraries
may be used by third party software to provide consensus verification
functionality.

Unless you know need this package, you probably do not.

%package devel
Summary:	Development files for electrum
Group:		Development/Libraries
Requires:	%{name}-libs = %{version}-%{release}

%description devel
This package contains the header files and static library for the
electrumconsensus shared library. If you are developing or compiling software
that wants to link against that library, then you need this package installed.

Most people do not need this package installed.

%package server
Summary:	The electrum daemon
Group:		System Environment/Daemons
Requires:	electrum-utils = %{version}-%{release}
Requires:	selinux-policy policycoreutils-python
Requires(pre):	shadow-utils
Requires(post):	%{_sbindir}/semodule %{_sbindir}/restorecon %{_sbindir}/fixfiles %{_sbindir}/sestatus
Requires(postun):	%{_sbindir}/semodule %{_sbindir}/restorecon %{_sbindir}/fixfiles %{_sbindir}/sestatus
BuildRequires:	systemd
BuildRequires:	checkpolicy
BuildRequires:	%{_datadir}/selinux/devel/Makefile

%description server
This package provides a stand-alone electrum-core daemon. For most users, this
package is only needed if they need a full-node without the graphical client.

Some third party wallet software will want this package to provide the actual
electrum-core node they use to connect to the network.

If you use the graphical electrum-core client then you almost certainly do not
need this package.

%package utils
Summary:	Electrum utilities
Group:		Applications/System

%description utils
This package provides several command line utilities for interacting with a
electrum-core daemon.

The electrum-cli utility allows you to communicate and control a electrum daemon
over RPC, the electrum-tx utility allows you to create a custom transaction, and
the bench_electrum utility can be used to perform some benchmarks.

This package contains utilities needed by the electrum-server package.


%prep
%setup -q
%patch0 -p1 -b .libressl
cp -p %{SOURCE10} ./electrum.conf.example
tar -zxf %{SOURCE1}
cp -p db-%{bdbv}.NC/LICENSE ./db-%{bdbv}.NC-LICENSE
mkdir db4 SELinux
cp -p %{SOURCE30} %{SOURCE31} %{SOURCE32} SELinux/


%build
CWD=`pwd`
cd db-%{bdbv}.NC/build_unix/
../dist/configure --enable-cxx --disable-shared --with-pic --prefix=${CWD}/db4
make install
cd ../..

./autogen.sh
%configure LDFLAGS="-L${CWD}/db4/lib/" CPPFLAGS="-I${CWD}/db4/include/" --with-miniupnpc --enable-glibc-back-compat %{buildargs}
make %{?_smp_mflags}

pushd SELinux
for selinuxvariant in %{selinux_variants}; do
	make NAME=${selinuxvariant} -f %{_datadir}/selinux/devel/Makefile
	mv electrum.pp electrum.pp.${selinuxvariant}
	make NAME=${selinuxvariant} -f %{_datadir}/selinux/devel/Makefile clean
done
popd


%install
make install DESTDIR=%{buildroot}

mkdir -p -m755 %{buildroot}%{_sbindir}
mv %{buildroot}%{_bindir}/electrumd %{buildroot}%{_sbindir}/electrumd

# systemd stuff
mkdir -p %{buildroot}%{_tmpfilesdir}
cat <<EOF > %{buildroot}%{_tmpfilesdir}/electrum.conf
d /run/electrumd 0750 electrum electrum -
EOF
touch -a -m -t 201504280000 %{buildroot}%{_tmpfilesdir}/electrum.conf

mkdir -p %{buildroot}%{_sysconfdir}/sysconfig
cat <<EOF > %{buildroot}%{_sysconfdir}/sysconfig/electrum
# Provide options to the electrum daemon here, for example
# OPTIONS="-testnet -disable-wallet"

OPTIONS=""

# System service defaults.
# Don't change these unless you know what you're doing.
CONFIG_FILE="%{_sysconfdir}/electrum/electrum.conf"
DATA_DIR="%{_localstatedir}/lib/electrum"
PID_FILE="/run/electrumd/electrumd.pid"
EOF
touch -a -m -t 201504280000 %{buildroot}%{_sysconfdir}/sysconfig/electrum

mkdir -p %{buildroot}%{_unitdir}
cat <<EOF > %{buildroot}%{_unitdir}/electrum.service
[Unit]
Description=Electrum daemon
After=syslog.target network.target

[Service]
Type=forking
ExecStart=%{_sbindir}/electrumd -daemon -conf=\${CONFIG_FILE} -datadir=\${DATA_DIR} -pid=\${PID_FILE} \$OPTIONS
EnvironmentFile=%{_sysconfdir}/sysconfig/electrum
User=electrum
Group=electrum

Restart=on-failure
PrivateTmp=true
TimeoutStopSec=120
TimeoutStartSec=60
StartLimitInterval=240
StartLimitBurst=5

[Install]
WantedBy=multi-user.target
EOF
touch -a -m -t 201504280000 %{buildroot}%{_unitdir}/electrum.service
#end systemd stuff

mkdir %{buildroot}%{_sysconfdir}/electrum
mkdir -p %{buildroot}%{_localstatedir}/lib/electrum

#SELinux
for selinuxvariant in %{selinux_variants}; do
	install -d %{buildroot}%{_datadir}/selinux/${selinuxvariant}
	install -p -m 644 SELinux/electrum.pp.${selinuxvariant} %{buildroot}%{_datadir}/selinux/${selinuxvariant}/electrum.pp
done

%if %{_buildqt}
# qt icons
install -D -p share/pixmaps/electrum.ico %{buildroot}%{_datadir}/pixmaps/electrum.ico
install -p share/pixmaps/nsis-header.bmp %{buildroot}%{_datadir}/pixmaps/
install -p share/pixmaps/nsis-wizard.bmp %{buildroot}%{_datadir}/pixmaps/
install -p %{SOURCE100} %{buildroot}%{_datadir}/pixmaps/electrum.svg
%{_bindir}/inkscape %{SOURCE100} --export-png=%{buildroot}%{_datadir}/pixmaps/electrum16.png -w16 -h16
%{_bindir}/inkscape %{SOURCE100} --export-png=%{buildroot}%{_datadir}/pixmaps/electrum32.png -w32 -h32
%{_bindir}/inkscape %{SOURCE100} --export-png=%{buildroot}%{_datadir}/pixmaps/electrum64.png -w64 -h64
%{_bindir}/inkscape %{SOURCE100} --export-png=%{buildroot}%{_datadir}/pixmaps/electrum128.png -w128 -h128
%{_bindir}/inkscape %{SOURCE100} --export-png=%{buildroot}%{_datadir}/pixmaps/electrum256.png -w256 -h256
%{_bindir}/convert -resize 16x16 %{buildroot}%{_datadir}/pixmaps/electrum256.png %{buildroot}%{_datadir}/pixmaps/electrum16.xpm
%{_bindir}/convert -resize 32x32 %{buildroot}%{_datadir}/pixmaps/electrum256.png %{buildroot}%{_datadir}/pixmaps/electrum32.xpm
%{_bindir}/convert -resize 64x64 %{buildroot}%{_datadir}/pixmaps/electrum256.png %{buildroot}%{_datadir}/pixmaps/electrum64.xpm
%{_bindir}/convert -resize 128x128 %{buildroot}%{_datadir}/pixmaps/electrum256.png %{buildroot}%{_datadir}/pixmaps/electrum128.xpm
%{_bindir}/convert %{buildroot}%{_datadir}/pixmaps/electrum256.png %{buildroot}%{_datadir}/pixmaps/electrum256.xpm
touch %{buildroot}%{_datadir}/pixmaps/*.png -r %{SOURCE100}
touch %{buildroot}%{_datadir}/pixmaps/*.xpm -r %{SOURCE100}

# Desktop File - change the touch timestamp if modifying
mkdir -p %{buildroot}%{_datadir}/applications
cat <<EOF > %{buildroot}%{_datadir}/applications/electrum-core.desktop
[Desktop Entry]
Encoding=UTF-8
Name=Electrum
Comment=Electrum P2P Cryptocurrency
Comment[fr]=Electrum, monnaie virtuelle cryptographique pair à pair
Comment[tr]=Electrum, eşten eşe kriptografik sanal para birimi
Exec=electrum-qt %u
Terminal=false
Type=Application
Icon=electrum128
MimeType=x-scheme-handler/electrum;
Categories=Office;Finance;
EOF
# change touch date when modifying desktop
touch -a -m -t 201511100546 %{buildroot}%{_datadir}/applications/electrum-core.desktop
%{_bindir}/desktop-file-validate %{buildroot}%{_datadir}/applications/electrum-core.desktop

# KDE protocol - change the touch timestamp if modifying
mkdir -p %{buildroot}%{_datadir}/kde4/services
cat <<EOF > %{buildroot}%{_datadir}/kde4/services/electrum-core.protocol
[Protocol]
exec=electrum-qt '%u'
protocol=electrum
input=none
output=none
helper=true
listing=
reading=false
writing=false
makedir=false
deleting=false
EOF
# change touch date when modifying protocol
touch -a -m -t 201511100546 %{buildroot}%{_datadir}/kde4/services/electrum-core.protocol
%endif

# man pages
install -D -p %{SOURCE20} %{buildroot}%{_mandir}/man1/electrumd.1
install -p %{SOURCE21} %{buildroot}%{_mandir}/man1/electrum-cli.1
%if %{_buildqt}
install -p %{SOURCE22} %{buildroot}%{_mandir}/man1/electrum-qt.1
%endif
install -D -p %{SOURCE23} %{buildroot}%{_mandir}/man5/electrum.conf.5

# nuke these, we do extensive testing of binaries in %%check before packaging
rm -f %{buildroot}%{_bindir}/test_*

%check
make check
pushd src
srcdir=. test/electrum-util-test.py
popd
qa/pull-tester/rpc-tests.py -extended

%post libs -p /sbin/ldconfig

%postun libs -p /sbin/ldconfig

%pre server
getent group electrum >/dev/null || groupadd -r electrum
getent passwd electrum >/dev/null ||
	useradd -r -g electrum -d /var/lib/electrum -s /sbin/nologin \
	-c "Electrum wallet server" electrum
exit 0

%post server
%systemd_post electrum.service
# SELinux
if [ `%{_sbindir}/sestatus |grep -c "disabled"` -eq 0 ]; then
for selinuxvariant in %{selinux_variants}; do
	%{_sbindir}/semodule -s ${selinuxvariant} -i %{_datadir}/selinux/${selinuxvariant}/electrum.pp &> /dev/null || :
done
%{_sbindir}/semanage port -a -t electrum_port_t -p tcp 5555
%{_sbindir}/semanage port -a -t electrum_port_t -p tcp 5556
%{_sbindir}/semanage port -a -t electrum_port_t -p tcp 45555
%{_sbindir}/semanage port -a -t electrum_port_t -p tcp 45556
%{_sbindir}/fixfiles -R electrum-server restore &> /dev/null || :
%{_sbindir}/restorecon -R %{_localstatedir}/lib/electrum || :
fi

%posttrans server
%{_bindir}/systemd-tmpfiles --create

%preun server
%systemd_preun electrum.service

%postun server
%systemd_postun electrum.service
# SELinux
if [ $1 -eq 0 ]; then
	if [ `%{_sbindir}/sestatus |grep -c "disabled"` -eq 0 ]; then
	%{_sbindir}/semanage port -d -p tcp 5555
	%{_sbindir}/semanage port -d -p tcp 5556
	%{_sbindir}/semanage port -d -p tcp 45555
	%{_sbindir}/semanage port -d -p tcp 45556
	for selinuxvariant in %{selinux_variants}; do
		%{_sbindir}/semodule -s ${selinuxvariant} -r electrum &> /dev/null || :
	done
	%{_sbindir}/fixfiles -R electrum-server restore &> /dev/null || :
	[ -d %{_localstatedir}/lib/electrum ] && \
		%{_sbindir}/restorecon -R %{_localstatedir}/lib/electrum &> /dev/null || :
	fi
fi

%clean
rm -rf %{buildroot}

%if %{_buildqt}
%files core
%defattr(-,root,root,-)
%license COPYING db-%{bdbv}.NC-LICENSE
%doc COPYING electrum.conf.example doc/README.md doc/bips.md doc/files.md doc/multiwallet-qt.md doc/reduce-traffic.md doc/release-notes.md doc/tor.md
%attr(0755,root,root) %{_bindir}/electrum-qt
%attr(0644,root,root) %{_datadir}/applications/electrum-core.desktop
%attr(0644,root,root) %{_datadir}/kde4/services/electrum-core.protocol
%attr(0644,root,root) %{_datadir}/pixmaps/*.ico
%attr(0644,root,root) %{_datadir}/pixmaps/*.bmp
%attr(0644,root,root) %{_datadir}/pixmaps/*.svg
%attr(0644,root,root) %{_datadir}/pixmaps/*.png
%attr(0644,root,root) %{_datadir}/pixmaps/*.xpm
%attr(0644,root,root) %{_mandir}/man1/electrum-qt.1*
%endif

%files libs
%defattr(-,root,root,-)
%license COPYING
%doc COPYING doc/README.md doc/shared-libraries.md
%{_libdir}/lib*.so.*

%files devel
%defattr(-,root,root,-)
%license COPYING
%doc COPYING doc/README.md doc/developer-notes.md doc/shared-libraries.md
%attr(0644,root,root) %{_includedir}/*.h
%{_libdir}/*.so
%{_libdir}/*.a
%{_libdir}/*.la
%attr(0644,root,root) %{_libdir}/pkgconfig/*.pc

%files server
%defattr(-,root,root,-)
%license COPYING db-%{bdbv}.NC-LICENSE
%doc COPYING electrum.conf.example doc/README.md doc/REST-interface.md doc/bips.md doc/dnsseed-policy.md doc/files.md doc/reduce-traffic.md doc/release-notes.md doc/tor.md
%attr(0755,root,root) %{_sbindir}/electrumd
%attr(0644,root,root) %{_tmpfilesdir}/electrum.conf
%attr(0644,root,root) %{_unitdir}/electrum.service
%dir %attr(0750,electrum,electrum) %{_sysconfdir}/electrum
%dir %attr(0750,electrum,electrum) %{_localstatedir}/lib/electrum
%config(noreplace) %attr(0600,root,root) %{_sysconfdir}/sysconfig/electrum
%attr(0644,root,root) %{_datadir}/selinux/*/*.pp
%attr(0644,root,root) %{_mandir}/man1/electrumd.1*
%attr(0644,root,root) %{_mandir}/man5/electrum.conf.5*

%files utils
%defattr(-,root,root,-)
%license COPYING
%doc COPYING electrum.conf.example doc/README.md
%attr(0755,root,root) %{_bindir}/electrum-cli
%attr(0755,root,root) %{_bindir}/electrum-tx
%attr(0755,root,root) %{_bindir}/bench_electrum
%attr(0644,root,root) %{_mandir}/man1/electrum-cli.1*
%attr(0644,root,root) %{_mandir}/man5/electrum.conf.5*



%changelog
* Fri Feb 26 2016 Alice Wonder <buildmaster@librelamp.com> - 0.12.0-2
- Rename Qt package from electrum to electrum-core
- Make building of the Qt package optional
- When building the Qt package, default to Qt5 but allow building
-  against Qt4
- Only run SELinux stuff in post scripts if it is not set to disabled

* Wed Feb 24 2016 Alice Wonder <buildmaster@librelamp.com> - 0.12.0-1
- Initial spec file for 0.12.0 release

# This spec file is written from scratch but a lot of the packaging decisions are directly
# based upon the 0.11.2 package spec file from https://www.ringingliberty.com/electrum/
