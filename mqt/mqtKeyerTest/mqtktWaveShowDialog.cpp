#include "MTrace.h"
#include "ddc.h"
#include "riff.h"
#include "adis_filter.h"
#include "SimpleComp.h"
#include "mqtktWaveShowDialog.h"
#include "ui_mqtktWaveShowDialog.h"

const double pi = 3.141592653;
extern bool sblog;

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
    sblog = true;

    QSettings settings;
    QByteArray geometry = settings.value("WaveShowDialog/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    baseChart = new QChart();
    baseChart->legend()->hide();
    baseChartView = new QChartView(baseChart);
    baseChartView->setRenderHint(QPainter::Antialiasing);
    ui->chartLayout->addWidget(baseChartView);

    originalChart = new QChart();
    originalChart->legend()->hide();

    originalChartView = new QChartView(originalChart);
    originalChartView->setRenderHint(QPainter::Antialiasing);

    ui->chartLayout->addWidget(originalChartView);

    processedChart = new QChart();
    processedChart->legend()->hide();

    processedChartView = new QChartView(processedChart);
    processedChartView->setRenderHint(QPainter::Antialiasing);

    ui->chartLayout->addWidget(processedChartView);

    diffChart = new QChart();
    diffChart->legend()->hide();

    diffChartView = new QChartView(diffChart);
    diffChartView->setRenderHint(QPainter::Antialiasing);

    ui->chartLayout->addWidget(diffChartView);

    ui->compFrame->setLayout(new QVBoxLayout());

    windowFrame = new SliderSpinner(this, tr("Window (ms)"), Qt::Horizontal, 1, +100, 1);
    ui->compFrame->layout()->addWidget(windowFrame);
    connect(windowFrame, &SliderSpinner::valueChanged, this, &WaveShowDialog::compressionChanged);

    thresholdFrame = new SliderSpinner(this, tr("Threshold (db below max)"), Qt::Horizontal, -40, 0, 0);
    ui->compFrame->layout()->addWidget(thresholdFrame);
    connect(thresholdFrame, &SliderSpinner::valueChanged, this, &WaveShowDialog::compressionChanged);

    ratioFrame = new SliderSpinner(this, tr("Compression Ratio"), Qt::Horizontal, 0, +50, 0);
    ui->compFrame->layout()->addWidget(ratioFrame);
    connect(ratioFrame, &SliderSpinner::valueChanged, this, &WaveShowDialog::compressionChanged);

    attackFrame = new SliderSpinner(this, tr("Attack (ms)"), Qt::Horizontal, 1, 100, 0);
    ui->compFrame->layout()->addWidget(attackFrame);
    connect(attackFrame, &SliderSpinner::valueChanged, this, &WaveShowDialog::compressionChanged);

    releaseFrame = new SliderSpinner(this, tr("Release (ms)"), Qt::Horizontal, 1, 100, 0);
    ui->compFrame->layout()->addWidget(releaseFrame);
    connect(releaseFrame, &SliderSpinner::valueChanged, this, &WaveShowDialog::compressionChanged);

    makeUpGainFrame = new SliderSpinner(this, tr("Makeup Gain (db)"), Qt::Horizontal, 0, +20, 0);
    ui->compFrame->layout()->addWidget(makeUpGainFrame);
    connect(makeUpGainFrame, &SliderSpinner::valueChanged, this, &WaveShowDialog::compressionChanged);

    ui->compFrame->setContentsMargins(0, 0, 0, 0);

    setSliders();

    getParams();
    showComp();
    showSeries();
}

