#ifndef SCREENCONFIG_H
#define SCREENCONFIG_H

#include <QDialog>
#include <QVBoxLayout>

#include "ScreenConfigFile.h"

class ScreenConfigRow;
class ScreenConfigElement;
namespace Ui {
class ScreenConfig;
}

class ScreenConfig : public QDialog
{
    Q_OBJECT

public:
    explicit ScreenConfig(QWidget *parent = nullptr);
    ~ScreenConfig();

    void addBefore(ScreenConfigRow *r);
    void remove(ScreenConfigRow *r);
    void addAfter(ScreenConfigRow *r);

    bool checkOk(ScreenConfigElement *e);

public slots:
    void reject();
    void accept();
private slots:
    void on_OKButton_clicked();

    void on_applyButton_clicked();

    void on_cancelButton_clicked();

    void on_addRowButton_clicked();

private:
    Ui::ScreenConfig *ui;
    QVBoxLayout *vbl;
    ScreenConfigFile scf;
    QString curConfigName;

    void doCloseEvent();
    SC getConfig();
};

#endif // SCREENCONFIG_H
