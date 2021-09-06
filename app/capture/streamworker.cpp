#include "streamworker.h"

StreamWorker::StreamWorker(CaptureDevice* device) : QObject(nullptr) {
    this->device = device;
//    server = new QTcpServer();
//    connect(server, &QTcpServer::newConnection, this, &StreamWorker::handleNewConnection);
}

StreamWorker::~StreamWorker()
{
    stop();
    delete server;
}

/*!
Start streaming (start listening)
*/
void StreamWorker::start(int port) {
    if(server == nullptr) {
        // the server does not like to be dragged across threads, so create it only when we need it (and already are in the right thread)
        server = new QTcpServer();
        connect(server, &QTcpServer::newConnection, this, &StreamWorker::handleNewConnection);
    }
    if(port < 1 || port > 65535) {
        // bad port value
        setState(StreamingState::ERROR);
        return;
    }

    if(getState() != StreamingState::STOPPED) {
        // we are not in the right state, stop and indicate an error
        stop();
        setState(StreamingState::ERROR);
        return;
    }

    if(!server->listen(QHostAddress::Any, port)) {
        // error with the port
        setState(StreamingState::ERROR);
        server->close();
        return;
    }

    qInfo() << "StreamWorker: Starting";
    setState(StreamingState::RUNNING);
}

/*!
 Stop streaming
 */
void StreamWorker::stop() {
    qInfo() << "StreamWorker: Stopping";
    if(getState() == StreamingState::RUNNING) {
        server->close();
        foreach (auto socket, sockets) {
            socket->deleteLater();
        }
        sockets.clear();
    }
    setState(StreamingState::STOPPED);
}

/*!
Sets the state, synchronizes with a mutex, and emits the appropriate signal
 */
void StreamWorker::setState(StreamingState newState) {
    QMutexLocker stateChangingMutexLocker {&stateChangingMutex};
    state = newState;
    switch (state) {
    case STOPPED:
        emit stopped();
        break;
    case RUNNING:
        emit running();
        break;
    case ERROR:
        emit error();
        break;
    };
}

/*!
    Handle a new connection by adding it to the list and remove it when disconnected
*/
void StreamWorker::handleNewConnection() {
    QTcpSocket* socket = server->nextPendingConnection();
    if(socket == nullptr) {
        // I have no idea why this would happen, but it happens
        return;
    }
    sockets.append(socket);
    qInfo() << "StreamWorker: Got new connection: " << (long)socket;
    connect(socket, &QTcpSocket::disconnected, [this, socket](){
        this->sockets.removeOne(socket);
        qDebug() << "StreamWorker: Removed a connection: " << (long)socket;
        socket->deleteLater();
    } );
}

/*!
    Send the captured data to all active sockets
*/
void StreamWorker::handleCaptureFinished(bool successful, QString msg) {
    // no need to further synchronize this, since all of this class is run via the event loop of its thread
    if(getState() == StreamingState::RUNNING && successful) {
        qDebug() << "StreamWorker: Got new data, sending to " << sockets.length() << " clients";
        QJsonObject json;
        this->writeToJson(json);
        // write compact json, so one message -> one line
        // this simplifies reading in clients with tcp streaming
        // so the delimiter between messages will be '\n'
        QByteArray jsonBytes = QJsonDocument(json).toJson(QJsonDocument::Compact);
        foreach(QTcpSocket* socket, sockets) {
            socket->write(jsonBytes);
            socket->write("\n");
        }
    }
}

/*!
    Writes the current state of the CaptureDevice to JSON
*/
void StreamWorker::writeToJson(QJsonObject &json) {
    QJsonArray jsonSignals;

    QList<DigitalSignal*> digitalSignals = device->digitalSignals();
    QList<AnalogSignal*> analogSignals = device->analogSignals();

    json.insert("sampleRate", device->usedSampleRate());

    foreach(DigitalSignal* s, digitalSignals) {
        QVector<int>* data = device->digitalData(s->id());
        if (data == NULL) continue;

        QJsonObject jsonSignal;
        jsonSignal.insert("id", s->id());
        jsonSignal.insert("name", s->name());
        jsonSignal.insert("type", "digital");
        QJsonArray jsonData;
        foreach(int dataPoint, *data) {
            jsonData.append(dataPoint);
        }
        jsonSignal.insert("data", jsonData);
        jsonSignals.append(jsonSignal);
    }
    foreach(AnalogSignal* s, analogSignals) {
        QVector<double>* data = device->analogData(s->id());
        if (data == NULL) continue;

        QJsonObject jsonSignal;
        jsonSignal.insert("id", s->id());
        jsonSignal.insert("name", s->name());
        jsonSignal.insert("type", "analog");
        QJsonArray jsonData;
        foreach(double dataPoint, *data) {
            jsonData.append(dataPoint);
        }
        jsonSignal.insert("data", jsonData);
        jsonSignals.append(jsonSignal);
    }

    json.insert("signals", jsonSignals);

}
