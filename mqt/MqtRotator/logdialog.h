/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016
//
//
/////////////////////////////////////////////////////////////////////////////



#ifndef LOGDIALOG_H
#define LOGDIALOG_H

#include <QDialog>

namespace Ui {
class LogDialog;
}

class LogDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LogDialog(QWidget *parent = nullptr);
    ~LogDialog();

signals:
    void showLogDialog();
    void bearingLogConfigChanged();

private slots:


    void on_logDirBrowsePb_clicked();
    void on_logFilenameEdit_textChanged(const QString &arg1);

    void on_logDialogOkPb_accepted();

    void on_logDialogOkPb_rejected();

    void on_bearingDifferenceSBox_valueChanged(int arg1);

    void on_logcheckBox_toggled(bool checked);
public slots:
    void loadLogConfig();
private:
    Ui::LogDialog *ui;
    QString directory;
    QString filename;
    bool dirChanged = false;
    bool filenameChanged = false;
    bool bearDiffChanged = false;
    bool logEnabledChanged = false;
    int bearingDiff = 0;
    bool logEnabled = false;
    int polltime;       // file write interval

    void saveLogConfig();
    void readLogConfig();
    void keyPressEvent(QKeyEvent *event);
};

#endif // LOGDIALOG_H
