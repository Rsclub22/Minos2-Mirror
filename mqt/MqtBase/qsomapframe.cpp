#include <QVBoxLayout>
#include <QTimer>
#include <math.h>

#ifdef Q_OS_WIN
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) || QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QQuickItem>
#include <QQmlContext>

extern QSharedPointer<QQmlApplicationEngine> appQmlEngine;

#endif
#endif

#include <QMouseEvent>

#include "MTrace.h"
#include "calcs.h"
#include "contacts.h"
#include "latlong.h"
#include "contest.h"
#include "MinosLoggerEvents.h"
#include "MinosParameters.h"
#include "clusterClientServer.h"
#include "spotbasedata.h"
#include "qmlcpplink.h"
#include "qsomapframe.h"
#include "ui_qsomapframe.h"

QSOMapFrame::QSOMapFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::QSOMapFrame)
{
#ifdef Q_OS_WIN
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) || QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    // This makes the resources accessible from the main executable
    Q_INIT_RESOURCE(qml);
#endif
#endif

    ui->setupUi(this);

    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::AfterLogContactToBandmap, this, &QSOMapFrame::on_AfterLogContact, Qt::UniqueConnection);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::redrawQSOMap, this, &QSOMapFrame::on_redrawQSOMap, Qt::QueuedConnection);

    connect (ClusterClientServer::getClusterClientServer(), &ClusterClientServer::dxSpot, this, &QSOMapFrame::dxSpots);

    purgeTimer = new QTimer(this);
    connect (purgeTimer, &QTimer::timeout, this, &QSOMapFrame::purgeSpots);
    purgeTimer->start(PURGE_TIME);
}

QSOMapFrame::~QSOMapFrame()
{
    delete ui;
}
void QSOMapFrame::setContest(BaseContestLog *c, bool grid, bool lines, bool spots, int spotDistance)
{
    // NB maps that aren't displayed never get ct set

    ct = c;
    if (c)
    {
        startMap();

        doRedraw(c, grid, lines, spots, spotDistance);
    }
    else
    {
        stopMap();
    }
}
void QSOMapFrame::startMap()
{
#ifdef Q_OS_WIN
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)  || QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    // make a reference to the QML window available to C++

    QLayout *qvb = layout();

    if (!qvb)
    {
        qvb = new QVBoxLayout(this);

        if (!appQmlEngine)
        {

    // We must have a QCoreApplication before we do this
    // And we do this now so that the QML debugger can connect
        appQmlEngine = QSharedPointer<QQmlApplicationEngine>(new QQmlApplicationEngine());

        }
        //QQmlApplicationEngine* engine = new QQmlApplicationEngine(qvb);

        QmlCppLink* linkObject = new QmlCppLink(ct);
        QVariant vl;
        vl.setValue(linkObject);

        appQmlEngine->rootContext()->setContextProperty("QmlCppLink", vl);


        QQuickView *view = new QQuickView(appQmlEngine.data(), nullptr);
        view->setSource(QUrl("qrc:/qsoview.qml"));

        QWidget *container = QWidget::createWindowContainer(view);
        auto qmlObj = view->rootObject();

        qvb->addWidget(container);

        // connect the C++ callSig signal to the QML slot

        connect(this, SIGNAL(callSig(QVariant)), qmlObj, SLOT(newCall(QVariant)), Qt::UniqueConnection);
        connect(this, SIGNAL(spotSig(QVariant)), qmlObj, SLOT(newSpot(QVariant)), Qt::UniqueConnection);
        connect(this, SIGNAL(homeSig(QVariant)), qmlObj, SLOT(newHome(QVariant)), Qt::UniqueConnection);

        connect(this, SIGNAL(drawLines(QVariant)), qmlObj, SLOT(setDrawLines(QVariant)), Qt::UniqueConnection);
        connect(this, SIGNAL(drawGrid(QVariant)), qmlObj, SLOT(setDrawGrid(QVariant)), Qt::UniqueConnection);
        connect(this, SIGNAL(clearAll()), qmlObj, SLOT(clearAll()), Qt::UniqueConnection);

        connect(qmlObj, SIGNAL(qmlSignal(QVariant)), this, SLOT(onQmlClicked(QVariant)), Qt::UniqueConnection);
    }
