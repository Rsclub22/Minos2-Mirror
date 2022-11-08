#ifndef QSOMAPFRAME_H
#define QSOMAPFRAME_H

#include "contacts.h"
#include <QFrame>
#include <QVariant>

class BaseContestLog;
class QTimer;

namespace Ui {
class QSOMapFrame;
}

class QSOMapFrame : public QFrame
{
    Q_OBJECT

public:
    explicit QSOMapFrame(QWidget *parent = nullptr);
    ~QSOMapFrame();

    void setContest(BaseContestLog *);
    void closeContest();

    void on_AfterLogContact(BaseContestLog *c, QSharedPointer<BaseContact> lct);
private:
    Ui::QSOMapFrame *ui;

    BaseContestLog *ct = nullptr;

signals:
    void callSig(QVariant stringList);
    void homeSig(QVariant stringList);


private slots:
    void timeout();
};

#endif // QSOMAPFRAME_H
