#include <QDebug>
#include "ddc.h"
#include "riff.h"
#include "MqtLogCompressor.h"
#include "mqtktWaveShowDialog.h"
#include "ui_mqtktWaveShowDialog.h"

bool sblog = true;
void trace(const QString &s)
{
    qDebug() << s;
}
class dvkFile
{
   public:
      QString fileName;
      bool loaded = false;
      bool frec = false;          // flag set to true if audio has been recorded
      unsigned int sampleRate = 0;       // system required sample rate
      unsigned long fsample = 0;        // number of bytes for each sound files
      int16_t *fptr = nullptr;          // data area for each sound file
      unsigned int rate = 0;
      int BitsPerSample = 0;
      int NumChannels = 0;

      bool LoadFile( QString &errmess )
      {
         if ( sblog )
         {
            trace( "Trying to open file " + fileName );
         }
         loaded = false;
         delete[] fptr;
         fptr = nullptr;
         // should be initiated by keyer, which should call the sound engine
         WaveFile inWave;
         int ret = inWave.OpenForRead( fileName );
         if ( ret != DDC_SUCCESS )
         {
            errmess = "Invalid WAV file " + fileName + "\n";
            if ( sblog )
            {
               trace( errmess );
            }
            return false;
         }
         else
         {
            rate = inWave.SamplingRate();
            BitsPerSample = inWave.BitsPerSample();
            NumChannels = inWave.NumChannels();
            frec = false;
            fsample = inWave.NumSamples();

            if ( rate == static_cast<unsigned int>(sampleRate) && BitsPerSample == 16 && NumChannels == 2 )
            {
               fptr = new int16_t[ fsample * 2 ];
               if ( inWave.ReadData( fptr, fsample * 2 ) == DDC_SUCCESS )
               {
                  if ( sblog )
                  {
                     trace( "File " + fileName + " opened samples = " + QString::number( fsample ) );
                  }
                  loaded = true;
               }
               else
               {
                  loaded = false;
               }
            }
            else
            {
               if ( sblog )
               {
                  trace( "File " + fileName + " wrong data format" );
               }
               loaded = false;
            }

         }
         return loaded;
      }
      dvkFile()
      {}
      ~dvkFile()
      {
         delete[] fptr;
         fptr = nullptr;
      }
};

WaveShowDialog::WaveShowDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WaveShowDialog)
{
    ui->setupUi(this);

    originalChart = new QChart();
    originalChart->legend()->hide();            // colours against series

    originalChartView = new QChartView(originalChart);
    originalChartView->setRenderHint(QPainter::Antialiasing);

    ui->chartLayout->addWidget(originalChartView);

    processedChart = new QChart();
    processedChart->legend()->hide();            // colours against series

    processedChartView = new QChartView(processedChart);
    processedChartView->setRenderHint(QPainter::Antialiasing);

    ui->chartLayout->addWidget(processedChartView);

    diffChart = new QChart();
    diffChart->legend()->hide();            // colours against series

    diffChartView = new QChartView(diffChart);
    diffChartView->setRenderHint(QPainter::Antialiasing);

    ui->chartLayout->addWidget(diffChartView);

    showSeries();
}

WaveShowDialog::~WaveShowDialog()
{
    delete ui;
}

void WaveShowDialog::on_closeButton_clicked()
{
    close();
}
void WaveShowDialog::showSeries()
{
    MqtLogCompressor compressor;

    dvkFile originalFile;
    originalFile.fileName = "C:/temp/CQF1.wav";
    originalFile.sampleRate = 48000;
    QString err;
    if (!originalFile.LoadFile(err))
    {
        trace(err);
        return;
    }

    compressor.setGamma(100);

    originalChart->removeAllSeries();       // removes AND DELETES
    processedChart->removeAllSeries();
    diffChart->removeAllSeries();

    originalSeries = new QLineSeries();
    processedSeries = new QLineSeries();
    diffSeries = new QLineSeries();

    // originalFile.fptr points to the data

    unsigned long bufferOffset = 0;

    int bufferStep = 100;

    int xaxis = 0;
    while (bufferOffset < originalFile.fsample * originalFile.NumChannels)
    {
        int slimit = bufferStep * originalFile.NumChannels;
        if (bufferOffset + slimit > originalFile.fsample * originalFile.NumChannels)
        {
            slimit = originalFile.fsample * originalFile.NumChannels - bufferOffset;
        }
        int sampleOffset = 0;
        qint16 imaxSample = 0;
        double dmaxSample = 0;
        while (sampleOffset < slimit)
        {
            qint16 is1 = originalFile.fptr[bufferOffset + sampleOffset];
            qint16 is2 = originalFile.fptr[bufferOffset + sampleOffset + 1];

            double ds1 = is1;
            double ds2 = is2;

            ds1 /= 32768.0;
            ds2 /= 32768.0;

            compressor.process(ds1, ds2);

            ds1 *= 32768.0;
            ds2 *= 32768.0;

            qint16 ival = std::max(abs(is1), abs(is2));
            imaxSample = std::max(imaxSample, ival);

            double dval = std::max(abs(ds1), abs(ds2));
            dmaxSample = std::max(dmaxSample, dval);


            sampleOffset +=  originalFile.NumChannels;
        }

        originalSeries->append(xaxis, imaxSample);
        processedSeries->append(xaxis, dmaxSample);
        diffSeries->append(xaxis, imaxSample - dmaxSample);

        bufferOffset += bufferStep * originalFile.NumChannels;
        xaxis++;
    }
    originalChart->addSeries(originalSeries);
    originalChart->createDefaultAxes();
    originalChart->setTitle("mqt Original Signal");

    processedChart->addSeries(processedSeries);
    processedChart->createDefaultAxes();
    processedChart->setTitle("mqt Processed Signal");

    diffChart->addSeries(diffSeries);
    diffChart->createDefaultAxes();
    diffChart->setTitle("mqt Difference");
}


