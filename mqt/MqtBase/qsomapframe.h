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

    void setContest(BaseContestLog *, bool monitor, bool grid, bool lines
                    , bool spots, int spotDistance
                    , bool sl, QString tl, QString br
                    , bool sn);

private:
    Ui::QSOMapFrame *ui;

    BaseContestLog *ct = nullptr;

    QMap <QString, int> locs;

    bool bmonitor = false;
    bool bdrawGrid = true;
    bool bdrawLines = true;
    bool drawSpots = true;
    int spotDistance = 0;
    bool showLoc = true;
    QString locTL;
    QString locBR;
    bool showNav = true;

    QString mCentreLat;
    QString mCentreLon;
    QString mZoom;

    // cluster spots
    QVector<QSharedPointer<ClusterSpotData> > spotQueue;
    bool clusterServerConnected = false;

    qlonglong timeToLive = 0;
    QTimer *purgeTimer = nullptr;

    void startMap();
    void stopMap();
    void doRedraw(const BaseContestLog *c, bool grid, bool lines, bool spots, int spotDistance
                  , bool sl, QString tl, QString br, bool sn);
    void drawSpot(QSharedPointer<ClusterSpotData>);

    void showContact(const BaseContestLog *c, const QSharedPointer<BaseContact> lct);
    bool checkSpotInTable(QSharedPointer<ClusterSpotData> spot);
    QPair<double, double> calcPosition(QString loc, bool &drawLine);
    QPair<double, double> calcLoc(QString loc);
signals:
    void callSig(QVariant stringList);
    void spotSig(QVariant stringList);
    void homeSig(QVariant stringList);

    void drawLines(QVariant dl);
    void drawGrid(QVariant dg);
    void showLocs(QVariant sl);
    void showLocsTL(QVariant tl);
    void showLocsBR(QVariant br);
    void showNavb(QVariant sn);
    void clearAll();

private slots:
    void onQmlSignal(QVariant v);
    void dxSpots(QVector<ClusterMessage> spotMsg);
    void purgeSpots();
    void saveParams();
    void onContestBandChanged(BaseContestLog *c);
public slots:
    void on_AfterLogContact(const BaseContestLog *c, const QSharedPointer<BaseContact> lct);
    void on_redrawQSOMap(bool grid, bool lines, bool spots, int sd, bool sl, QString tl, QString br, bool sn);

};

#endif // QSOMAPFRAME_H
