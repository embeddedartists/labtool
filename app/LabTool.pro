SOURCES += \
    capture/uicapturestreamer.cpp \
    main.cpp \
    generator/i2cgenerator.cpp \
    device/devicemanager.cpp \
    device/capturedevice.cpp \
    analyzer/uianalyzer.cpp \
    analyzer/analyzermanager.cpp \
    device/labtool/labtooldevicetransfer.cpp \
    device/labtool/labtooldevicecommthread.cpp \
    device/labtool/labtooldevicecomm.cpp \
    device/simulator/uisimulatorconfigdialog.cpp \
    device/labtool/uilabtooltriggerconfig.cpp \
    analyzer/uart/uiuartanalyzer.cpp \
    generator/uartgenerator.cpp \
    analyzer/uianalyzerconfig.cpp \
    analyzer/uart/uiuartanalyzerconfig.cpp \
    common/inputhelper.cpp \
    common/types.cpp \
    generator/spigenerator.cpp \
    analyzer/i2c/uii2canalyzerconfig.cpp \
    analyzer/i2c/uii2canalyzer.cpp \
    analyzer/spi/uispianalyzer.cpp \
    analyzer/spi/uispianalyzerconfig.cpp \
    device/device.cpp \
    device/generatordevice.cpp \
    device/simulator/simulatorcapturedevice.cpp \
    device/simulator/simulatordevice.cpp \
    device/labtool/labtoolcapturedevice.cpp \
    device/labtool/labtooldevice.cpp \
    generator/uidigitalgenerator.cpp \
    generator/uianaloggenerator.cpp \
    device/simulator/simulatorgeneratordevice.cpp \
    device/labtool/labtoolgeneratordevice.cpp \
    generator/uigeneratorarea.cpp \
    generator/generatorapp.cpp \
    generator/digitaldelegate.cpp \
    generator/digitalsignals.cpp \
    generator/uieditdigital.cpp \
    generator/uieditanalog.cpp \
    generator/uianalogshape.cpp \
    generator/uigeneratorsignaldialog.cpp \
    common/stringutil.cpp \
    uimainwindow.cpp \
    capture/uitimeaxis.cpp \
    capture/uisimpleabstractsignal.cpp \
    capture/uiselectsignaldialog.cpp \
    capture/uiplot.cpp \
    capture/uimeasurmentarea.cpp \
    capture/uilistspinbox.cpp \
    capture/uigrid.cpp \
    capture/uidigitaltrigger.cpp \
    capture/uidigitalsignal.cpp \
    capture/uidigitalgroup.cpp \
    capture/uicursorgroup.cpp \
    capture/uicursor.cpp \
    capture/uicapturearea.cpp \
    capture/uianalogtrigger.cpp \
    capture/uianalogsignal.cpp \
    capture/uianaloggroup.cpp \
    capture/uiabstractsignal.cpp \
    capture/uiabstractplotitem.cpp \
    capture/signalmanager.cpp \
    capture/cursormanager.cpp \
    capture/captureapp.cpp \
    common/configuration.cpp \
    device/analogsignal.cpp \
    capture/uicaptureexporter.cpp \
    device/labtool/labtoolcalibrationwizard.cpp \
    device/labtool/labtoolcalibrationwizardintropage.cpp \
    device/labtool/labtoolcalibrationwizardconclusionpage.cpp \
    device/labtool/labtoolcalibrationwizardanalogout.cpp \
    device/labtool/labtoolcalibrationwizardanalogin.cpp \
    device/labtool/labtoolcalibrationdata.cpp \
    device/digitalsignal.cpp \
    device/reconfigurelistener.cpp

