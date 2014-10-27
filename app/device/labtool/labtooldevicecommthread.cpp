/*
 *  Copyright 2013 Embedded Artists AB
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
#include "labtooldevicecommthread.h"
#include <time.h>
#include <QFile>
#include <QTime>
#include <stdio.h>
#include <QCoreApplication>
#include <QDebug>

/*!
    \class LabToolDeviceCommThread
    \brief Drives the libusbx USB stack and looks for LabTool Hardware to connect to

    \ingroup Device

    As long as there is a connection established with the LabTool Hardware
    this thread will drive the libusbx by continuously calling \a libusb_handle_events_timeout.

    As long as there is no connection established with the LabTool Hardware
    this thread will attempt to make one by:
    -# Run the dfu-util-static.exe tool from http://dfu-util.gnumonks.org/
        to attempt to download the firmware to a matching LPC-DFU device.
        If the LabTool Hardware is not connected or not in DFU mode nothing
        happens. If the firmware is downloaded then the LabTool Hardware
        will be rebooted into LabTool-mode.
    -# Creates an instance of the \ref LabToolDeviceComm and uses it to
        communicate with the LabTool Hardware. If communication works then
        the \ref connectionChanged signal is sent.
*/

/*!
    Constructs a communication thread with the given \a parent.
*/
LabToolDeviceCommThread::LabToolDeviceCommThread(QObject *parent) :
    QThread(parent)
{
    mContext = NULL;
    mRun = true;
    mReconnect = false;
    mConnected = false;
    mDeviceComm = NULL;
    mFirstConnectAttempt = true;
}

/*!
    Drives the libusbx library or looks for a LabTool Hardware to connect to.
*/
void LabToolDeviceCommThread::run()
{
    int err;
    timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    QTime time;
    time.start();

    // Deallocation:
    //   By connecting finished to deleteLater, this object should be deleted
    //   when the run() function exits
    //QObject::connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));

    while (mRun) {
        if (mReconnect) {
            if (mDeviceComm != NULL) {
                mDeviceComm->disconnectFromDevice();
                delete mDeviceComm;
                mDeviceComm = NULL;
            }
            mConnected = false;
            mReconnect = false;
        }
        if (!mConnected) {
            QThread::msleep(1000);
            runDFU();
            mConnected = connectToDevice();
        }
        if (mConnected) {
            err = libusb_handle_events_timeout(mContext, &tv);
            if (err != LIBUSB_SUCCESS) {
                qDebug("...CommThread: got error %s", libusb_error_name(err));
            }

            if (time.elapsed() >= 3000) {
                mDeviceComm->ping();
                time.restart();
            }
        }
    }

}

/*!
    Stops the thread by letting the \ref run() loop end.
*/
void LabToolDeviceCommThread::stop()
{
    mRun = false;
}

/*!
    Terminates the current (if any) connection to the LabTool Hardware
    and starts to reconnect.
*/
void LabToolDeviceCommThread::reconnectToTarget()
{
    mReconnect = true;
}

/*!
    \fn void LabToolDeviceCommThread::connectionChanged(LabToolDeviceComm* newComm)

    Sent to notify that a new connection to the LabTool Hardware has been established.
*/

/*!
    Takes the LabTool Hardware's firmware (in .bin format) and prepends
    the header needed to allow DFU download. The firmware with the
    correct header is saved as .bin.qthdr to avoid overwriting the original.

    The primary location for the firmware is the fw build directory (only
    available when building the firmware yourself, and if that cannot be
    found then the application's folder. When the application is installed
    as a binary the firmware will be placed in the application's folder.
*/
void LabToolDeviceCommThread::prepareDfuImage()
{
#ifndef Q_OS_MACX
    QString fName = "fw/firmware.bin";
#else
    QString appPath = QCoreApplication::applicationDirPath();
    QString fName = appPath + "/../Resources/firmware.bin";
#endif

    if (!QFile::exists(fName)) {
        fName = "../" + fName;
        if (!QFile::exists(fName)) {
            fName = "firmware.bin";
        }
    }

    QFile fIn(fName);
    if (!fIn.exists()) {
        if (mFirstConnectAttempt) {
            qDebug("Target DFU file is missing");
        }
        return;
    }
    fIn.open(QFile::ReadOnly);
    QByteArray arr = fIn.readAll();
    int size = fIn.size();
    qint16 hashSize = (size+511)/512;

    quint8 header[16];
    header[ 0] = 0x1a & 0x3f; // AES_ACTIVE:  0x1a = AES Encryption not active
    header[ 0] |= 0x00 & 0xc0; // HASH_ACTIVE: 0x00 = CMAC Hash is used, value is in HASH_VALUE
    header[ 1] = 0x3f & 0x3f; // RESERVED
    header[ 1] |= 0x00 & 0xc0; // AES_CONTROL: 0x00 = not used here
    header[ 2] = ((hashSize & 0x00ff) >>  0); // HASH_SIZE lsb
    header[ 3] = ((hashSize & 0xff00) >>  8); // HASH_SIZE msb
    header[ 4] = 0x00; //HASH_VALUE
    header[ 5] = 0x00; //HASH_VALUE
    header[ 6] = 0x00; //HASH_VALUE
    header[ 7] = 0x00; //HASH_VALUE
    header[ 8] = 0x00; //HASH_VALUE
    header[ 9] = 0x00; //HASH_VALUE
    header[10] = 0x00; //HASH_VALUE
    header[11] = 0x00; //HASH_VALUE
    header[12] = 0xff; //RESERVED
    header[13] = 0xff; //RESERVED
    header[14] = 0xff; //RESERVED
    header[15] = 0xff; //RESERVED

    fName.append(".qthdr");
    QFile fOut(fName);
    fOut.open(QFile::WriteOnly);
    fOut.write((const char*)header, sizeof(header));
    fOut.write(arr);
    fOut.flush();
    fOut.close();
    fIn.close();

    mPreparedImage = fName;
}

