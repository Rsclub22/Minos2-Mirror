#ifndef WAVESHOWDIALOG_H
#define WAVESHOWDIALOG_H

#include <QDialog>
#include <QtCharts>

namespace Ui {
class WaveShowDialog;
}

class WaveShowDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WaveShowDialog(QWidget *parent, int fno);
    ~WaveShowDialog();

private slots:
    void on_closeButton_clicked();

public Q_SLOTS:
   virtual void accept() override;
   virtual void reject() override;

private:
    Ui::WaveShowDialog *ui;

    int fno = -1;

    QLineSeries *originalSeries = nullptr;
    QChartView *originalChartView = nullptr;
    QChart *originalChart = nullptr;

    void doCloseEvent();

    void showSeries();
    void on_showButton_clicked();
};

#endif // WAVESHOWDIALOG_H
