#include "uicapturestreamer.h"

#include <QFormLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QProgressDialog>


/*!
    Dialog Window
*/
UiCaptureStreamer::UiCaptureStreamer(CaptureDevice* device, QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(tr("Stream"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    mCaptureDevice = device;

    // Deallocation: Ownership changed when calling setLayout.
    mMainLayout = new QVBoxLayout();

    QFormLayout* formLayout = new QFormLayout();

    mMainLayout->addWidget(new QLabel(tr("Stream data over TCP as JSON"), this));
    mMainLayout->addWidget(new QLabel(tr("Every message is in a new line"), this));

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mPortSpinBox = new QSpinBox(this);
    mPortSpinBox->setMinimum(1);
    mPortSpinBox->setMaximum(65535);
    mPortSpinBox->setValue(18080); // random port
    formLayout->addRow(tr("Port: "), mPortSpinBox);

    mMainLayout->addLayout(formLayout);

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QPushButton* streamBtn = new QPushButton("Stream", this);
    connect(streamBtn, SIGNAL(clicked()), this, SLOT(handleStreamBtnPressed()));

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
    streamWorker = new StreamWorker(device);
    workerThread = new QThread();
    streamWorker->moveToThread(workerThread);
    connect(this, &UiCaptureStreamer::startWorker, streamWorker, &StreamWorker::start);
    connect(this, &UiCaptureStreamer::stopWorker, streamWorker, &StreamWorker::stop);
    connect(streamWorker, &StreamWorker::running, this, &UiCaptureStreamer::handleStreamRunning);
    connect(streamWorker, &StreamWorker::error, this, &UiCaptureStreamer::handleStreamError);
    connect(device, &CaptureDevice::captureFinished, streamWorker, &StreamWorker::handleCaptureFinished);
    // make sure to properly set up deletion across threads
    connect(this, &UiCaptureStreamer::destroyWorker, streamWorker, &StreamWorker::deleteLater);
    connect(streamWorker, &StreamWorker::deleted, workerThread, &QThread::deleteLater);

}

/*!
Destructor that takes care of stopping the thread etc.
*/
UiCaptureStreamer::~UiCaptureStreamer()
{
    // this stops the worker and then frees its resources
    // then the worker will detele its thread via signals
    emit destroyWorker();

}

/*!
Called when the user clicks the Stream button.
*/
void UiCaptureStreamer::handleStreamBtnPressed()
{
    // check if the worker is in the right state
    if(streamWorker->getState() != StreamWorker::STOPPED) {
        reject();
        return;
    }

    // start thread if necessary
    if(!workerThread->isRunning()) {
        workerThread->start();
    }

    emit startWorker(mPortSpinBox->value());

    // the answer is handled by handleStreamWorkerRunning/Error
}

/*!
This will get called after the start request and a successful start
*/
void UiCaptureStreamer::handleStreamRunning() {
    // just accept the dialog, everything went well
    accept();
}

/*!
This will get called after the start request and an error
*/
void UiCaptureStreamer::handleStreamError() {
    QMessageBox::warning(this,
                         "Stream Error",
                         "Failed to set up server, check port!");
    // this sets the state back to stopped
    emit stopWorker();
    reject();
}

