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

private slots:
    void on_closeButton_clicked();

    void on_recalcButton_clicked();

    void compressionChanged();
private:
    Ui::WaveShowDialog *ui;

    double window = 10.0;       // milliseconds
    double threshold = -10.0;
    double ratio = 0.1;
    double attack = 1.0;     // 1ms seems like a good look-ahead to me
    double release = 10.0; // 10ms release is good
    double makeUpGain = 0;  // db

    void getParams();
    void setSliders();

    QLineSeries *originalSeries = nullptr;
    QChartView *originalChartView = nullptr;
    QChart *originalChart = nullptr;

    QLineSeries *processedSeries = nullptr;
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
};

#endif // MQTKTWAVESHOWDIALOG_H
