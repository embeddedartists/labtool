LabTool Software
================

This repository hold the software for the [LabTool][1] hardware and consist of three parts:

1. The LabTool [User Interface](app) - a program written in Qt that is executed on a PC.
2. The LabTool [Firmware](fw) - a program that executes on the onboard LPC4370
3. The LabTool [Demo](fw_812) - a program that executes on the onboard LPC812

Documentation
-------------
The User's Manual and information about LabTool is found on the [product page][1].

The code documentation can be generated with doxygen for each of the three parts.

Mailing List
------------
There is a mailing list [here](https://groups.google.com/d/forum/labtool) where issues and questions about LabTool can be discussed.

Typical topics are:
* Installation related problems
* Suggested improvements
* Bugs and problems with the code

Installation - Windows
----------------------
For Windows there is an installer which can be downloaded [here][1]. It will install the LabTool User Interface and the needed drivers.

Installation - Raspberry Pi
---------------------------
For Raspberry Pi there are prebuilt binaries. The prerequisites are:
* A Raspberry Pi model B (needed for the extra 256MBytes RAM)
* A powered USB hub to be able to connect LabTool, a mouse and optionally a keyboard

If you don't have Raspbian Wheezy running already, you can download `2013-09-25-wheezy-raspbian.zip` [here][4] and write it to an SD card using one of the methods described [here][3].

After booting into Raspbian open an LXTerminal and type in these commands:

    $ cd ~/Desktop
    $ wget http://www.embeddedartists.com/sites/default/files/support/app/labtool/labtool_raspi_2013-10-18.tgz
    $ tar -xf labtool_raspi_2013-10-18.tgz
    $ cd LabTool
    $ sudo cp 10-ea-labtool.rules /etc/udev/rules.d/
    $ cp LabTool.desktop ~/Desktop/

Note that the exact name of the archive `labtool_raspi_2013-10-18.tgz` will change over time and the latest version is always available on the [product page][1].

After installing you will have an icon on the desktop to start LabTool with.

Compiling in Windows
--------------------
If you would like to compile LabTool yourself, follow the instructions for that part - [User Interface](app/COMPILE.md), [Firmware](fw/COMPILE.md) or [Demo](fw_812/COMPILE.md).

Compiling in Linux
------------------
Raspberry Pi is used as a reference system for Linux compilation. The instructions for building the LabTool User Interface on a Raspberry Pi are available [here](app/COMPILE.raspi.md). Instructions for building on Ubuntu are available [here](app/COMPILE.ubuntu.md).

If you use a different Linux distribution and/or hardware the Raspberry Pi instructions can give you a starting point.

Suggested Improvements
----------------------
1. More analyzers, e.g. CAN bus, I2S, 1-Wire and other custom analyzers
2. Implement a frequency counter
3. Implement an I2C monitor

Contributing
------------
1. Fork it.
2. Create a branch (`git checkout -b my_labtool`)
3. Commit your changes (`git commit -am "Added CoolFeature"`)
4. Push to the branch (`git push origin my_labtool`)
5. Open a [Pull Request][2]
6. Enjoy a refreshing Diet Coke and wait


[1]: http://www.embeddedartists.com/products/app/labtool.php
[2]: http://github.com/embeddedartists/LabTool/pulls
[3]: http://www.raspberrypi.org
[4]: http://www.raspberrypi.org/downloads
[5]: http://www.raspbian.org/

