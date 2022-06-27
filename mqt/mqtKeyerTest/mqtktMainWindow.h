#ifndef MQTKTMAINWINDOW_H
#define MQTKTMAINWINDOW_H

#include <QMainWindow>
#include <QtCharts>

namespace Ui {
class mqtktMainWindow;
}

class mqtktMainWindow : public QMainWindow
{
    Q_OBJECT

    void genTone(int16_t *dest, int tone, int samples, int rate, int rtime, double volmult);

    QLineSeries *originalSeries;
    QLineSeries *processedSeries;
    QChartView *chartView;
    QChart *chart;

public:
    explicit mqtktMainWindow(QWidget *parent = 0);
    ~mqtktMainWindow();

private slots:
    void on_compressorButton_clicked();

    void on_filterButton_clicked();

    void on_closeButton_clicked();

    void on_bpFilterButton_clicked();

    void on_showWaveButton_clicked();

private:
    Ui::mqtktMainWindow *ui;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;
};

#endif // MQTKTMAINWINDOW_H
