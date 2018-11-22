/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018
//
//
//
/////////////////////////////////////////////////////////////////////////////

#ifndef ROTPRESETDIALOG_H
#define ROTPRESETDIALOG_H

#include <QDialog>
#include "rotatorcommon.h"

namespace Ui {
class RotPresetDialog;
}

class RotPresetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RotPresetDialog(QWidget *parent, int buttonNumber, RotPresetData* _editData, RotPresetData* _curData, QString name);
    virtual ~RotPresetDialog() override;

    private slots:
        bool bearingEditFinished();
        void editAccepted();
        void editRejected();

private:
    Ui::RotPresetDialog *ui;
    RotPresetData* editData;
    RotPresetData* curData;
    void copyPresetData(RotPresetData *src, RotPresetData *dest);
};

#endif // ROTPRESETDIALOG_H
