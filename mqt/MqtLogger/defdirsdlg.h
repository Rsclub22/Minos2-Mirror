#ifndef DEFDIRSDLG_H
#define DEFDIRSDLG_H

#include <QFrame>
#include "ConfigurationOption.h"

namespace Ui {
class DefDirsDlg;
}

class DefDirsDlg : public QFrame
{
    Q_OBJECT

public:
    explicit DefDirsDlg(QWidget *parent = nullptr);
    ~DefDirsDlg();

    void initialise();
    void finalise();

    bool check();
    void cancel();
private slots:
    void on_logsDirBrowse_clicked();

    void on_listsDirBrowse_clicked();

    void on_ageCb_stateChanged(int arg1);

private:
    Ui::DefDirsDlg *ui;

    ConfigurationOption ConfAgeProtectContests;
    ConfigurationOption ConfAge;
    ConfigurationOption ConfListDir;
    ConfigurationOption ConfLogDir;

    ConfigurationOption autoQRZCsHF;
    ConfigurationOption autoQRZCsVHF;
    ConfigurationOption autoQRZWSJTHF;
    ConfigurationOption autoQRZWSJTVHF;


    QString browseDefDir(const QString &prompt);
};

#endif // DEFDIRSDLG_H
