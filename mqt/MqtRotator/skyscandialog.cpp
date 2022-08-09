/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016
//
//
// Hamlib Library
//
/////////////////////////////////////////////////////////////////////////////

#include "skyscandialog.h"
#include "ui_skyscandialog.h"

SkyScanDialog::SkyScanDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SkyScanDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

SkyScanDialog::~SkyScanDialog()
{
    delete ui;
}
