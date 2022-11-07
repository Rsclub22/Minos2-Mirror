#ifndef MQTKTWAVESHOWDIALOG_H
#define MQTKTWAVESHOWDIALOG_H

#include <QDialog>
#include "SliderSpinner.h"
#include <QtCharts>

namespace Ui {
class WaveShowDialog;
}

class WaveShowDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WaveShowDialog(QWidget *parent = nullptr);
    ~WaveShowDialog();

    void showComp();
private slots:
    void on_closeButton_clicked();

    void on_recalcButton_clicked();

    void compressionChanged();
private:
    Ui::WaveShowDialog *ui;

    double window = 3.0;       // milliseconds
    double threshold = -30.0;
    double ratio = 1.0/40;
    double attack = 1.0;     // 1ms seems like a good look-ahead to me
    double release = 3.0; // 10ms release is good
    double makeUpGain = 7.5;  // db

    int16_t *toneptr = nullptr;

    void getParams();
    void setSliders();

    QLineSeries *baseSeries;
    QLineSeries *processedBaseSeries;
    QChartView *baseChartView;
    QChart *baseChart;

    QLineSeries *originalSeries = nullptr;
    QChartView *originalChartView = nullptr;
    QChart *originalChart = nullptr;

    QLineSeries *processedSeries = nullptr;
    QLineSeries *limitSeries = nullptr;
    QChartView *processedChartView = nullptr;
    QChart *processedChart = nullptr;

    QLineSeries *diffSeries = nullptr;
    QChartView *diffChartView = nullptr;
    QChart *diffChart = nullptr;

    SliderSpinner *windowFrame = nullptr;
    SliderSpinner *thresholdFrame = nullptr;
    SliderSpinner *ratioFrame = nullptr;
    SliderSpinner *attackFrame = nullptr;
    SliderSpinner *releaseFrame = nullptr;
    SliderSpinner *makeUpGainFrame = nullptr;


    void showSeries();
    void genTone(int16_t *dest, int tone, int samples, int rate, int rtime, double volmult);

    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;

};

#endif // MQTKTWAVESHOWDIALOG_H
