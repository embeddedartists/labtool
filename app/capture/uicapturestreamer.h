#ifndef UICAPTURESTREAMER_H
#define UICAPTURESTREAMER_H

#include <QDialog>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QThread>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMutex>
#include<QDebug>

#include "device/capturedevice.h"

/*!
    \class StreamWorker
    \brief Worker class/thread managed by \a UiCaptureStreamer

    \ingroup Capture
*/
class StreamWorker : public QObject {
    Q_OBJECT

public:
    enum StreamingState {
        STOPPED, // not running, no current error
        RUNNING, // successfully running
        ERROR    // not running because of an error
    };
    QMutex stateChangingMutex; // indicate that a (possible) state change is happening
    StreamingState getState() { return state; };
    void setCaptureDevice(CaptureDevice* device) { this->device = device; };

private:
    QTcpServer *server = nullptr;
    StreamingState state = StreamingState::STOPPED;
    CaptureDevice* device;

    QList<QTcpSocket*> sockets;

    void writeToJson(QJsonObject &json);

private slots:
    void handleNewConnection();
public slots:
    void start(int port);
    void stop();
    void handleCaptureFinished(bool successful, QString msg);

};

/*!
    \class UiCaptureStreamer
    \brief This class is responsible for setting up and managing the data streaming.
    Largely an adapted form of \a UiCaptureExporter

    \ingroup Capture
*/
class UiCaptureStreamer : public QDialog
{
    Q_OBJECT
public:
    explicit UiCaptureStreamer(CaptureDevice* device, QWidget *parent = 0);
    void stopStreaming();

private:

    QThread workerThread;
    StreamWorker streamWorker;

    CaptureDevice* mCaptureDevice;
    QVBoxLayout* mMainLayout;

    QSpinBox* mPortSpinBox;

private slots:
    void streamData();

signals:
    void start(int port);
    void stop();

};

#endif // UICAPTURESTREAMER_H