WaveShowDialog::~WaveShowDialog()
{
    delete [] toneptr;

    delete ui;
}
void WaveShowDialog::moveEvent(QMoveEvent *event)
{
    QSettings settings;
    settings.setValue("WaveShowDialog/geometry", saveGeometry());
    QDialog::moveEvent(event);
}
void WaveShowDialog::resizeEvent(QResizeEvent * event)
{
    QSettings settings;
    settings.setValue("WaveShowDialog/geometry", saveGeometry());
    QDialog::resizeEvent(event);
}
void WaveShowDialog::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        QSettings settings;
        settings.setValue("WaveShowDialog/geometry", saveGeometry());
    }
}
void WaveShowDialog::on_closeButton_clicked()
{
    close();
}
void WaveShowDialog::genTone(int16_t *dest, int tone, int samples, int rate, int rtime, double volmult )
{

   double deltaAngle = 2 * pi * tone / rate;
   double yk = 2 * cos( deltaAngle );
   double y1 = sin ( -2 * deltaAngle );
   double y2 = sin ( -deltaAngle );

#define CHUNKSIZE 1024

   int16_t *buff = new int16_t [ CHUNKSIZE ];


   for ( int buffstart = 0; buffstart < samples * 2; buffstart += CHUNKSIZE * 2 )
   {
      int16_t * destptr = dest + buffstart;
      int i;
      for ( i = 0; i < CHUNKSIZE && buffstart + i*2 < samples * 2; i++ )
      {
         double y3 = yk * y2 - y1;
         y1 = y2;
         y2 = y3;
         if ( buffstart + i * 2 < rtime * 2 )
         {
            buff[ i ] = int16_t( y3 * ( ( volmult * (( buffstart + i )/2) ) / rtime ) );	// not full volume
         }
         else
         {
            if ( buffstart + i * 2 > ( samples - rtime ) * 2 )
            {
               buff[ i ] = int16_t( y3 * ( ( volmult * (( samples * 2 - ( buffstart + i*2 ) )/2) ) / rtime ) );	// not full volume
            }
            else
            {
               buff[ i ] = int16_t( y3 * volmult );	// not full volume
            }
         }

      }
      // write (or add in place) i bytes to the handle

      for (int j = 0; j < i; j++)
      {
          destptr[j * 2] = buff[j];
          destptr[j * 2 + 1] = buff[j];
      }
   }
   delete [] buff;
   buff = nullptr;
}

void WaveShowDialog::showComp()
{
    // build single frequency buffer, pass it through compressor at varying volume mult,
    // plot multiplier against original and old levels

    int samples = 49000;    // 1 secs worth
    int tone = 1000;
    if (!toneptr)
    {
        int ramptime = 0;
        const double tvolmult = 32767.0 * 100.0 / 100.0;

        toneptr = new int16_t [ samples * 2 ];

        genTone( toneptr, tone, samples, samples, ramptime, tvolmult );
    }
    chunkware_simple::SimpleCompRms compressor;
    compressor.setSampleRate(samples);

    compressor.setWindow(window);       // milliseconds
    compressor.setThresh( threshold );
    compressor.setRatio( ratio );
    compressor.setAttack( attack );     // 1ms seems like a good look-ahead to me
    compressor.setRelease( release ); // 10ms release is good

    compressor.initRuntime();

    baseChart->removeAllSeries();       // removes AND DELETES

    baseSeries = new QLineSeries();
    processedBaseSeries = new QLineSeries();

    for (int s = 0; s <= 100; s++)
    {
        qreal volmult = s * 0.01;

        int16_t * q = reinterpret_cast<  int16_t * > ( toneptr );
        int16_t maxvol = 0;

        for (int i = 0; i < samples/tone ; i++)
        {
            int t1 = q[i * 2];
            int t2 = q[i * 2 + 1];
            double initi1 = t1 * volmult;
            double initi2 = t2 * volmult;

            double s1 = initi1;
            double s2 = initi2;

            s1 /= 32768.0;
            s2 /= 32768.0;

            compressor.process(s1, s2);

            s1 *= chunkware_simple::dB2lin(makeUpGain);
            s2 *= chunkware_simple::dB2lin(makeUpGain);

            s1 *= 32768.0;
            s2 *= 32768.0;

            int16_t sample = static_cast<int16_t>(std::abs( (s1 + s2)/2 ));
            if ( sample > maxvol )
               maxvol = sample;
        }

        baseSeries->append(s, volmult);

        processedBaseSeries->append(s, maxvol/32768.0);
    }

    baseChart->addSeries(baseSeries);
    baseChart->addSeries(processedBaseSeries);

    baseChart->createDefaultAxes();
    baseChart->setTitle("mqt Compressor Test");
}

