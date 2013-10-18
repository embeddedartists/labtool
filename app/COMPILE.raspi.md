Compiling LabTool Application
=============================
These instructions are for compiling the LabTool Application for [Raspberry Pi][2]. The concepts could probably be used on other linux distributions on other hardware with some modifications - but only Raspberry Pi has been verified.

Note that you don't have to compile LabTool yourself to be able to test it on a Raspberry Pi. The easiest way to evaluate LabTool on a Raspberry Pi is to use the archive with prebuilt binaries, see instructions in [README](README.md).

Requirements
------------
* A Raspberry Pi model B (needed for the extra 256MB RAM)
* An 8GByte SD card
* A powered USB hub to be able to connect LabTool, a mouse and optionally a keyboard

Preparation of Raspberry Pi
---------------------------
1. Download an image with [Raspbian][4] Wheezy (this guide was based on `2013-09-25-wheezy-raspbian.zip` available [here][3].
2. Write it to the SD card using one of the methods described [here][3]
3. Boot the Raspbery Pi 
4. Suggested (optional) changes to the raspberry configuration: 
  * Enable SSH - allows remote access
  * Boot into Desktop - LabTool requires a GUI so a desktop is required
  * Expand file system on next boot - to fully utilize the 8GByte file system
5. Reboot to apply the changes

Setup
-----
Start by installing some dependancies. Open an LXTerminal and enter the following commands:

    $ sudo apt-get update
    $ sudo apt-get install git libudev-dev libtool automake qt-sdk
	
The next task is to update the `libusb` to version `lubusb-1.0.17`:

    $ mkdir ~/projects
    $ cd ~/projects
    $ wget http://sourceforge.net/projects/libusbx/files/releases/1.0.17/source/libusbx-1.0.17.tar.bz2/download 
    $ mv download libusbx-1.0.17.tar.bz2 
    $ tar -xf libusbx-1.0.17.tar.bz2
    $ cd libusbx-1.0.17 
    $ ./configure
    $ make
    $ sudo make install

Now download the latest version of LabTool from github

    $ cd ~/projects/
    $ git clone https://github.com/embeddedartists/labtool.git

The newly compiled libusb must also be made available to LabTool:

    $ cp ~/projects/libusbx-1.0.17/libusb/.libs/libusb-1.0.a ~/projects/labtool/app/libusbx/Linux/	
	
Before using Qt Creator to build LabTool it must be configured to allow `Desktop` builds. It defaults to cross-compiled `embedded` builds. To do this:

1. Start Qt Creator
2. Got to Help->About Plugins
3. Uncheck the "Remote Linux" option below "Device Support"
4. Restart Qt Creator
5. Open Tools->Options->Build&Run
6. Go to the Toolchains tab and select Add->GCC and fill in 
	`/usr/bin/arm-linux-gnueabihf-gcc`
	`/usr/bin/gdb`
	default
7. Go to the "Qt Versions" tab and click Add. Point to `/usr/bin/qmake-qt4`
8. Close

Now that Qt Creator supports `Desktop` builds it is time to compile LabTool

1. Start Qt Creator
2. File->Open File or Project
3. Select `/home/pi/projects/labtool/app/LabTool.pro`
4. Configure as "Desktop", "Shadow Build" and "For Each Qt Version One Debug and One Release"
5. Change build type to Debug
6. Build. This will take a very long time so complete some of the steps below in the meantime...

The LabTool program will need access to connected USB devices in order to communicate with the LabTool hardware. This can be achieved by running as root, but a more elagant way is to add a udev rule which changes the access rights for specific USB devices so that all users in the `plugdev` group (which the `pi` user belongs to) have full access.

Create a new file `/etc/udev/rules.d/10-ea-labtool.rules` and add the following to it:

    # Allow group plugdev to access the LabTool Hardware (1fc9:0018)
    ACTION=="add", SUBSYSTEM=="usb", ATTRS{idVendor}=="1fc9", ATTRS{idProduct}=="0018", MODE="664", GROUP="plugdev"
    
    # Allow group plugdev to access the LPC DFU device (1fc9:000c)
    ACTION=="add", SUBSYSTEM=="usb", ATTRS{idVendor}=="1fc9", ATTRS{idProduct}=="000c", MODE="664", GROUP="plugdev"

Running/Debugging
-----------------
Running and/or debugging the LabTool Application is easiest done from inside Qt Creator.

Note that LabTool expects the firmware for the LabTool hardware. That can either be compiled (see instructions in the [LabTool Firmware](../fw/COMPILE.md)) or extracted from the [archive with binaries][1].

Store the `firmware.bin` file in `/home/pi/projects/labtool/fw/program/uVision/Internal_SRAM/` folder (create it if it doesn't exist). The reason for putting the firware.bin file there is that both the Debug and Release versions look for the file there.


[1]: http://www.embeddedartists.com/products/app/labtool.php
[2]: http://www.raspberrypi.org
[3]: http://www.raspberrypi.org/downloads
[4]: http://www.raspbian.org/
