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
#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QDateTime>

#include "uimainwindow.h"

#ifdef QT_NO_DEBUG
#if QT_VERSION >= 0x050000
void logOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    (void)context;
#else
void logOutput(QtMsgType type, const char * msg)
{
#endif

    QString outMsg = QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss");

    switch (type) {
    case QtDebugMsg:
        outMsg += " [D] ";
        break;
    case QtWarningMsg:
        outMsg += " [W] ";
        break;
    case QtCriticalMsg:
        outMsg += " [C] ";
        break;
    case QtFatalMsg:
        outMsg += " [F] ";
        break;
    }

//    QString logFilename = QCoreApplication::applicationDirPath()
//            + "/"
//            + QCoreApplication::applicationName()
//            + ".log";
    QString logFilename =
#if QT_VERSION >= 0x050000
            QStandardPaths::writableLocation(QStandardPaths::DataLocation)
#else
            QDesktopServices::storageLocation(QDesktopServices::DataLocation)
#endif
            + QDir::separator()
            + QCoreApplication::applicationName()
            + ".log";


    QFile logFile(logFilename);
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);

        out << outMsg << msg << "\n";
    }

    logFile.close();

    if (type == QtFatalMsg) {
        abort();
    }

}
#endif


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // random functions are used by the application. Set the seed used to
    // generate these number to current time
    qsrand(QDateTime::currentDateTime().toTime_t());

    // application name and organization details for the creator of the application
    QCoreApplication::setOrganizationName("Embedded Artists");
    QCoreApplication::setOrganizationDomain("embeddedartists.com");
    QCoreApplication::setApplicationName("LabTool");

#ifdef QT_NO_DEBUG

#if QT_VERSION >= 0x050000
    qInstallMessageHandler(logOutput);
#else
    qInstallMsgHandler(logOutput);
#endif

#endif  //QT_NO_DEBUG


    UiMainWindow w;
    w.setWindowIcon(QIcon(":/resources/oscilloscope.ico"));
    w.show();
    
    return a.exec();
}
