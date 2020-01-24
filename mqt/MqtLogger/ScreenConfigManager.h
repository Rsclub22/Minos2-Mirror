#ifndef SCREENCONFIGMANAGER_H
#define SCREENCONFIGMANAGER_H
#include "ScreenConfigFile.h"

#include <QDialog>

namespace Ui {
class ScreenConfigManager;
}

class QListWidgetItem;


class ScreenConfigManager : public QDialog
{
    Q_OBJECT

public:
    static const char * defLayoutText;
    explicit ScreenConfigManager(QWidget *parent = nullptr);
    ~ScreenConfigManager() override;

    int exec() override;

    static QString stripDefaultDecoration(QString s);

public slots:
    void reject() override;
    void accept() override;

private slots:
    void on_layoutList_itemSelectionChanged();

    void on_layoutList_itemDoubleClicked(QListWidgetItem *item);

    void on_newButton_clicked();

    void on_cloneButton_clicked();

    void on_deleteButton_clicked();

    void on_renameButton_clicked();

    void on_editButton_clicked();

    void on_OKButton_clicked();

    void on_applyButton_clicked();

    void on_cancelButton_clicked();

    void on_makeDefaultButton_clicked();

private:
    Ui::ScreenConfigManager *ui;
    bool suppressItemSelect = false;

    ScreenConfigFile scf;
    QString curConfigName;
    QString defaultConfigName;
    void showDetails();
    void checkEnabled();

    void doCloseEvent();
    bool getNewName(QString &Value);
};

#endif // SCREENCONFIGMANAGER_H
