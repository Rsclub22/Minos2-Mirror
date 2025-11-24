/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Qrz Server
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2021
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////



#ifndef QRZCONFIGUREDIALOG_H
#define QRZCONFIGUREDIALOG_H

#include <QDialog>
#include "cutils.h"

namespace Ui {
class QrzConfigureDialog;
}

class QrzConfigureDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QrzConfigureDialog(QWidget *parent = nullptr);
    ~QrzConfigureDialog();

    QString logCallsign;
    QString logPassword;
    int cacheAge = 0;

    bool resetDB = false;

public Q_SLOTS:
        virtual int exec() override;

private slots:

    void on_resetDBButton_clicked();

    void on_OKButton_clicked();

    void on_cancelButton_clicked();

private:
    Ui::QrzConfigureDialog *ui;
    UpperCaseValidator ucValidator;
    void saveSettings();
};

#endif // QRZCONFIGUREDIALOG_H
