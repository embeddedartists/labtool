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
#include "uicaptureexporter.h"

#include <QFormLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QProgressDialog>

#define FORMAT_WIDGET_INDEX (1)

/*!
    \class UiCaptureExporter
    \brief This class is responsible for export of captured signal data.

    \ingroup Capture

    A dialog window will be presented to the user with a number of
    choices and settings related to export of data. The supported formats
    as well as the actual export to file is handled within this class.
*/

/*!
    Constructs the UiCaptureExporter with the given \a parent.
*/
UiCaptureExporter::UiCaptureExporter(CaptureDevice* device, QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(tr("Export Data"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    mFormatWidget = NULL;
    mCaptureDevice = device;

    // Deallocation: Ownership changed when calling setLayout.
    mMainLayout = new QVBoxLayout();


    // Deallocation: Re-parented when calling mMainLayout->addLayout.
    QFormLayout* formLayout = new QFormLayout();

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mExportFormatBox = new QComboBox(this);
    foreach(QString t, exportFormats()) {
        mExportFormatBox->addItem(t);
    }
    connect(mExportFormatBox, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(handleFormatChanged(QString)));
    mExportFormatBox->setToolTip(tr("Select export format"));
    formLayout->addRow(tr("Format: "), mExportFormatBox);

    mMainLayout->addLayout(formLayout);

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QPushButton* exportBtn = new QPushButton("Export", this);
    connect(exportBtn, SIGNAL(clicked()), this, SLOT(exportData()));

    QPushButton* cancelBtn = new QPushButton("Cancel", this);
    connect(cancelBtn, SIGNAL(clicked()), this, SLOT(reject()));

    // Deallocation: Re-parented when calling mMainLayout->addLayout.
    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->addWidget(exportBtn);
    hLayout->addWidget(cancelBtn);
    hLayout->addStretch();
    mMainLayout->addLayout(hLayout);

    mMainLayout->addStretch();

    setLayout(mMainLayout);

    // force format widget to be displayed
    handleFormatChanged(exportFormats().at(0));
}

/*
    ---------------------------------------------------------------------------
    >>>> BEGIN -- Handle export formats>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    ---------------------------------------------------------------------------
*/


#define FORMAT_CSV "CSV"

/*!
    Returns the supported export formats.
*/
QStringList UiCaptureExporter::exportFormats()
{
    return QList<QString>()
            << FORMAT_CSV;
}

/*!
    Creates widget used for a the given export format. The widget contains
    settings specific for the given \a format.
*/
QWidget* UiCaptureExporter::createFormat(QString format)
{
    if (FORMAT_CSV == format) {
        return createFormatCsv();
    }


    return NULL;
}

/*!
    Export data to file with using the format \a format. Settings are
    retrieved from the widget \a w.
*/
void UiCaptureExporter::exportData(QString format, QWidget* w)
{
    if (FORMAT_CSV == format) {
        exportToCsv(w);
    }
}

/*!
    Create a widget for CSV format settings.
*/
QWidget* UiCaptureExporter::createFormatCsv()
{
    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QFrame* w = new QFrame(this);
    w->setFrameShape(QFrame::StyledPanel);

    // Deallocation: Ownership changed when calling setLayout
    QFormLayout* l = new QFormLayout();

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QComboBox* delimBox = new QComboBox(w);
    delimBox->setObjectName("csvDelim");
    delimBox->addItem("Comma", 0);
    delimBox->addItem("Tab", 1);
    l->addRow(tr("Delimeter:"), delimBox);

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QComboBox* sampleBox = new QComboBox(w);
    sampleBox->setObjectName("csvSample");
    sampleBox->addItem("Sample time", 0);
    sampleBox->addItem("Sample number", 1);
    l->addRow(tr("Sample column:"), sampleBox);

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QComboBox* rowBox = new QComboBox(w);
    rowBox->setObjectName("csvRow");
    rowBox->addItem("One row per sample", 0);
    rowBox->addItem("One row per change", 1);
    l->addRow(tr("Row:"), rowBox);

    w->setLayout(l);


    return w;
}

/*!
    Export signal data in CSV format.
*/
void UiCaptureExporter::exportToCsv(QWidget* w)
{
    bool delimAsComma = true;
    bool sampleAsTime = true;
    bool rowEachSample = true;

    do {
        QComboBox* box = w->findChild<QComboBox*>("csvDelim");
        if (box == NULL) break;
        delimAsComma = (box->itemData(box->currentIndex()).toInt() == 0);

        box = w->findChild<QComboBox*>("csvSample");
        if (box == NULL) break;
        sampleAsTime = (box->itemData(box->currentIndex()).toInt() == 0);

        box = w->findChild<QComboBox*>("csvRow");
        if (box == NULL) break;
        rowEachSample = (box->itemData(box->currentIndex()).toInt() == 0);


    } while(false);


    do {

        QString filePath = QFileDialog::getSaveFileName(
                    this,
                    tr("Save File"),
                    QDir::currentPath()+"/export.csv",
                    "Comma Separated values (*.csv)");

        if (filePath.isNull() || filePath.isEmpty()) break;

        QFile file(filePath);
        file.open(QIODevice::Truncate | QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);


        QChar delim = ',';
        if (!delimAsComma) {
            delim = '\t';
        }

        QList<DigitalSignal*> digitalSignals = mCaptureDevice->digitalSignals();
        QList<AnalogSignal*> analogSignals = mCaptureDevice->analogSignals();

        QList<QVector<int>*> digitalData;
        QList<QVector<double>*> analogData;

        int numSamples = -1;
        int sampleRate = mCaptureDevice->usedSampleRate();

        //  >>> Header >>>>>>>>>>>>>>>>>>>>>>>>>>

        out << "sample";

        foreach(DigitalSignal* s, digitalSignals) {
            QVector<int>* data = mCaptureDevice->digitalData(s->id());
            if (data == NULL) continue;

            out << delim << QString("D%1").arg(s->id());

            digitalData.append(data);
            if (numSamples == -1 || data->size() < numSamples) {
                numSamples = data->size();
            }
        }

        foreach(AnalogSignal* s, analogSignals) {
            QVector<double>* data = mCaptureDevice->analogData(s->id());
            if (data == NULL) continue;

            out << delim << QString("A%1").arg(s->id());

            analogData.append(data);
            if (numSamples == -1 || data->size() < numSamples) {
                numSamples = data->size();
            }
        }

        out << '\n';

        //  <<< Header <<<<<<<<<<<<<<<<<<<<<<<<<<


        //  >>> Samples >>>>>>>>>>>>>>>>>>>>>>>>>>

        QProgressDialog progress("Exporting data", "Abort", 0, numSamples, this);
        progress.setWindowModality(Qt::WindowModal);

        QString lastSampleRow;
        for (int i = 0; i < numSamples; i++) {

            // do not call progress or wasCanceled for each sample
            // since this greatly slows down the export
            if ((i % 100) == 0 || i == numSamples-1) {
                progress.setValue(i);

                if (progress.wasCanceled()) {
                    break;
                }
            }

            QString sample = QString("%1").arg(i);
            if (sampleAsTime) {
                sample = QString::number((double)i/sampleRate);
            }

            QString sampleRow;
            foreach(QVector<int>* d, digitalData) {
                sampleRow.append(delim);
                sampleRow.append(QString("%1").arg(d->at(i)));

            }

            foreach(QVector<double>* d, analogData) {
                sampleRow.append(delim);
                sampleRow.append(QString("%1").arg(d->at(i)));
                //out << delim << d->at(i);
            }

            // only exporting changes
            if (!rowEachSample && sampleRow == lastSampleRow) {
                continue;
            }

            out << sample;
            out << sampleRow;

            lastSampleRow = sampleRow;



            out << '\n';

        }

        //  <<< Samples <<<<<<<<<<<<<<<<<<<<<<<<<<


        file.close();

    } while(false);


}

/*
    ---------------------------------------------------------------------------
    <<<< END -- Handle export formats <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    ---------------------------------------------------------------------------
*/

/*!
    The selected export format has changed to \a format.
*/
void UiCaptureExporter::handleFormatChanged(QString format)
{
    if (mFormatWidget != NULL) {
        mFormatWidget->close();
        delete mFormatWidget;
    }

    mFormatWidget = createFormat(format);
    mMainLayout->insertWidget(FORMAT_WIDGET_INDEX, mFormatWidget);


    adjustSize();
}

/*!
    Called when the user clicks the Export button.
*/
void UiCaptureExporter::exportData()
{
    exportData(mExportFormatBox->currentText(), mFormatWidget);

    accept();
}