void WaveShowDialog::showSeries()
{
    dvkFile originalFile;
    originalFile.fileName = "C:/temp/CQF1.wav";
    originalFile.sampleRate = 48000;
    QString err;
    if (!originalFile.LoadFile(err))
    {
        trace(err);
        return;
    }

    chunkware_simple::SimpleCompRms compressor;
    compressor.setSampleRate(48000);

    compressor.setWindow(window);       // milliseconds
    compressor.setThresh( threshold );
    compressor.setRatio( ratio );
    compressor.setAttack( attack );     // 1ms seems like a good look-ahead to me
    compressor.setRelease( release ); // 10ms release is good

    compressor.initRuntime();

    BWBandPass* filter1 = create_bw_band_pass_filter(4, 48000, 100, 3000);   // order, sampling freq, lower half power, upper half power
    BWBandPass* filter2 = create_bw_band_pass_filter(4, 48000, 100, 3000);   // order, sampling freq, lower half power, upper half power


    originalChart->removeAllSeries();       // removes AND DELETES
    processedChart->removeAllSeries();
    diffChart->removeAllSeries();

    originalSeries = new QLineSeries();
    processedSeries = new QLineSeries();
    diffSeries = new QLineSeries();

    limitSeries = new QLineSeries();

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

            ds1 =  bw_band_pass(filter1, ds1);
            ds2 =  bw_band_pass(filter2, ds2);
            compressor.process(ds1, ds2);

            ds1 *= 32768.0;
            ds2 *= 32768.0;

            ds1 *= chunkware_simple::dB2lin(makeUpGain);
            ds2 *= chunkware_simple::dB2lin(makeUpGain);

            qint16 ival = std::max(abs(is1), abs(is2));
            imaxSample = std::max(imaxSample, ival);

            double dval = std::max(abs(ds1), abs(ds2));
            dmaxSample = std::max(dmaxSample, dval);


            sampleOffset +=  originalFile.NumChannels;
        }

        if (dmaxSample > 32768)
        {
            dmaxSample = 40000;     // clip the output
        }
        originalSeries->append(xaxis, imaxSample);
        processedSeries->append(xaxis, dmaxSample);
        limitSeries->append(xaxis, 32768.0);
        diffSeries->append(xaxis, imaxSample - dmaxSample);

        bufferOffset += bufferStep * originalFile.NumChannels;
        xaxis++;
    }
    originalChart->addSeries(originalSeries);
    originalChart->createDefaultAxes();
    originalChart->setTitle("mqt Original Signal");

    processedChart->addSeries(processedSeries);
    processedChart->addSeries(limitSeries);
    processedChart->createDefaultAxes();
    processedChart->setTitle("mqt Processed Signal");

    diffChart->addSeries(diffSeries);
    diffChart->createDefaultAxes();
    diffChart->setTitle("mqt Difference");

    free_bw_band_pass(filter1);
    free_bw_band_pass(filter2);
}



void WaveShowDialog::on_recalcButton_clicked()
{
    // reset the compressor parameters and re-display

    getParams();
    showComp();
    showSeries();
}

void WaveShowDialog::getParams()
{
    window = windowFrame->getValue();       // milliseconds
    threshold = thresholdFrame->getValue();

    double rrange = ratioFrame->maximum() - ratioFrame->minimum() + 1;
    ratio = 1 - ratioFrame->getValue()/rrange;
    attack = attackFrame->getValue();     // 1ms seems like a good look-ahead to me
    release = releaseFrame->getValue(); // 10ms release is good
    makeUpGain = makeUpGainFrame->getValue();
}

void WaveShowDialog::setSliders()
{
    windowFrame->setValue(window);       // milliseconds
    thresholdFrame->setValue(threshold);
    double rrange = ratioFrame->maximum() - ratioFrame->minimum() + 1;
    ratioFrame->setValue(rrange * (1 - ratio));
    attackFrame->setValue(attack);     // 1ms seems like a good look-ahead to me
    releaseFrame->setValue(release); // 10ms release is good
    makeUpGainFrame->setValue(makeUpGain);
}
void WaveShowDialog::compressionChanged()
{
}
