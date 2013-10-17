LabTool Software
================

This repository hold the software for the [LabTool][1] hardware and consist of three parts:

1. The LabTool [User Interface][app] - a program written in Qt that is executed on a PC.
2. The LabTool [Firmware][fw] - a program that executes on the onboard LPC4370
3. The LabTool [Demo][fw_812] - a program that executes on the onboard LPC812

Documentation
-------------
The User's Manual and information about LabTool is found on the [product page][1].

The code documentation can be generated with doxygen for each of the three parts.

Installation
-----------
Download and run the installer [here][1]. It will install the LabTool User Interface and the needed drivers in Windows.

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
