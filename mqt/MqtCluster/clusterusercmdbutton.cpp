/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Cluster Server
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018
//
//
//
/////////////////////////////////////////////////////////////////////////////


#include "clusterusercmdbutton.h"



ClusterUserCmdButton::ClusterUserCmdButton(QToolButton *b, int num, QShortcut* key, QShortcut* shiftkey)
{
    userCmdNum = num;

    userCmdButton = b;
    presetMenu = new QMenu(userCmdButton);

    userCmdButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    userCmdButton->setPopupMode(QToolButton::MenuButtonPopup);
    userCmdButton->setFocusPolicy(Qt::NoFocus);
    userCmdButton->setText(QString("%1:").arg(QString::number(userCmdNum + 1)));

    shortKey = key;
    shiftShortKey = shiftkey;
    sendAction = new QAction(tr("&Send"), userCmdButton);
    //writeAction = new QAction(tr("&Write"), userCmdButton);
    newAction = new QAction(tr("&New"),userCmdButton);
    editAction = new QAction(tr("&Edit"), userCmdButton);
    clearAction = new QAction(tr("&Clear"),userCmdButton);
    presetMenu->addAction(sendAction);
    presetMenu->addAction(newAction);
    presetMenu->addAction(editAction);
    presetMenu->addAction(clearAction);
    userCmdButton->setMenu(presetMenu);

    connect(shortKey, SIGNAL(activated()), this, SLOT(recallShortCutSelected()));
    connect(shiftShortKey, SIGNAL(activated()), this, SLOT(recallShiftShortCutSelected()));
    connect(userCmdButton, SIGNAL(clicked(bool)), this, SLOT(sendActionSelected()));
    connect( sendAction, SIGNAL( triggered() ), this, SLOT(sendActionSelected()));
    connect( newAction, SIGNAL( triggered() ), this, SLOT(newActionSelected()));
    connect( editAction, SIGNAL( triggered() ), this, SLOT(editActionSelected()));
    connect( clearAction, SIGNAL( triggered() ), this, SLOT(clearActionSelected()));


}


ClusterUserCmdButton::~ClusterUserCmdButton()
{
//    delete memButton;
}


void ClusterUserCmdButton::recallShortCutSelected()
{
    emit userCmdShortCutRecall();
}

void ClusterUserCmdButton::recallShiftShortCutSelected()
{
   emit userCmdShiftShortCutRecall();
}


void ClusterUserCmdButton::showButtonMenu()
{
    shortCutSelected();
}

void ClusterUserCmdButton::shortCutSelected()
{
//    rigControlFrame->memoryShortCutSelected(memNo);
    userCmdButton->showMenu();
    //emit lostFocus();
}
void ClusterUserCmdButton::sendActionSelected()
{
    emit userCmdSendAction();
}
void ClusterUserCmdButton::editActionSelected()
{
    emit userCmdEditAction();
}
void ClusterUserCmdButton::writeActionSelected()
{
    emit userCmdNewAction();
}
void ClusterUserCmdButton::clearActionSelected()
{
    emit userCmdClearAction();
}




void ClusterUserCmdButton::setText(QString t)
{
    userCmdButton->setText(t);
}

QString ClusterUserCmdButton::getText()
{
    return userCmdButton->text();
}