#endif
#endif
}
void QSOMapFrame::stopMap()
{
    QLayout *lo = layout();
    delete lo;
}
void QSOMapFrame::onQmlClicked(QVariant /*v*/)
{
//   QList<QVariant> gc = v.toList();
//   QString latitude = gc[0].toString();
//   QString longitude = gc[1].toString();
//   QString bearing = gc[2].toString();

#ifdef Q_OS_WIN
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) || QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
//   qDebug() << latitude << " " << longitude << " " << bearing;
#endif
#endif
}
void QSOMapFrame::doRedraw(const BaseContestLog *ctest, bool grid, bool lines, bool spots, int sd)
{
    trace(QString("doRedraw grid %1 lines %2 spots %3 sd %4").arg(grid).arg(lines).arg(spots).arg(sd));
    if (ct == nullptr || ctest != ct)
    {
        return;
    }
    bdrawGrid = grid;
    bdrawLines = lines;

    emit drawGrid(grid);
    emit drawLines(lines);

    drawSpots = spots;
    spotDistance = sd;

    QStringList callInfo; // [callsign, latitude, longitude]

    callInfo << ct->mycall.getFullCall();
    callInfo << QString::number(raddeg(ct->odna));
    callInfo << QString::number(raddeg(ct->odea));
    callInfo << ct->myloc.getLoc();
    emit homeSig(callInfo);

    locs.clear();

    for ( auto const &c: qAsConst(ct->ctList ))
    {
        QSharedPointer<BaseContact> cct = c.wt;

        trace(QString("doRedraw %1 %2 %3").arg(cct->cs.getFullCall()).arg(cct->cs.getValRes()).arg(cct->loc.getLoc()));

        if ( cct->notValidContact() )
        {
            trace("Not valid");
           continue;
        }

        if (cct->cs.getValRes() != CS_OK)    // duplicate?
        {
            continue;
        }

        showContact(ct, cct);
    }

    for( auto const &s: qAsConst(spotQueue))
    {
        drawSpot(s);
    }
}
void QSOMapFrame::showContact(const BaseContestLog *c, const QSharedPointer<BaseContact> lct)
{
    if (ct != nullptr && ct == c && !ct->isReadOnly())
    {
        if ( lct->notValidContact() )
            return;

        if (lct->cs.getValRes() != CS_OK)    // duplicate?
        {
            return;
        }

        QString loc = lct->loc.getLoc();
        if (loc.isEmpty())
        {
            return;
        }
        trace(QString("showContact %1 %2 %3").arg(lct->cs.getFullCall()).arg(lct->cs.getValRes()).arg(lct->loc.getLoc()));

        double slat;
        double slon;
        /*char v =*/ lonlat( loc, slon, slat, MinosParameters::getMinosParameters() ->getAllowLoc4());

        double lat = raddeg(slat);
        double lon = raddeg(slon);

        bool fourLoc = (loc.length() == 4);

        int n = 0;
        if (locs.contains(loc))
        {
            n = locs[loc];
            locs[loc] = ++n;

        }
        else
        {
            n = 1;
            locs[loc] = n;
        }
        if (fourLoc)
        {
            if (lat < 0)
            {
                lat = lat - 1;
            }
            lat = std::round(lat) + 0.75;
            if (lon < 0)
            {
                lon = lon -1;
            }
            lon = std::round(lon);
            int ilon = lon;
            lon = lon - ilon%2 + 0.25  + 0.1 * n;

        }

        if (lct->cs.getFullCall() == "PA5Y")
        {
            int a = 0;
            a++;
        }
        QStringList callInfo; // [callsign, latitude, longitude]

        callInfo << lct->cs.getFullCall();
        callInfo << QString::number(lat);
        callInfo << QString::number(lon);
        callInfo << loc;
        emit callSig(callInfo);

        for( auto const &s: qAsConst(spotQueue))
        {
            if (s->getDxCall() == lct->cs)
            {
                s->setSpotType(bandmapSpotType::DELETED);
            }
        }

    }

}
void QSOMapFrame::on_AfterLogContact(const BaseContestLog *c, const QSharedPointer<BaseContact> lct)
{
    if (ct != nullptr && ct == c && !ct->isReadOnly())
    {
        if (!lct || lct->getHistory().size() > 1)
        {
            emit clearAll();
            locs.clear();
            doRedraw(c, bdrawGrid, bdrawLines, drawSpots, spotDistance);
        }
        else
        {
            showContact(c, lct);
        }
    }
}

void QSOMapFrame::on_redrawQSOMap(bool grid, bool lines, bool spots, int sd)
{
    if (ct != nullptr)
    {
        // clear map, redraw home and all QSOs

        emit clearAll();
        locs.clear();

        doRedraw(ct, grid, lines, spots, sd);
    }
}

