#ifndef MINOSCONTESTLOADDIALOG_H
#define MINOSCONTESTLOADDIALOG_H

#include <QDialog>
#include <QEventLoop>
#include <QTimer>

namespace Ui {
class MinosContestLoadDialog;
}

class MinosContestLoadDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MinosContestLoadDialog(QWidget *parent = nullptr);
    ~MinosContestLoadDialog() override;

    void setLoadMessage(QString mess, bool newFile, bool list);
    void doShow();

private:
    Ui::MinosContestLoadDialog *ui;
    QEventLoop *el = nullptr;
    //QTimer *timer = nullptr;

protected:
    void showEvent(QShowEvent *) Q_DECL_OVERRIDE;

};

#endif // MINOSCONTESTLOADDIALOG_H
