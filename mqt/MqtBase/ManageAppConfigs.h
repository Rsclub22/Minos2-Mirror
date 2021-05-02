#ifndef MANAGEAPPCONFIGS_H
#define MANAGEAPPCONFIGS_H

#include <QDialog>

namespace Ui {
class ManageAppConfigs;
}
class QListWidgetItem;

class ManageAppConfigs : public QDialog
{
    Q_OBJECT

public:
    explicit ManageAppConfigs(QWidget *parent, bool showAutoStart);
    virtual ~ManageAppConfigs() override;

    int exec() override;

private slots:
    void on_cancelButton_clicked();

    void on_OKButton_clicked();
    void on_editButton_clicked();

    void on_newButton_clicked();

    void on_cloneButton_clicked();

    void on_deleteButton_clicked();

    void on_renameButton_clicked();

    void on_layoutList_itemSelectionChanged();
    void on_layoutList_itemDoubleClicked(QListWidgetItem *);
public Q_SLOTS:
   virtual void accept() override;
   virtual void reject() override;

private:
    Ui::ManageAppConfigs *ui;

    bool showAutoStart = false;
    bool suppressItemSelect = false;
    QString curConfigName;

    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;

    void showDetails();
    bool getNewName(QString &Value);
};

#endif // MANAGEAPPCONFIGS_H
