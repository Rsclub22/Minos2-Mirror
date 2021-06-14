#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include <QTimer>

namespace Ui {
class OptionsDialog;
}

class ClusterBandmapConfigure;
class DefDirsDlg;
class DisplayOptions;
class RadioSettingDialog;
class N1MMBroadcastConfig;
class WsjtxConfigure;

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(QWidget *parent = nullptr);
    virtual ~OptionsDialog() override;

private slots:
    void on_OKButton_clicked();

    void on_cancelButton_clicked();

    void doCloseEvent();

    void onMainRaised();
    void on_optionTabs_currentChanged(int index);

public Q_SLOTS:
    virtual int exec() override;
    virtual void accept() override;
    virtual void reject() override;

private:
    Ui::OptionsDialog *ui;

    ClusterBandmapConfigure *cbc = nullptr;
    DefDirsDlg *ddd = nullptr;
    DisplayOptions *dod = nullptr;
    RadioSettingDialog *rdc = nullptr;
    N1MMBroadcastConfig *nbc= nullptr;
    WsjtxConfigure *wc = nullptr;

    bool check();

};

#endif // OPTIONSDIALOG_H
