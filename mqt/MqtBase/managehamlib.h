#ifndef MANAGEHAMLIB_H
#define MANAGEHAMLIB_H

#include <QDialog>
#include <QSslError>

namespace Ui {
class ManageHamlib;
}

class ManageHamlib : public QDialog
{
    Q_OBJECT

public:
    explicit ManageHamlib(QWidget *parent = nullptr);
    ~ManageHamlib();
public slots:
    void reject() override;
    void accept() override;
private slots:
    void on_cancelButton_clicked();
    void on_updateHamlibButton_clicked();

    void on_revertHamlibButton_clicked();

    void ignoreSslErrors(const QList<QSslError> &errors);
private:
    Ui::ManageHamlib *ui;

    QString hamlibDLLPath;

    bool downloadFile(QString url, QString path, bool showError, QWidget *parent);
    bool checkHamlib();
    void doCloseEvent();
};

#endif // MANAGEHAMLIB_H
