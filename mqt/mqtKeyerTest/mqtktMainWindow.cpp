#include "mqtktMainWindow.h"
#include "ui_mqtktMainWindow.h"

#include "SimpleComp.h"

const double pi = 3.141592653;

mqtktMainWindow::mqtktMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::mqtktMainWindow)
{
    ui->setupUi(this);

    chart = new QChart();
    chart->legend()->hide();            // colours against series

    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    ui->chartLayout->addWidget(chartView);
    show();
}

mqtktMainWindow::~mqtktMainWindow()
{
    delete ui;
}
void mqtktMainWindow::genTone(int16_t *dest, int tone, int samples, int rate, int rtime, double volmult )
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

void mqtktMainWindow::on_compressorButton_clicked()
{
    // build single frequency buffer, pass it through compressor at varying volume mult,
    // plot multiplier against original and old levels

    int tone = 1000;
    int samples = 49000;    // 1 secs worth
    int ramptime = 0;
    const double volmult = 32767.0 * 100.0 / 100.0;

    int16_t *toneptr = new int16_t [ samples * 2 ];

    genTone( toneptr, tone, samples, samples, ramptime, volmult );


    chunkware_simple::SimpleCompRms compressor;

    compressor.setSampleRate(samples);
    compressor.setWindow(10);       // milliseconds
    compressor.setThresh( -10 );
    compressor.setRatio( 0.1 );
    compressor.setAttack( 1.0 );     // 1ms seems like a good look-ahead to me
    compressor.setRelease( 10.0 ); // 10ms release is good
    compressor.initRuntime();


    chart->removeAllSeries();       // removes AND DELETES

    originalSeries = new QLineSeries();
    processedSeries = new QLineSeries();

    for (int i = 0; i <= 100; i++)
    {
        qreal volmult = i * 0.01;


        int16_t * q = reinterpret_cast<  int16_t * > ( toneptr );
        int16_t maxvol = 0;

        for (int i = 0; i < samples ; i++)
        {
            double initi1 = q[i * 2] * volmult;
            double initi2 = q[i * 2 + 1] * volmult;

            double s1 = initi1;
            double s2 = initi2;

            s1 /= 32768.0;
            s2 /= 32768.0;

            compressor.process(s1, s2);


            s1 *= 32768.0;
            s2 *= 32768.0;

            qreal val1 = s1;
            qreal val2 = s2;

            if (val1 > 32767.0)
                val1 = 32767.0;
            if (val1 < -32767.0)
                val1 = -32767.0;

            if (val2 > 32767.0)
                val2 = 32767.0;
            if (val2 < -32767.0)
                val2 = -32767.0;

            int16_t sample = static_cast<int16_t>(std::abs( (val1 + val2)/2 ));
            if ( sample > maxvol )
               maxvol = sample;
        }

        originalSeries->append(i, volmult);

        processedSeries->append(i, maxvol/32768.0);
    }

    delete [] toneptr;

    chart->addSeries(originalSeries);
    chart->addSeries(processedSeries);

    chart->createDefaultAxes();
    chart->setTitle("mqt Compressor Test");
}

class LPFilter
{
    int mNumChannels;
    // past data
    double mZx[4];
    double mZy[4];
    // filter coefficients
    double a0;
    double a1;
    double a2;
    double b1;
    double b2;
public:
    LPFilter(int channels, double corner, double sampleRate);
    inline double filterSample (const double inSample, const int channel);
};

LPFilter::LPFilter (int channels, double corner, double sampleRate):mNumChannels(channels)
{
    mZx[0] = mZx[1] = mZx[2] = mZx[3] = 0.0;
    mZy[0] = mZy[1] = mZy[2] = mZy[3] = 0.0;

    double mQuality = 0.707;

    double theta = 2. * M_PI * (corner/sampleRate);
    double d = 0.5 * (1. / mQuality) * sin(theta);
    double beta = 0.5 * ( (1. - d) / (1. + d) );
    double gamma = (0.5 + beta) * cos(theta);

    a0 = 0.5 * (0.5 + beta - gamma);
    a1 = 0.5 + beta - gamma;

    a2 = a0;
    b1 = -2. * gamma;
    b2 = 2. * beta;
}

double LPFilter::filterSample (const double inSample, const int channel)
{
    // Derived from
    // http://creatingsound.com/2014/02/dsp-audio-programming-series-part-2/

    double outSample;
    int idx0 = mNumChannels * channel;
    int idx1 = idx0 + 1;

    outSample = (a0 * inSample)
                + (a1 * mZx[idx0])
                + (a2 * mZx[idx1])
                - (b1 * mZy[idx0])
                - (b2 * mZy[idx1]);

    mZx[idx1] = mZx[idx0];
    mZx[idx0] = inSample;
    mZy[idx1] = mZy[idx0];
    mZy[idx0] = outSample;

    return outSample;
}

void mqtktMainWindow::on_filterButton_clicked()
{
    // build series of frequency buffers, pass each through filter
    // plot frequency against original and old levels

    chart->removeAllSeries();       // removes AND DELETES

    int samples = 49000;    // 1 secs worth
    int corner = 2000;

    int ramptime = 0;
    LPFilter mCoeffs(2, corner, samples);

    originalSeries = new QLineSeries();
    processedSeries = new QLineSeries();

    int16_t *toneptr = new int16_t [ samples * 2 ];
    int16_t *toneop = new int16_t [ samples * 2 ];

    for (int tone = 50; tone < 10000; tone += 100)
    {
        const double volmult = 32767.0 * 100.0 / 100.0;
        genTone( toneptr, tone, samples, samples, ramptime, volmult );

        int imax = 0;
        // and apply low pass filter
        toneop[0] = toneptr[0] + 0;
        for (int n=1; n < samples ; n++)
        {
            for (int chan = 0; chan < 2; ++chan)
            {
                qreal o =  mCoeffs.filterSample(toneptr[n * 2 + chan], chan);
                toneop[n * 2 + chan] = static_cast<int16_t>(o);
                int m = abs(static_cast<int>(o));
                if (m > imax)
                    imax = m;
            }
        }

        originalSeries->append(tone, 1.0);

        qreal maxvol = imax/32768.0;

        processedSeries->append(tone, maxvol);
    }
    chart->addSeries(originalSeries);
    chart->addSeries(processedSeries);

    chart->createDefaultAxes();
    chart->setTitle("mqt Filter Test");
}

void mqtktMainWindow::on_closeButton_clicked()
{
    close();
}

void mqtktMainWindow::on_toneButton_clicked()
{
    // we want to draw the tone. Actually, we'd like to FFT it!
    int samples = 49000;    // 1 secs worth
    int tone = 1000;

    int ramptime = 0;

    originalSeries = new QLineSeries();
    processedSeries = new QLineSeries();

    int16_t *toneptr = new int16_t [ samples * 2 ];
    int16_t *toneop = new int16_t [ samples * 2 ];

    const double volmult = 32767.0 * 100.0 / 100.0;
    genTone( toneptr, tone, samples, samples, ramptime, volmult );}
