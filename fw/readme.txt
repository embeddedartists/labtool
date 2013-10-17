Disclaimer
==========
The code is delivered "AS-IS". There will be restructuring needed when migrating to LPC Open Platform
as well as when the code is paritioned on the three available cores. 

Prerequisites
=============
The easiest way to run the code is to install the UI and drivers (instructions for that has been
emailed earlier).

The code has been developed in Keil's uVision v4.70

About the firmware
==================
A very short introduction to the software:

After setting everything up, main enters an infinite loop in the usb_handler_Run() function
in usb_handler.c waiting for commands from the PC application.

The PC's request for sampling results in a call to capture_Configure() and then capture_Arm()
in capture.c which then delegates to capture_sgpio.c or capture_vadc.c depending on the request.

After completed SGPIO sampling the result is reported through the capture_ReportSGPIODone()
function which sets a flag. The flag is detected in usb_handler_Run(), causing the data to
be sent to the PC application. The VADC sampling works in the same way.

In the case of VADC sampling of both VADC channels the usb_handler_Run() function also 
have a LabTool_FindSkippedSamples() function that detects and prints missed samples.

NOTE: As there is no USB uart available for printing, the project uses the Trace functionallity
      by remapping the UARTPutChar() function into ITM_SendChar() in debug_frmwrk.c.
	  
	  This must also be correctly configured in the Debugger settings dialog under the Trace
	  tab. This has already been done, but when changing the PLL1 to anything other than 200MHz
	  it is important to change it in the dialog as well, otherwise the printouts will be lost.
	  
	  To see the printouts start the debug session and then select the 
	  View->Serial Windows->Debug (printf) Viewer
	  
NOTE: The freq_measure.fzm file may cause build errors if the SCT tools are not installed.
      To get around the problem right-click the file, select Options... and remove the
	  string in the Custom Arguments box. The SCT file is actually not used at this time
	  (disabled in labtool_config.h), but the file is still compiled.

Running
=======
1) Compile the code and start the debugging session from uVision.
2) Start the "LabTool PC" application.
3) Wait a couple of seconds for the application to detect the device. 
4) Select the "LabTool Device" from the Devices menu

For information about the UI see the User's manual that Anders Rosvall has sent earlier.