/*!
    Runs the dfu-util-static.exe tool (see http://dfu-util.gnumonks.org/) from
    the tools folder. The program will download the firmware and reboot the hardware.
*/
void LabToolDeviceCommThread::runDFU()
{
    if (mPreparedImage.isEmpty()) {
        prepareDfuImage();
    }

#ifdef Q_OS_MACX
    QString appPath = QCoreApplication::applicationDirPath();
    QString program = appPath + "/dfu-util";
    qDebug("DFU program %s", qPrintable(program));
#else
   #ifdef Q_OS_WIN
       QString program = "tools/dfu-util-0.7-binaries/win32-mingw32/dfu-util-static.exe";
   #else // Q_OS_LINUX
      QString program;
      if (QFile::exists("/usr/bin/dfu-util"))
      {
         program = "/usr/bin/dfu-util";
      }
      else
      {
         #ifdef QT_ARCH_ARM
            program = "tools/dfu-util-0.7-binaries/linux-armel/dfu-util";
         #else // Revert to i386; Maybe add a case + build for x86_64
            program = "tools/dfu-util-0.7-binaries/linux-i386/dfu-util";
         #endif
      }
   #endif
    if (!QFile::exists(program))
    {
        program = "../" + program;
    }
#endif
    // Test that 'program' is executable before executing it to avoid zombie processes, see
    // https://bugreports.qt-project.org/browse/QTBUG-5990
    if (!(QFile::permissions(program) & QFile::ExeUser))
    {
        qCritical() << "Please change the permssion on \"" << program << "\" to make it executable";
        return;
    }

    QStringList arguments;
    arguments << "-R" << "-d 1fc9:000c" << "-D" << mPreparedImage;

    QProcess DFUProcess;
    // DFUProcess.setWorkingDirectory("../tools/dfu-util-0.7-binaries/win32-mingw32/");
    DFUProcess.start(program, arguments);
    if (!DFUProcess.waitForStarted())
    {
        qDebug() << "DFU program \"" << program << "\" failed to start";
        return;
    }

    if (DFUProcess.waitForFinished())
    {
//        qDebug("DFU program finished");

//        switch(DFUProcess.exitStatus()) {
//        case QProcess::NormalExit: qDebug("exitStatus() = QProcess::NormalExit"); break;
//        case QProcess::CrashExit:  qDebug("exitStatus() = QProcess::CrashExit"); break;
//        default:                   qDebug("exitStatus() = unknown code %d", DFUProcess.exitStatus()); break;
//        }

//        switch(DFUProcess.error()) {
//        case QProcess::FailedToStart: qDebug("error() = QProcess::FailedToStart"); break;
//        case QProcess::Crashed:       qDebug("error() = QProcess::Crashed"); break;
//        case QProcess::Timedout:      qDebug("error() = QProcess::Timedout"); break;
//        case QProcess::WriteError:    qDebug("error() = QProcess::WriteError"); break;
//        case QProcess::ReadError:     qDebug("error() = QProcess::ReadError"); break;
//        case QProcess::UnknownError:  qDebug("error() = QProcess::UnknownError"); break;
//        default:                      qDebug("error() = unknown code %d", DFUProcess.error()); break;
//        }

//        qDebug("exitCode() = %d", DFUProcess.exitCode());

//        qDebug("readAllStandardError(): -->%s<--", DFUProcess.readAllStandardError().constData());
//        qDebug("readAllStandardOutput(): -->%s<--", DFUProcess.readAllStandardOutput().constData());
    }
    else
    {
        qDebug("DFU program timed out waiting to finish");
    }
}

/*!
    Attempts to connect to the LabTool Hardware. A successfull connection will be
    result in the \ref connectionChanged signal.
*/
bool LabToolDeviceCommThread::connectToDevice()
{
    bool first = mFirstConnectAttempt;
    mFirstConnectAttempt = false;

    // Deallocation:
    //   LabToolDeviceComm will be deallocated by the LabToolDevice
    //   or this function
    LabToolDeviceComm* pComm = new LabToolDeviceComm();
    if (pComm->connectToDevice(!first)) {
        mDeviceComm = pComm;
        mContext = pComm->usbContext();
        emit connectionChanged(mDeviceComm);
        return true;
    } else {
        delete pComm;
        return false;
    }
}