HEADERS += \
    capture/uicapturestreamer.h \
    generator/i2cgenerator.h \
    libusbx/include/libusbx-1.0/libusb.h \
    device/devicemanager.h \
    device/capturedevice.h \
    analyzer/uianalyzer.h \
    device/labtool/labtooldevicetransfer.h \
    device/labtool/labtooldevicecommthread.h \
    device/labtool/labtooldevicecomm.h \
    device/simulator/uisimulatorconfigdialog.h \
    device/labtool/uilabtooltriggerconfig.h \
    analyzer/uart/uiuartanalyzer.h \
    generator/uartgenerator.h \
    analyzer/uianalyzerconfig.h \
    analyzer/uart/uiuartanalyzerconfig.h \
    common/types.h \
    generator/spigenerator.h \
    analyzer/i2c/uii2canalyzerconfig.h \
    analyzer/i2c/uii2canalyzer.h \
    analyzer/spi/uispianalyzer.h \
    analyzer/spi/uispianalyzerconfig.h \
    device/device.h \
    device/generatordevice.h \
    device/simulator/simulatorcapturedevice.h \
    device/simulator/simulatordevice.h \
    device/labtool/labtoolcapturedevice.h \
    device/labtool/labtooldevice.h \
    generator/uidigitalgenerator.h \
    generator/uianaloggenerator.h \
    device/simulator/simulatorgeneratordevice.h \
    device/labtool/labtoolgeneratordevice.h \
    generator/uigeneratorarea.h \
    generator/generatorapp.h \
    generator/digitaldelegate.h \
    generator/digitalsignals.h \
    generator/uieditdigital.h \
    generator/uieditanalog.h \
    generator/uianalogshape.h \
    generator/uigeneratorsignaldialog.h \
    common/stringutil.h \
    uimainwindow.h \
    capture/uitimeaxis.h \
    capture/uisimpleabstractsignal.h \
    capture/uiselectsignaldialog.h \
    capture/uiplot.h \
    capture/uimeasurmentarea.h \
    capture/uilistspinbox.h \
    capture/uigrid.h \
    capture/uidigitaltrigger.h \
    capture/uidigitalsignal.h \
    capture/uidigitalgroup.h \
    capture/uicursorgroup.h \
    capture/uicursor.h \
    capture/uicapturearea.h \
    capture/uianalogtrigger.h \
    capture/uianalogsignal.h \
    capture/uianaloggroup.h \
    capture/uiabstractsignal.h \
    capture/uiabstractplotitem.h \
    capture/signalmanager.h \
    capture/captureapp.h \
    analyzer/analyzermanager.h \
    common/configuration.h \
    capture/cursormanager.h \
    common/inputhelper.h \
    device/analogsignal.h \
    capture/uicaptureexporter.h \
    device/labtool/labtoolcalibrationwizard.h \
    device/labtool/labtoolcalibrationwizardintropage.h \
    device/labtool/labtoolcalibrationwizardconclusionpage.h \
    device/labtool/labtoolcalibrationwizardanalogout.h \
    device/labtool/labtoolcalibrationwizardanalogin.h \
    device/labtool/labtoolcalibrationdata.h \
    device/digitalsignal.h \
    device/reconfigurelistener.h

RESOURCES += \
    icons.qrc

RC_FILE = icon.rc

INCLUDEPATH += .
win32:CONFIG(release, debug|release): LIBS += -L$$PWD/libusbx/MinGW32/dll/ -lusb-1.0
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/libusbx/MinGW32/dll/ -lusb-1.0
else:mac: LIBS += `/usr/local/bin/pkg-config libusb-1.0 --static --libs`
else:unix:!symbian: LIBS += -L$$PWD/libusbx/Linux/ -lusb-1.0 -ludev

INCLUDEPATH += $$PWD/libusbx/MS32/dll
DEPENDPATH += $$PWD/libusbx/MS32/dll

QT += widgets network

mac {
    ICON = resources/oscilloscope.icns

    FIRMWARE_BIN.files = ../firmware.bin
    FIRMWARE_BIN.path = Contents/Resources
    #DFU_UTIL.files = /usr/local/bin/dfu-util
    DFU_UTIL.files = ../tools/dfu-util-0.7-binaries/darwin-universal/dfu-util
    DFU_UTIL.path = Contents/MacOS
    LIBUSB.files = ../tools/dfu-util-0.7-binaries/darwin-universal/libusb-1.0.0.dylib
    LIBUSB.path = Contents/MacOS
    QMAKE_BUNDLE_DATA += FIRMWARE_BIN DFU_UTIL LIBUSB
}
