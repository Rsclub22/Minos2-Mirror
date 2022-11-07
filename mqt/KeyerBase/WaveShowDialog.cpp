#include "riff.h"
#include "sbdriver.h"

#include "WaveShowDialog.h"
#include "ui_WaveShowDialog.h"

WaveShowDialog::WaveShowDialog(QWidget *parent, int fno) :
    QDialog(parent),
    ui(new Ui::WaveShowDialog), fno(fno)
{
    ui->setupUi(this);

    QSettings settings;
    QByteArray geometry = settings.value("KeyerWaveShow/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);


    originalChart = new QChart();
    originalChart->legend()->hide();

    originalChartView = new QChartView(originalChart);
    originalChartView->setRenderHint(QPainter::Antialiasing);

    ui->chartLayout->addWidget(originalChartView);

    showSeries();
}

WaveShowDialog::~WaveShowDialog()
{
    delete ui;
}
void WaveShowDialog::doCloseEvent()
{
    QSettings settings;
    settings.setValue("KeyerWaveShow/geometry", saveGeometry());
}
void WaveShowDialog::reject()
{
    doCloseEvent();
    QDialog::reject();
}
void WaveShowDialog::accept()
{
    doCloseEvent();
    QDialog::accept();
}

void WaveShowDialog::on_closeButton_clicked()
{
    accept();
}
void WaveShowDialog::showSeries()
{
    originalChart->removeAllSeries();       // removes AND DELETES

    const dvkFile *originalFile = SoundSystemDriver::getSbDriver()->getFile(fno);
    if (!originalFile || !originalFile->loaded)
    {
        return;
    }


    originalSeries = new QLineSeries();

    // originalFile.fptr points to the data

    unsigned long bufferOffset = 0;

    int bufferStep = 100;

    int xaxis = 0;
    while (bufferOffset < originalFile->fsample * originalFile->NumChannels)
    {
        int slimit = bufferStep * originalFile->NumChannels;
        if (bufferOffset + slimit > originalFile->fsample * originalFile->NumChannels)
        {
            slimit = originalFile->fsample * originalFile->NumChannels - bufferOffset;
        }
        int sampleOffset = 0;
        qint16 imaxSample = 0;
        while (sampleOffset < slimit)
        {
            qint16 is1 = originalFile->fptr[bufferOffset + sampleOffset];
            qint16 is2 = originalFile->fptr[bufferOffset + sampleOffset + 1];

            qint16 ival = std::max(abs(is1), abs(is2));
            imaxSample = std::max(imaxSample, ival);


            sampleOffset +=  originalFile->NumChannels;
        }

        originalSeries->append(xaxis, imaxSample);

        bufferOffset += bufferStep * originalFile->NumChannels;
        xaxis++;
    }
    originalChart->addSeries(originalSeries);
    originalChart->createDefaultAxes();
    originalChart->setTitle("mqt Original Signal");
}

