#ifndef QSOMAPFRAME_H
#define QSOMAPFRAME_H

#include <QFrame>
#include <QVariant>

#include "clustercommon.h"
#include "contacts.h"

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

    void setContest(BaseContestLog *, bool grid, bool lines, bool spots, int spotDistance);

private:
    Ui::QSOMapFrame *ui;

    BaseContestLog *ct = nullptr;

    QMap <QString, int> locs;

    bool bdrawGrid = true;
    bool bdrawLines = true;
    bool drawSpots = true;
    int spotDistance = 0;

    // cluster spots
    QVector<QSharedPointer<ClusterSpotData> > spotQueue;
    bool clusterServerConnected = false;

    qlonglong timeToLive = 0;
    QTimer *purgeTimer = nullptr;

    void startMap();
    void stopMap();
    void doRedraw(const BaseContestLog *c, bool grid, bool lines, bool spots, int spotDistance);
    void drawSpot(QSharedPointer<ClusterSpotData>);

    void showContact(const BaseContestLog *c, const QSharedPointer<BaseContact> lct);
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
    void purgeSpots();
public slots:
    void on_AfterLogContact(const BaseContestLog *c, const QSharedPointer<BaseContact> lct);
    void on_redrawQSOMap(bool grid, bool lines, bool spots, int sd);

};

#endif // QSOMAPFRAME_H
