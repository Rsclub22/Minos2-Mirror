/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2017
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////




#ifndef RIGMEMDIALOG_H
#define RIGMEMDIALOG_H
#include "base_pch.h"
#include "rigmemcommondata.h"

class LoggerContestLog;

namespace Ui {
class RigMemDialog;
}

class RigMemDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RigMemDialog(QWidget *parent = nullptr);
    ~RigMemDialog();

    void setLogData(memoryData::memData*, int buttonNumber, LoggerContestLog *ct);


private slots:
    void on_okButton_clicked();
    void on_cancelButton_clicked();
    void onFreqEditFinish();

    void onLocEditFinish();
private:
    Ui::RigMemDialog *ui;
    memoryData::memData *logData;
    int memoryNumber;


};

#endif // RIGMEMDIALOG_H
