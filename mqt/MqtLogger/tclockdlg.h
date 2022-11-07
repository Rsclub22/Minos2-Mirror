#ifndef TCLOCKDLG_H
#define TCLOCKDLG_H

#include <QDialog>
#include <QTimer>

namespace Ui {
class TClockDlg;
}

class TClockDlg : public QDialog
{
    Q_OBJECT

    Ui::TClockDlg *ui;
public:
    explicit TClockDlg(QWidget *parent = nullptr);
    ~TClockDlg( ) override;

private slots:
    void on_OKButton_clicked();

    void on_clearButton_clicked();

    void on_applyButton_clicked();

    void on_cancelButton_clicked();

    void on_clockTicked();

    void on_dateEdit_dateChanged(const QDate &date);

    void on_timeEdit_timeChanged(const QTime &time);

private:
    void setEdits();
    void handleEdit();
    void doCloseEvent();

    bool initialised;
    qint64 initialCorrection;
    QTimer clockTick;
public Q_SLOTS:
    virtual void accept() override;
    virtual void reject() override;

};

#endif // TCLOCKDLG_H
