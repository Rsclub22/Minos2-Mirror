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
    explicit RSConfigure(QWidget *parent = nullptr);
    ~RSConfigure();

    void setServerList(QStringList rigServers, QString current);
    QString getSubServer();

private slots:
    void on_OKButton_clicked();

    void on_canceButton_clicked();

private:
    Ui::RSConfigure *ui;
};

#endif // RSCONFIGURE_H
