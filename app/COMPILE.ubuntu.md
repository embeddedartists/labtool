Compiling LabTool Application
=============================
These instructions are for compiling the LabTool Application for [Ubuntu Desktop][2]. The concepts could probably be used on other linux distributions on other hardware with some modifications - but only Ubuntu Desktop 14.04 LTS on an x86_64 architecture has been verified.

Requirements
------------
* A pc with Ubuntu Desktop preinstalled.

Setup
-----
Start by installing some dependencies. Open an LXTerminal and enter the following commands:

    $ sudo apt-get update
    $ sudo apt-get install build-essential git libudev-dev libtool automake libusb-1.0-0-dev qt5-sdk dfu-util
	
Now download the latest version of LabTool from github

    $ cd ~/projects/
    $ git clone https://github.com/embeddedartists/labtool.git
    $ cd ~/projects/labtool/app

The newly installed libusb must also be made available to LabTool (source location may differ per distribution and architecture):

    $ ln -s /usr/lib/x86_64-linux-gnu/libusb-1.0.a ~/projects/labtool/app/libusbx/Linux/

Create a makefile using qmake
Optionally add parameters `-r -spec linux-g++-64` for a specific instruction set and/or `CONFIG+=debug CONFIG+=declarative_debug CONFIG+=qml_debug` to enable debugging

    $ qmake LabTool.pro

Compile and build the source code

    $ make

Run the executable, also consider section "Running/Debugging"

    $ ./LabTool

If succesful, optionally cleanup all intermediary files

    $ make clean

The LabTool program will need access to connected USB devices in order to communicate with the LabTool hardware. This can be achieved by running as root, but a more elagant way is to add a udev rule which changes the access rights for specific USB devices so that all users in the `plugdev` group (which regular users normally belong to) have full access.

Create a new file `/etc/udev/rules.d/10-ea-labtool.rules` and add the following to it:

    # Allow group plugdev to access the LabTool Hardware (1fc9:0018)
    ACTION=="add", SUBSYSTEM=="usb", ATTRS{idVendor}=="1fc9", ATTRS{idProduct}=="0018", MODE="664", GROUP="plugdev"
    
    # Allow group plugdev to access the LPC DFU device (1fc9:000c)
    ACTION=="add", SUBSYSTEM=="usb", ATTRS{idVendor}=="1fc9", ATTRS{idProduct}=="000c", MODE="664", GROUP="plugdev"

Running/Debugging
-----------------
Note that LabTool expects the firmware for the LabTool hardware. That can either be compiled (see instructions in the [LabTool Firmware](../fw/COMPILE.md)) or extracted from the [archive with binaries][1].

Store the `firmware.bin` file in `~/projects/labtool/fw/` folder. The reason for putting the firware.bin file there is that both the Debug and Release versions look for the file there.


[1]: http://www.embeddedartists.com/products/app/labtool.php
[2]: http://www.ubuntu.org
