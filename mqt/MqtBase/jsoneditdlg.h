#ifndef JSONEDITDLG_H
#define JSONEDITDLG_H

#include <QDialog>

namespace Ui {
class JsonEditDlg;
}

class SettingsBundle;
class JsonEditDlg : public QDialog
{
    Q_OBJECT

    Ui::JsonEditDlg *ui;

    QJsonDocument *bundle;
    bool currSectionOnly;
    QString initialSection;

    QString baseTitle;
    QString name;
    void showSections(QString currSection);
    void showSection();
    void showDetails();
    void getDetails();
    void doCloseEvent();
public:
    explicit JsonEditDlg(QWidget *parent , QString name, QJsonDocument *bundle);
    void ShowCurrentSectionOnly();
    ~JsonEditDlg() override;

    int exec() override;

private slots:
    void on_NewSectionButton_clicked();

    void on_CopyButton_clicked();

    void on_DeleteButton_clicked();

    void on_CancelButton_clicked();

    void on_OKButton_clicked();

    void on_SectionsList_itemSelectionChanged();
    void on_renameButton_clicked();

    void on_splitter_splitterMoved(int pos, int index);

public Q_SLOTS:
    virtual void accept() override;
    virtual void reject() override;

};
#endif // JSONEDITDLG_H
