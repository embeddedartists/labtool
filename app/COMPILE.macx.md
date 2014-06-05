Compiling LabTool Application on Mac OSX
=============================
This note is how to compile the LabTool application on Mac OSX.

Requirements
------------
Compiling the LabTool Application requires [Qt](http://qt-project.org/). For Mac OSX, Qt version `5.3.0` has been tested. And also libusb is required too.

Compiling
---------
1. Install pkg-config and libusb from homebrew.

    `$ brew install pkg-config libusb`

2. Prepare firmware.bin and place it inside labtool directory.

    `$ cp fw/firmware.bin .`

3. Open the [LabTool.pro](app/LabTool.pro) project file in Qt Creator.
4. Select the configuration to create a **Debug** or a **Release** target.
5. Build project "LabTool"

Deploying
---------
The LabTool Application can be executed directly from inside Qt Creator. If you have LabTool installed and want to replace that with the version you built then 

1. Go to build directory.

    `$ cd build-LabTool-Desktop_Qt_5_3_0_clang_64bit-Release`

2. Add writable flag on libusb dylib file to change dependency while deploying.

    `$ chmod +w LabTool.app/Contents/MacOS/libusb-1.0.0.dylib`

3. Run macdeployqt script with `-dmg` option.

    `$ /{Your Qt Directory}/Qt/5.3/clang_64/bin/macdeployqt LabTool.app -dmg`

4. You have the disk image `LabTool.dmg` which contains LabTool Application. That's all.

[1]: http://www.embeddedartists.com/products/app/labtool.php
