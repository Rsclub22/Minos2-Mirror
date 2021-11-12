#ifndef PSTCONFIGDIALOG_H
#define PSTCONFIGDIALOG_H

#include <QDialog>

namespace Ui {
class PstConfigDialog;
}

class PstConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PstConfigDialog(QWidget *parent = nullptr);
    ~PstConfigDialog();

    QString getPstRotatorFilePathx86Text();
    QString getpstRotatorFilePathText();
    QString getPstRotatorAzFilePathx86Text();
    QString getPstRotatorAzFilePathText();

private:
    Ui::PstConfigDialog *ui;

    QString pstRotatorFilePathx86;
    QString pstRotatorFilePath;

    QString pstRotatorAzFilePathx86;
    QString pstRotatorAzFilePath;


    void loadDialog();
};

#endif // PSTCONFIGDIALOG_H
