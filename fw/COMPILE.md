Compiling LabTool Firmware
==========================
The LabTool Firware is for the LPC4370 on the LPC-Link2 board. It is only available with a SRAM version (no FLASH) so that the LabTool application can always load the latest version in runtime.

Requirements
------------
Compiling the LabTool firmware requires [KEIL uVision IDE](http://www.keil.com/uvision/) with a professional license as the evaluation version is limited to 32Kbytes of code and data. Version 4.72 of uVision was used during development of the firmware.

Compiling
---------
1. Open the [firmware.uvproj](program/uVision/firmware.uvproj) project file.
2. Select Project->Build Target to compile
3. The firmware.bin file will be available in `program/uVision/Internal_SRAM/firmware.bin`

Deploying
---------
The LabTool application will look for the `firmware.bin` file in the `program/uVision/Internal_SRAM/` folder first and if it cannot be found there then the folder with the `LabTool.exe` file will be searched. If you have compiled the LabTool application as well then the `firmware.bin` file is already located in the correct place and you don't have to do anything else.

If you have an installed version of LabTool that you would like to use the new binary with then copy firmware.bin to the folder where you installed LabTool, e.g. `c:\program files\Embedded Artists\LabTool\`.

Debugging
---------
The [firmware.uvproj](program/uVision/firmware.uvproj) project is setup to use KEIL's ULINK2 debug adapter which connects to the J2 connector on the LPC-Link 2.

If you have a different debug adapter, e.g. another LPC-Link 2 board, that can be changed in the Project->Options for Target settings on the Debug and Utilities tabs.
