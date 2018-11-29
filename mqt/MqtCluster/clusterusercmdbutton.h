/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Cluster Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018
//
//
//
/////////////////////////////////////////////////////////////////////////////


#ifndef CLUSTERUSERCMDBUTTON_H
#define CLUSTERUSERCMDBUTTON_H



#include <QShortcut>
#include <QToolButton>
#include <QMenu>





class ClusterUserCmdButton : public QObject
{
    Q_OBJECT

public:
    explicit ClusterUserCmdButton(QToolButton *b, int num, QShortcut* key, QShortcut* shiftkey);

    virtual ~ClusterUserCmdButton() override;

    QToolButton* userCmdButton;

    void showButtonMenu();
    void setText(QString t);

    QString getText();
signals:

    void userCmdShortCutRecall();
    void userCmdShiftShortCutRecall();
    void userCmdSendAction();
    void userCmdEditAction();
    void userCmdNewAction();
    void userCmdClearAction();


private:


    QMenu* presetMenu;
    QShortcut* shortKey;
    QShortcut* shiftShortKey;
    QAction* sendAction;
    QAction* newAction;
    QAction* editAction;
    QAction* clearAction;


    int userCmdNum;

private slots:
    //void presetUpdate();

    void shortCutSelected();
    void sendActionSelected();
    void editActionSelected();
    void writeActionSelected();
    void clearActionSelected();






    void recallShortCutSelected();
    void recallShiftShortCutSelected();
};





#endif // CLUSTERUSERCMDBUTTON_H