//---------------------- Cluster Spots -------------------------------------

void QSOMapFrame::drawSpot(QSharedPointer<ClusterSpotData> bsd)
{
    if (drawSpots && ct != nullptr)
    {
        if (bsd->getDxCall().getValRes() != CS_OK)    // duplicate?
        {
            return;
        }

        QString loc = bsd->getDxLocator();
        if (loc.isEmpty())
        {
            return;
        }

        int bearing = 0;
        double distance = 0.0;
        ct->calcDistanceBearing(loc, &distance, &bearing);

        if (distance > spotDistance)
        {
            return;
        }

        double slat;
        double slon;
        /*char v =*/ lonlat( loc, slon, slat, MinosParameters::getMinosParameters() ->getAllowLoc4());

        double lat = raddeg(slat);
        double lon = raddeg(slon);

        bool fourLoc = (loc.length() == 4);

        int n = 0;
        if (locs.contains(loc))
        {
            n = locs[loc];
            locs[loc] = ++n;

        }
        else
        {
            n = 1;
            locs[loc] = n;
        }
        if (fourLoc)
        {
            if (lat < 0)
            {
                lat = lat - 1;
            }
            lat = std::round(lat) + 0.75;
            if (lon < 0)
            {
                lon = lon -1;
            }
            lon = std::round(lon);
            int ilon = lon;
            lon = lon - ilon%2 + 0.25  + 0.1 * n;

        }

        QStringList callInfo; // [callsign, latitude, longitude]

        callInfo << bsd->getDxCall().getFullCall();
        callInfo << QString::number(lat);
        callInfo << QString::number(lon);
        callInfo << loc;
        emit spotSig(callInfo);
    }
}
void QSOMapFrame::dxSpots(QVector<ClusterMessage> spotMsg)
{
    // if contest is protected, or no map showing, ignore
    if (ct && !ct->isReadOnly())
    {
        //get spot Message from queue
        for (int i = 0; i < spotMsg.count(); i++)
        {
            ClusterMessage msg = spotMsg[i];
            trace(QString("retrieve cluster spot from queue - spot = %1 for loggeruuid = %2, this contest uuid = %3").arg(msg.getMessage(), msg.getLoggerUuid(), ct->uuid));

            // if loggerUuid is empty, message is for all frames
            if ((msg.getLoggerUuid().isEmpty() || msg.getLoggerUuid() == ct->uuid)
                    && (msg.getFrameId() == resendFrameId::BANDMAP_CLIENT
                        || msg.getFrameId() == resendFrameId::ALL_CLIENTS))
            {
                if (msg.getMessage().contains(DXSPOT) || msg.getMessage().contains(RESENTSPOT))
                {
                    trace(QString("Spot for this loggeruuid = %1, add to queue").arg(ct->uuid));
                    QSharedPointer<ClusterSpotData> sp = stringToDxSpot(msg.getMessage(), ct, timeToLive);
                    if (sp)
                    {
                        bool fromNode = sp->getDxLocatorIsFromNode();
                        bool wkd = sp->getDxCallWorked();
                        bool locEmpty = sp->getDxLocator().isEmpty();
                        if (!fromNode && !wkd && !locEmpty)
                        {
                            trace(QString("draw cluster spot spot = %1").arg(msg.getMessage()));
                            spotQueue += sp;
                            drawSpot(sp);
                        }
                        else
                        {
                            trace(QString("don't draw cluster spot spot = %1 FN %2 WKD %3 LE %4")
                                  .arg(msg.getMessage())
                                  .arg(fromNode)
                                  .arg(wkd)
                                  .arg(locEmpty)
                                  );
                        }
                    }
                }
            }
            else
            {
                trace("Not for this contest");
            }
        }
    }
}
void QSOMapFrame::purgeSpots()
{
    if (timeToLive > 0)
    {
        if (spotQueue.count() > 0)
        {
           int idx = spotQueue.count() - 1;
           while (idx >= 0 && spotQueue.count() > 0)
           {
               if (spotTimedOut(spotQueue[idx]->getRxTime(), timeToLive)
                       || spotQueue[idx]->getSpotType() == bandmapSpotType::DELETED)
               {
                   trace(QString("purged spot = %1, count %2").arg(spotQueue[idx]->getDxCall().getFullCall()).arg(spotQueue.count()));

                   spotQueue.remove(idx);
               }
               idx--;
           }
        }
    }
}

