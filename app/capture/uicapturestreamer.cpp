#include "uicapturestreamer.h"

#include <QFormLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QProgressDialog>


/*!
    \class UiCaptureStreamer
    \brief This class is responsible for setting up and managing the data streaming.

    \ingroup Capture

*/

/*!
    Dialog Window
*/
UiCaptureStreamer::UiCaptureStreamer(CaptureDevice* device, QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(tr("Stream Data"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    mCaptureDevice = device;

    // Deallocation: Ownership changed when calling setLayout.
    mMainLayout = new QVBoxLayout();

    // Deallocation: Re-parented when calling mMainLayout->addLayout.
    QFormLayout* formLayout = new QFormLayout();

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mPortSpinBox = new QSpinBox(this);
    mPortSpinBox->setMinimum(1);
    mPortSpinBox->setMaximum(65535);
    mPortSpinBox->setValue(18080); // random port
    formLayout->addRow(tr("Port: "), mPortSpinBox);

    mMainLayout->addLayout(formLayout);

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QPushButton* streamBtn = new QPushButton("Stream", this);
    connect(streamBtn, SIGNAL(clicked()), this, SLOT(streamData()));

    QPushButton* cancelBtn = new QPushButton("Cancel", this);
    connect(cancelBtn, SIGNAL(clicked()), this, SLOT(reject()));

    // Deallocation: Re-parented when calling mMainLayout->addLayout.
    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->addWidget(streamBtn);
    hLayout->addWidget(cancelBtn);
    hLayout->addStretch();
    mMainLayout->addLayout(hLayout);

    mMainLayout->addStretch();

    setLayout(mMainLayout);

    // set up and connect StreamWorker
    streamWorker.moveToThread(&workerThread);
    streamWorker.setCaptureDevice(device);
    connect(this, &UiCaptureStreamer::start, &streamWorker, &StreamWorker::start);
    connect(this, &UiCaptureStreamer::stop, &streamWorker, &StreamWorker::stop);
    connect(device, &CaptureDevice::captureFinished, &streamWorker, &StreamWorker::handleCaptureFinished);

}

/*!
    Called when the user clicks the Stream button.
*/
void UiCaptureStreamer::streamData()
{
    // check if the worker is in the right state
    if(streamWorker.getState() != StreamWorker::STOPPED) {
        reject();
        return;
    }

    // start thread if necessary
    if(!workerThread.isRunning()) {
        workerThread.start();
    }

    emit start(mPortSpinBox->value());

    // check if Worker is done initializing or timeout, a bit ugly
    for (int i = 0; i < 50 && streamWorker.getState() == StreamWorker::STOPPED; i++) {
        streamWorker.stateChangingMutex.tryLock(100);
    }
    if(streamWorker.getState() == StreamWorker::STOPPED) {
        QMessageBox::warning(this,
                             "Stream Error",
                             "Timeout when setting up server, aborting");
        // for good measure, try to close the server again
        emit stop();
        reject();
        return;
    }
    if(streamWorker.getState() == StreamWorker::ERROR) {
        QMessageBox::warning(this,
                             "Stream Error",
                             "Failed to set up server, check port!");
        // this sets the state back to stopped
        emit stop();
        reject();
        return;
    }

    accept();
}

/*!
    Waits until the streaming is stopped
*/
void UiCaptureStreamer::stopStreaming() {
    emit stop();
    while(streamWorker.getState() != StreamWorker::STOPPED) {
        streamWorker.stateChangingMutex.tryLock(1000);
    }
}

///////////////////////////////////////////////////////////////////////////////
// StreamWorker Implementation below
///////////////////////////////////////////////////////////////////////////////

/*!
 * Start streaming (start listening)
 */
void StreamWorker::start(int port) {
    if(port < 1 || port > 65535) {
        // bad port value
        state = StreamingState::ERROR;
        return;
    }

    if(state != StreamingState::STOPPED) {
        // we are not in the right state, stop and indicate an error
        stop();
        state = StreamingState::ERROR;
        return;
    }

    if (server == nullptr) {
        server = new QTcpServer();
    }

    if(!server->listen(QHostAddress::Any, port)) {
        // error with the port
        state = StreamingState::ERROR;
        server->close();
        return;
    }

    connect(server, &QTcpServer::newConnection, this, &StreamWorker::handleNewConnection);
    qInfo() << "StreamWorker: Starting";
    state = StreamingState::RUNNING;
}

/*!
 * Stop streaming
 */
void StreamWorker::stop() {
    qInfo() << "StreamWorker: Stopping";
    if(state == StreamingState::RUNNING) {
        server->close();
        sockets.clear();
    }
    state = StreamingState::STOPPED;
}

/*!
    Handle a new connection by adding it to the list and remove it when disconnected
    \todo check if this works
*/
void StreamWorker::handleNewConnection() {
    QTcpSocket* socket = server->nextPendingConnection();
    if(socket == nullptr) {
        // I have no idea why this would happen, but it happens
        return;
    }
    sockets.append(socket);
    qInfo() << "StreamWorker: Got new connection";
    connect(socket, &QTcpSocket::disconnected, [this, socket](){
        this->sockets.removeOne(socket);
    //  delete socket;
    //  maybe there is a memory leak here, but with the delete, it will crash
    } );
}

/*!
    Send the captured data to all active sockets
    \todo check if this works
*/
void StreamWorker::handleCaptureFinished(bool successful, QString msg) {
    qDebug() << "StreamWorker: Got new data, sending to " << sockets.length() << " clients";
    if(state == StreamingState::RUNNING && successful) {
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
