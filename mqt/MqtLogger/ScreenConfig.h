#ifndef SCREENCONFIG_H
#define SCREENCONFIG_H

#include <QDialog>
#include <QVBoxLayout>

#include "ScreenConfigFile.h"

class ScreenConfigScreen;
class ScreenConfigRow;
class ScreenConfigElement;
class ScreenConfigAddColumn;

namespace Ui {
class ScreenConfig;
}

class ScreenConfig : public QDialog
{
    Q_OBJECT

public:
    explicit ScreenConfig(QWidget *parent, ScreenConfigFile &scf, QString curConfigName);
    ~ScreenConfig() override;

    QVector<ScreenConfigScreen *> screens;

    ScreenConfigScreen *curScreen = nullptr;
    bool checkOk(ScreenConfigElement *s);
public slots:
    void reject() override;
    void accept() override;
private slots:
    void on_OKButton_clicked();

    void on_applyButton_clicked();

    void on_cancelButton_clicked();

    void on_addScreenButton_clicked();

    void on_screenTabs_currentChanged(int index);

private:
    Ui::ScreenConfig *ui;

    ScreenConfigFile &scf;
    QString curConfigName;

    void doCloseEvent();
    SC getConfig();
    ScreenConfigScreen *buildScreens(SC &sc);
    ScreenConfigScreen *buildScreen(SCScreen &s);

};

extern ScreenConfig *screenConfigDialog;
#endif // SCREENCONFIG_H
