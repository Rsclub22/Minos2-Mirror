///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur RadRotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2019
//
//
//
/////////////////////////////////////////////////////////////////////////////




#ifndef BANDMAPCLIENTFILTERDIALOG_H
#define BANDMAPCLIENTFILTERDIALOG_H

#include <QDialog>

namespace Ui {
class BandmapClientFilterDialog;
}

class BandmapClientFilterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BandmapClientFilterDialog(QWidget *parent = nullptr);
    ~BandmapClientFilterDialog();

private:
    Ui::BandmapClientFilterDialog *ui;
};

#endif // BANDMAPCLIENTFILTERDIALOG_H
