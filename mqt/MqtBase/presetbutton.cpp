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

#include "rotatorcommon.h"
#include "presetbutton.h"



PresetButton::PresetButton(QToolButton *b, int num, QShortcut* key, QShortcut* shiftkey, const QStringList buttonLabels)
{
    presetNum = num;

    presetButton = b;
    presetMenu = new QMenu(presetButton);

    presetButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    presetButton->setPopupMode(QToolButton::MenuButtonPopup);
    presetButton->setFocusPolicy(Qt::NoFocus);
    presetButton->setText(QString("%1:").arg(QString::number(presetNum + 1)));

    shortKey = key;
    shiftShortKey = shiftkey;
    readAction = new QAction(buttonLabels[0], presetButton);
    writeAction = new QAction(buttonLabels[1],presetButton);
    editAction = new QAction(buttonLabels[2], presetButton);
    clearAction = new QAction(buttonLabels[3],presetButton);
    presetMenu->addAction(readAction);
    presetMenu->addAction(writeAction);
    presetMenu->addAction(editAction);
    presetMenu->addAction(clearAction);
    presetButton->setMenu(presetMenu);

    connect(shortKey, &QShortcut::activated, this, &PresetButton::memoryRecallShortCutSelected);
    connect(shiftShortKey, &QShortcut::activated, this, &PresetButton::memoryShiftShortCutSelected);
    connect(presetButton, &QToolButton::clicked, this, &PresetButton::readActionSelected);
    connect( readAction, &QAction::triggered, this, &PresetButton::readActionSelected);
    connect( writeAction, &QAction::triggered, this, &PresetButton::writeActionSelected);
    connect( editAction, &QAction::triggered, this, &PresetButton::editActionSelected);
    connect( clearAction, &QAction::triggered, this, &PresetButton::clearActionSelected);


}


PresetButton::~PresetButton()
{
//    delete memButton;
}


void PresetButton::memoryRecallShortCutSelected()
{
    emit presetShortCutRecall();
}

void PresetButton::memoryShiftShortCutSelected()
{
   emit presetShiftShortCutRecall();
}


void PresetButton::showButtonMenu()
{
    presetShortCutSelected();
}

void PresetButton::presetShortCutSelected()
{
//    rigControlFrame->memoryShortCutSelected(memNo);
    presetButton->showMenu();
    //emit lostFocus();
}
void PresetButton::readActionSelected()
{
    emit presetReadAction();
}
void PresetButton::editActionSelected()
{
    emit presetEditAction();
}
void PresetButton::writeActionSelected()
{
    emit presetWriteAction();
}
void PresetButton::clearActionSelected()
{
    emit presetClearAction();
}




void PresetButton::setText(QString t)
{
    presetButton->setText(t);
}

QString PresetButton::getText()
{
    return presetButton->text();
}
