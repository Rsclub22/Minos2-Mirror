#ifndef QSOMAPFRAME_H
#define QSOMAPFRAME_H

#include "clustercommon.h"
#include "contacts.h"
#include <QFrame>
#include <QVariant>

class BaseContestLog;
class QTimer;
class ClusterSpotData;

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

    // cluster spots
    QVector<QSharedPointer<ClusterSpotData> > spotQueue;
    bool clusterServerConnected = false;


    void startMap();
    void stopMap();
    void doRedraw(BaseContestLog *c, bool grid, bool lines);
    void drawSpot(QSharedPointer<ClusterSpotData>);

signals:
    void callSig(QVariant stringList);
    void spotSig(QVariant stringList);
    void homeSig(QVariant stringList);

    void drawLines(QVariant dl);
    void drawGrid(QVariant dg);
    void clearAll();

private slots:
    void onQmlClicked(QVariant v);
    void dxSpots(QVector<ClusterMessage> spotMsg);
public slots:
    void on_AfterLogContact(const BaseContestLog *c, const QSharedPointer<BaseContact> lct);
    void on_redrawQSOMap(bool grid, bool lines);

};

#endif // QSOMAPFRAME_H
