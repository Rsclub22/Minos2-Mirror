#ifndef DEFDIRSDLG_H
#define DEFDIRSDLG_H

#include <QDialog>

namespace Ui {
class DefDirsDlg;
}

class DefDirsDlg : public QDialog
{
    Q_OBJECT

public:
    explicit DefDirsDlg(QWidget *parent = nullptr);
    ~DefDirsDlg();

private slots:
    void on_OKButton_clicked();

    void on_cancelButton_clicked();

    void on_logsDirBrowse_clicked();

    void on_listsDirBrowse_clicked();

private:
    Ui::DefDirsDlg *ui;
    void doCloseEvent();

    QString browseDefDir(const QString &prompt);
    
public Q_SLOTS:
    virtual void accept() override;
    virtual void reject() override;
};

#endif // DEFDIRSDLG_H
