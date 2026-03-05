#ifndef KSTSENDMEEPFRAME_H
#define KSTSENDMEEPFRAME_H

#include "callsign.h"
#include <QFrame>

class QPushButton;
namespace Ui {
class KSTSendMeepFrame;
}

class KSTSendMeepFrame : public QFrame
{
    Q_OBJECT
    virtual bool eventFilter(QObject *obj, QEvent *event) override;

public:
    explicit KSTSendMeepFrame(QWidget *parent = nullptr);
    ~KSTSendMeepFrame();

    void on_FontChanged();

    void setNameFromCall(const Callsign &call, int activeChat);

    void setDefaultButton(QPushButton *d);
private slots:
    void on_genmsgButton_clicked();
    void on_meepButton_clicked();
    void on_callEdit_textChanged(const QString &);
    void on_msgEdit_textChanged(const QString &);
public slots:
    void on_clearMessageButton_clicked();
private:
    Ui::KSTSendMeepFrame *ui;
};

#endif // KSTSENDMEEPFRAME_H
