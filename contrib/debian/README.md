
Debian
====================
This directory contains files used to package electrumd/electrum-qt
for Debian-based Linux systems. If you compile electrumd/electrum-qt yourself, there are some useful files here.

## electrum: URI support ##


electrum-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install electrum-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your electrum-qt binary to `/usr/bin`
and the `../../share/pixmaps/electrum128.png` to `/usr/share/pixmaps`

electrum-qt.protocol (KDE)

