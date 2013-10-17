Compiling LabTool Application
=============================
The LabTool Application is running on a PC. These instructions are for Microsoft Windows only.

Requirements
------------
Compiling the LabTool Application requires [Qt](http://qt-project.org/). During development different versions of Qt have been tested: `4.8.1`, `5.0.0` and `5.1.1`. The LabTool installation program that is available on the [project page][1] will deploy LabTool based on `4.8.1`.

Only the MinGW 4.8 toolchain that is bundled with Qt has been tested.

Compiling
---------
1. Open the [LabTool.pro](app/LabTool.pro) project file in Qt Creator.
2. Accept the default configuration (i.e. to use Shadow Build and to create a Debug and a Release target)
3. Compile

Debugging
---------
Debugging can be done from inside Qt Creator.

Deploying
---------
The LabTool Application can be executed directly from inside Qt Creator. If you have LabTool installed and want to replace that with the version you built then 

1. Make sure that the Qt version you have is 4.8 (otherwise the installed dll's will be incompatible)
2. Make sure that you build LabTool in Release mode (otherwise the installed dll's will be incompatible)
3. Copy `../LabTool-build-desktop-Qt_4_8_1_for_Desktop_-_MinGW__Qt_SDK__Release/release/LabTool.exe` to the folder where you installed LabTool, e.g. `c:\program files\Embedded Artists\LabTool\`.


[1]: http://www.embeddedartists.com/products/app/labtool.php