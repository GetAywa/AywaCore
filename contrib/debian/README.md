
Debian
====================
This directory contains files used to package aywad/aywa-qt
for Debian-based Linux systems. If you compile aywad/aywa-qt yourself, there are some useful files here.

## aywa: URI support ##


aywa-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install aywa-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your aywa-qt binary to `/usr/bin`
and the `../../share/pixmaps/aywa128.png` to `/usr/share/pixmaps`

aywa-qt.protocol (KDE)

