#ifndef KSTCONFIGURE_H
#define KSTCONFIGURE_H

#include <QDialog>
#include <QIntValidator>
#include "cutils.h"
#include "airscoutlink.h"

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
    UpperCaseValidator userValidator;
    QString firstName;

    QString password;
    bool autoConnect;
    QString locator;
    UpperCaseValidator locValidator;

    int maxDistance;

    bool ASActive;
    ASBand ASActiveBand;
    QString ASServerName;
    QString ASMyName;
    int ASMinDistance;
    int ASMaxDistance;
    int ASPort;
    int ASTimeout;

public Q_SLOTS:
        virtual int exec() override;

private slots:


    void on_cancelButton_clicked();

    void on_OKButton_clicked();

private:
    Ui::KSTConfigure *ui;
};

#endif // KSTCONFIGURE_H
