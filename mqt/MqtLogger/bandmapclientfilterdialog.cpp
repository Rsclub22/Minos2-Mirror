///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur RadRotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2019
//
//
//
/////////////////////////////////////////////////////////////////////////////



#include "bandmapclientfilterdialog.h"
#include "ui_bandmapclientfilterdialog.h"

BandmapClientFilterDialog::BandmapClientFilterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BandmapClientFilterDialog)
{
    ui->setupUi(this);
}

BandmapClientFilterDialog::~BandmapClientFilterDialog()
{
    delete ui;
}
