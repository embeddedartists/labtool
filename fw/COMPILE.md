Compiling LabTool Firmware
==========================
The LabTool Firmware is for the LPC4370 on the LPC-Link2 board. It is only available with a SRAM version (no FLASH) so that the LabTool application can always load the latest version in runtime.

Requirements
------------
Compiling the LabTool firmware can be done in any of these three tools:

1. [KEIL uVision IDE](http://www.keil.com/uvision/) with a professional license as the evaluation version is limited to 32Kbytes of code and data. Version 4.72 of uVision was used during development of the firmware.
2. [LPCXpresso IDE](http://www.lpcware.com/lpcxpresso/home), available in a Free Edition. Tested with version 6.1.0.
3. [GNU ARM GCC](https://launchpad.net/gcc-arm-embedded). Tested with version 4.7-2013-q3.

Compiling
---------
[KEIL uVision IDE](http://www.keil.com/uvision/):

1. Open the [firmware.uvproj](program/uVision/firmware.uvproj) project file.
2. Select Project->Build Target to compile
3. The firmware.bin file will be available in `./firmware.bin`


[LPCXpresso IDE](http://www.lpcware.com/lpcxpresso/home):

1. Start the LPCXpresso IDE and create a new workspace
2. Select File->Import... to start the import dialog
3. Expand General and select Existing Projects into Workspace. Press Next
4. Enter the path to the `fw` folder in the Select Root Directory box. Press Finish
5. Right click the `firmware` project and select Build Configurations->Set Active->Release
6. Select Project->Build Project to compile
7. The firmware.bin file will be available in `./firmware.bin`


[GNU ARM GCC](https://launchpad.net/gcc-arm-embedded):

1. Start a terminal and make sure that `arm-none-eabi-gcc.exe` is in the path.
2. Go to the `fw` folder
3. Type `make` to compile the firmware
4. The firmware.bin file will be available in `./firmware.bin`

Deploying
---------
The LabTool application will look for the `firmware.bin` file in the `fw/` folder first and if it cannot be found there then the folder with the `LabTool.exe` file will be searched. If you have compiled the LabTool application as well then the `firmware.bin` file is already located in the correct place and you don't have to do anything else.

If you have an installed version of LabTool that you would like to use the new binary with then copy firmware.bin to the folder where you installed LabTool, e.g. `c:\program files\Embedded Artists\LabTool\`.

Debugging
---------
[KEIL uVision IDE](http://www.keil.com/uvision/):

The [firmware.uvproj](program/uVision/firmware.uvproj) project is setup to use KEIL's ULINK2 debug adapter which connects to the J2 connector on the LPC-Link 2.

If you have a different debug adapter, e.g. another LPC-Link 2 board, that can be changed in the Project->Options for Target settings on the Debug and Utilities tabs.

[LPCXpresso IDE](http://www.lpcware.com/lpcxpresso/home):

LPCXpresso supports several types of debug adapters but only LPC-Link 2 has been tested. Insert a 10-pos cable in the J7 connector on the standalone LPC-Link 2 board. Connect the other end of that cable to the J2 connector on the LPC-Link 2 board that is mounted on the LabTool board. After attaching the cable, carefully place the LPC-Link 2 back on top of the LabTool board while making sure the connectors still align.

In the LPCXpresso IDE:

1. Click `Debug 'firmware' [Debug]` in the Start Here box
2. Stop the debug session as soon as it completes as it must be modified before working correctly
3. Select Run->Debug Configurations... in the menues
4. Go to the Debugger tab
5. Scroll down to the Reset Script and change it to `LPC18LPC43RamReset.scp`
6. Click `Debug 'firmware' [Debug]` in the Start Here box and this time it should work correctly

Note that you have two LPC-Link 2 boards connected to the PC and LPCXpresso must select the correct one when debugging (otherwise it uses LabTool's LPC-Link 2 as debugger).
