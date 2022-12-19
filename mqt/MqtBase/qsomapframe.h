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

    void setContest(BaseContestLog *, bool grid, bool lines);

private:
    Ui::QSOMapFrame *ui;

    BaseContestLog *ct = nullptr;

    QMap <QString, int> locs;

    void startMap();
    void stopMap();
    void doRedraw(BaseContestLog *c, bool grid, bool lines);

signals:
    void callSig(QVariant stringList);
    void homeSig(QVariant stringList);

    void drawLines(QVariant dl);
    void drawGrid(QVariant dg);
    void clearAll();

private slots:
    void onQmlClicked(QVariant v);
public slots:
    void on_AfterLogContact(const BaseContestLog *c, const QSharedPointer<BaseContact> lct);
    void on_redrawQSOMap(bool grid, bool lines);

};

#endif // QSOMAPFRAME_H
