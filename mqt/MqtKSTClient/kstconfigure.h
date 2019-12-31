#ifndef KSTCONFIGURE_H
#define KSTCONFIGURE_H

#include <QDialog>
#include <QIntValidator>

namespace Ui {
class KSTConfigure;
}

class KSTConfigure : public QDialog
{
    Q_OBJECT

public:
    explicit KSTConfigure(QWidget *parent = nullptr);
    ~KSTConfigure() override;

    QString hostname;
    QString port;
    QString username;
    QString password;
    bool autoConnect;
    QString locator;

public Q_SLOTS:
        virtual int exec() override;

private slots:


    void on_cancelButton_clicked();

    void on_OKButton_clicked();

private:
    Ui::KSTConfigure *ui;
};

#endif // KSTCONFIGURE_H
