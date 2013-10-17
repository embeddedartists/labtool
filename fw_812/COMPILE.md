Compiling LabTool Demo
======================
The LabTool Demo is for the LPC812 on the LabTool board. It outputs UART, SPI, I2C and PWM signals that can be explored using the LabTool application. See the [User's Manual][1] for more information about this.

Requirements
------------
Compiling the LabTool Demo requires [LPCXpresso IDE][1]. Version 6.0.2 of LPCXpresso was used during development of the demo.

Compiling
---------
1. Open LPCXpresso IDE with a new workspace
2. Select Import Project(s)
3. Click the Browse button for the Root directory and point to the [fw_812](fw_812) folder
4. Click Next and then Finish to import the projects
5. Select the LabTool_Demo project and then Project->Build Project to start compiling

Debugging / Deploying
---------------------
The LabTool demo can be flashed with the LPC-Link 2 board mounted on the LabTool board - they don't have to be separated.

To flash the LabTool demo:

1. Use the 10-pos IDC ribbon cable that accompanied the LPC-Link 2 board to connect the two 10-pin sockets on the LabTool board. 
2. Insert the USB cable from the LPC-Link 2 to the PC.
3. In LPCXpresso select Debug LabTool_Demo
4. LPCXpresso will detect and use the LPC-Link 2 to download the program

As the program is stored in FLASH all that is needed to deploy it is to run the debugger once.


[1]: http://www.embeddedartists.com/products/app/labtool.php
[2]: http://www.lpcware.com/lpcxpresso/home
