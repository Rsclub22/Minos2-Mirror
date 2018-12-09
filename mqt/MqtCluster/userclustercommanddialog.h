#ifndef USERCLUSTERCOMMANDDIALOG_H
#define USERCLUSTERCOMMANDDIALOG_H

#include <QDialog>

namespace Ui {
class userClusterCommandDialog;
}

class ClusterUserCommandData
{

public:

    ClusterUserCommandData(QString _name, QString _cmdString);
    ClusterUserCommandData();

    QString name;
    QString cmdString;

};

class userClusterCommandDialog : public QDialog
{
    Q_OBJECT

public:
    explicit userClusterCommandDialog(QWidget *parent, int buttonNumber, ClusterUserCommandData* _editData, ClusterUserCommandData* _curData, QString name);
    ~userClusterCommandDialog();

private slots:
    void nameEditFinished();
    void cmdStringEditFinished();
    void editAccepted();
    void editRejected();

private:
    Ui::userClusterCommandDialog *ui;
    ClusterUserCommandData *editData;
    ClusterUserCommandData *curData;
    bool nameChanged;
    bool cmdStringChanged;
    void doCloseEvent();
};

#endif // USERCLUSTERCOMMANDDIALOG_H
