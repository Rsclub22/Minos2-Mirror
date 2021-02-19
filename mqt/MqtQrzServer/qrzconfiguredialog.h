#ifndef QRZCONFIGUREDIALOG_H
#define QRZCONFIGUREDIALOG_H

#include <QDialog>

namespace Ui {
class QrzConfigureDialog;
}

class QrzConfigureDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QrzConfigureDialog(QWidget *parent = nullptr);
    ~QrzConfigureDialog();

    QString logCallsign;
    QString logPassword;

public Q_SLOTS:
        virtual int exec() override;

private slots:


    void onRejected();
    void onAccepted();


private:
    Ui::QrzConfigureDialog *ui;
    void saveSettings();
};

#endif // QRZCONFIGUREDIALOG_H
