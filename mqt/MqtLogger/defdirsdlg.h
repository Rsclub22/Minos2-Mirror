#ifndef DEFDIRSDLG_H
#define DEFDIRSDLG_H

#include <QFrame>
#include "OptionsDialog.h"

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

    int cap = 0;
    bool doAge = false;

    void doCloseEvent();

    QString browseDefDir(const QString &prompt);
};

#endif // DEFDIRSDLG_H
