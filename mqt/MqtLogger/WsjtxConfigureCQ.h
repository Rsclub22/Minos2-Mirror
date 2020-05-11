#ifndef WSJTXCONFIGURECQ_H
#define WSJTXCONFIGURECQ_H

#include <QDialog>

namespace Ui {
class WsjtxConfigureCQ;
}

class WsjtxConfigureCQ : public QDialog
{
    Q_OBJECT

public:
    explicit WsjtxConfigureCQ(QWidget *parent = nullptr);
    ~WsjtxConfigureCQ();

private slots:
    void on_OKButton_clicked();

    void on_cancelButton_clicked();

private:
    Ui::WsjtxConfigureCQ *ui;
};

#endif // WSJTXCONFIGURECQ_H
