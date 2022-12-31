#include "spotbasedata.h"
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

}

QSOMapFrame::~QSOMapFrame()
{
    delete ui;
}
void QSOMapFrame::setContest(BaseContestLog *c, bool grid, bool lines)
{
    // NB maps that aren't displayed never get ct set

    ct = c;
    if (c)
    {
        startMap();

        doRedraw(c, grid, lines);
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
void QSOMapFrame::doRedraw(BaseContestLog *ctest, bool grid, bool lines)
{
    if (ct == nullptr || ctest != ct)
    {
        return;
    }
    emit drawGrid(grid);
    emit drawLines(lines);

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

        if ( cct->notValidContact() )
           continue;

        if (cct->cs.getValRes() != CS_OK)    // duplicate?
        {
            continue;
        }


        on_AfterLogContact(ct, cct);
    }
}

void QSOMapFrame::on_AfterLogContact(const BaseContestLog *c, const QSharedPointer<BaseContact> lct)
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

        double lat = raddeg(lct->lat);
        double lon = raddeg(lct->lon);

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

        callInfo << lct->cs.getFullCall();
        callInfo << QString::number(lat);
        callInfo << QString::number(lon);
        callInfo << loc;
        emit callSig(callInfo);
    }
}

void QSOMapFrame::on_redrawQSOMap(bool grid, bool lines)
{
    if (ct != nullptr)
    {
        // clear map, redraw home and all QSOs

        emit clearAll();

        doRedraw(ct, grid, lines);
    }
}

//---------------------- Cluster Spots -------------------------------------

void QSOMapFrame::drawSpot(QSharedPointer<BandmapSpotData> bsd)
{
    if (ct != nullptr)
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
            if ((msg.getLoggerUuid().isEmpty() || msg.getLoggerUuid() == ct->uuid) && (msg.getFrameId() == resendFrameId::BANDMAP_CLIENT || msg.getFrameId() == resendFrameId::ALL_CLIENTS))
            {
                if (msg.getMessage().contains(DXSPOT) || msg.getMessage().contains(RESENTSPOT))
                {
                    qlonglong timeToLive = 0;
                    trace(QString("Spot for this loggeruuid = %1, add to queue").arg(ct->uuid));
                    QSharedPointer<BandmapSpotData> sp = stringToDxSpot(msg.getMessage(), ct, timeToLive);
                    if (sp)
                    {
                        spotQueue += sp;
                        drawSpot(sp);
                    }
                }
            }
        }
    }
}

