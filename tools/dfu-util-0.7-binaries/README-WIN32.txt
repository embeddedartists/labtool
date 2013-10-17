README file - dfu-util 0.7 binaries for win32

SOURCE CODE:

The sources for the libusbx library (here distributed as libusb-1.0.dll)
can be found at http://libusbx.org/

The libusbx 1.0.14 release was patched with this bug fix:
https://github.com/libusbx/libusbx/commit/09759d5836766fb3b886824cd669bc0dfc149d00

The sources for dfu-util.exe and dfu-suffix.exe can be found
at http://dfu-util.gnumonks.org/

See individual source files for full copyright information.
See enclosed COPYING file for distribution.

BUILDING:

The binaries were built using the enclosed build-mingw32-libusbx.sh script,
on a Ubuntu 12.04 system.

INSTALLATION:

Keep libusb-1.0.dll in the same directory as the dfu-util.exe and
dfu-suffix.exe executables or copy it to the system library folder
(e.g. C:\WINDOWS\SYSTEM32)

- or - use the dfu-util-static.exe which has libusbx statically linked.

dfu-util uses libusb to access USB devices, and this version of libusb
uses WinUSB (a Microsoft Windows system driver installed by default in
Microsoft Vista and later versions). The USB device to be accessed
must therefore be registered with the WinUSB driver. This can easily be
done with the zadig tool from
https://sourceforge.net/projects/libwdi/files/zadig/

Plug in your device, run zadig.exe, select your device in the left field
and select WinUSB in the right field. You can now access your device from
any WinUSB or libusb based programs.

This libusbx release includes experimental support for the libusb-win32
and libusbK drivers, as alternatives to the WinUSB driver.

Please see https://github.com/libusbx/libusbx/wiki/Windows-Backend for more
information.

