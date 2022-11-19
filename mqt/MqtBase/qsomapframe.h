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

    void on_AfterLogContact(const BaseContestLog *c, const QSharedPointer<BaseContact> lct);
private:
    Ui::QSOMapFrame *ui;

    BaseContestLog *ct = nullptr;

    void startMap();
    void stopMap();
signals:
    void callSig(QVariant stringList);
    void homeSig(QVariant stringList);

private slots:
    void onQmlClicked(QVariant v);
};

#endif // QSOMAPFRAME_H
