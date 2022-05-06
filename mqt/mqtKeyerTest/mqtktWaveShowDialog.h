#ifndef MQTKTWAVESHOWDIALOG_H
#define MQTKTWAVESHOWDIALOG_H

#include <QDialog>
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

private:
    Ui::WaveShowDialog *ui;

    QLineSeries *originalSeries = nullptr;
    QChartView *originalChartView = nullptr;
    QChart *originalChart = nullptr;

    QLineSeries *processedSeries = nullptr;
    QChartView *processedChartView = nullptr;
    QChart *processedChart = nullptr;

    QLineSeries *diffSeries = nullptr;
    QChartView *diffChartView = nullptr;
    QChart *diffChart = nullptr;

    void showSeries();
};

#endif // MQTKTWAVESHOWDIALOG_H
