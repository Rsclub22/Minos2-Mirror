#ifndef RSCONFIGURE_H
#define RSCONFIGURE_H

#include <QDialog>

namespace Ui {
class RSConfigure;
}

class RSConfigure : public QDialog
{
    Q_OBJECT

public:
    int wsjtxPort = 2237;
    QString wsjtxAddress;

    int n1mmPort = 12060;

    explicit RSConfigure(QWidget *parent = nullptr);
    ~RSConfigure();

    void setServerList(QStringList rigServers, QString mainCurrent, QString subCurrent);
    QString getSubServer();
    QString getMainServer();
    void doCloseEvent();

 public Q_SLOTS:
    virtual void accept() override;
    virtual void reject() override;

private slots:
    void on_OKButton_clicked();

    void on_canceButton_clicked();

private:
    Ui::RSConfigure *ui;
};

#endif // RSCONFIGURE_H
