#ifndef UICAPTURESTREAMER_H
#define UICAPTURESTREAMER_H

#include <QDialog>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QThread>
#include <QLabel>

#include "streamworker.h"

#include "device/capturedevice.h"


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
    ~UiCaptureStreamer();
    void stopStreaming();

private:

    QThread* workerThread;
    StreamWorker* streamWorker;

    CaptureDevice* mCaptureDevice;
    QVBoxLayout* mMainLayout;

    QSpinBox* mPortSpinBox;

private slots:
    void handleStreamBtnPressed();
    void handleStreamRunning();
    void handleStreamError();

signals:
    void startWorker(int port);
    void stopWorker();
    void destroyWorker();
    void destroyThread();

};

#endif // UICAPTURESTREAMER_H
