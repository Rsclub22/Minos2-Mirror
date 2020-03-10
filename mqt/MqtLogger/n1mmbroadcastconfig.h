#ifndef N1MMBROADCASTCONFIG_H
#define N1MMBROADCASTCONFIG_H

#include <QDialog>

namespace Ui {
class N1MMBroadcastConfig;
}

class N1MMBroadcastConfig : public QDialog
{
    Q_OBJECT

public:
    explicit N1MMBroadcastConfig(QWidget *parent = nullptr);
    virtual ~N1MMBroadcastConfig() override;

private slots:
    void on_OKButton_clicked();

    void on_cancelButton_clicked();

private:
    Ui::N1MMBroadcastConfig *ui;
    void doCloseEvent();

public Q_SLOTS:
    virtual void accept() override;
    virtual void reject() override;

};

#endif // N1MMBROADCASTCONFIG_H
