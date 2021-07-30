#ifndef STREAMWORKER_H
#define STREAMWORKER_H

#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMutex>
#include <QDebug>

#include "device/capturedevice.h"

/*!
    \class StreamWorker
    \brief Worker class/thread managed by \a UiCaptureStreamer

    \ingroup Capture
*/
class StreamWorker : public QObject {
    Q_OBJECT

public:
    StreamWorker(CaptureDevice* device);
    ~StreamWorker();

    enum StreamingState {
        STOPPED, // not running, no current error
        RUNNING, // successfully running
        ERROR    // not running because of an error
    };

    /// makes sure that this synchronized by a mutex
    /// probably a bit paranoid, but doesn't hurt
    StreamingState getState() { QMutexLocker stateChangingMutexLocker {&stateChangingMutex}; return state; };

private:
    QMutex stateChangingMutex; // indicate that a (possible) state change is happening
    QTcpServer *server = nullptr;
    StreamingState state = StreamingState::STOPPED;
    CaptureDevice* device = nullptr;

    QList<QTcpSocket*> sockets;

    void writeToJson(QJsonObject &json);

    /// makes sure that this synchronized by a mutex and also emits the signal every time
    void setState(StreamingState newState);

private slots:
    void handleNewConnection();

public slots:
    void start(int port);
    void stop();
    void handleCaptureFinished(bool successful, QString msg);

signals:
    void running();
    void stopped();
    void error();
    void deleted();

};

#endif // STREAMWORKER_H
