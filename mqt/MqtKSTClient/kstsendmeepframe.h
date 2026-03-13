#ifndef KSTSENDMEEPFRAME_H
#define KSTSENDMEEPFRAME_H

#include "callsign.h"
#include <QFrame>

class QPushButton;
namespace Ui {
class KSTSendMeepFrame;
}

enum Salutation {esNone, esHi, esHiName};
class KSTSendMeepFrame : public QFrame
{
    Q_OBJECT
    virtual bool eventFilter(QObject *obj, QEvent *event) override;
    Salutation sal = esHiName;
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
    void on_salNonerb_clicked();

    void on_salHirb_clicked();

    void on_salHiNamerb_clicked();

public slots:
    void on_clearMessageButton_clicked();
private:
    Ui::KSTSendMeepFrame *ui;
};

#endif // KSTSENDMEEPFRAME_H
