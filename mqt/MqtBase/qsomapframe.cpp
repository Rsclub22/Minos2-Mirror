#include "cutils.h"
#ifdef INC_MAP
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QQuickItem>
#include <QQmlContext>
#include <QPushButton>

extern QSharedPointer<QQmlApplicationEngine> appQmlEngine;

#endif

#include <QMouseEvent>
#include <QVBoxLayout>
#include <QTimer>
#include <cmath>

#include <QLabel>
#include <QDesktopServices>
#include <QUrl>

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
#include "bandmapclientfilterdialog.h"
#include "ui_qsomapframe.h"

QSOMapFrame::QSOMapFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::QSOMapFrame)
{
#ifdef INC_MAP
    // This makes the resources accessible from the main executable
    Q_INIT_RESOURCE(qml);
#endif

    ui->setupUi(this);

    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::AfterLogContact, this, &QSOMapFrame::on_AfterLogContact, Qt::UniqueConnection);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::redrawQSOMap, this, &QSOMapFrame::on_redrawQSOMap, Qt::QueuedConnection);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::ContestBandChanged, this, &QSOMapFrame::onContestBandChanged);

    connect (ClusterClientServer::getClusterClientServer(), &ClusterClientServer::dxSpot, this, &QSOMapFrame::dxSpots);

    purgeTimer = new QTimer(this);
    connect (purgeTimer, &QTimer::timeout, this, &QSOMapFrame::purgeSpots);
    purgeTimer->start(PURGE_TIME);
    paramSaveTimer = new QTimer(this);
    connect (paramSaveTimer, &QTimer::timeout, this, &QSOMapFrame::saveParams);
    paramSaveTimer->start(1000);
}

QSOMapFrame::~QSOMapFrame()
{
    delete ui;
}
void QSOMapFrame::setContest(BaseContestLog *c, bool monitor, bool grid, bool lines, bool spots, bool sl, QString tl, QString br, bool sn)
{
    // NB maps that aren't displayed never get ct set

    ct = c;
    if (c)
    {

        trace(QString("Set Contest: contest uuid =  ContestUuid = %1").arg(ct->uuid));

        QString contestModeStr = ct->currentMode.getValue();


        if (!ct->getQSOMapFilterSettingsExist())       // have settings been saved before?
        {
            BandmapClientFilterSettings filterSettings;
            // no, save current mode filter for this contest
            if (ct->isHF())
            {
                filterSettings.setIgnoreDistanceFlag(true);
            }

            //set current mode
            if (contestModeStr == hamlibData::MGM)
            {
                for (auto &m: mgmModes)
                {
                    filterSettings.setModeFilter(m, true); // set all the mgm modes in filter

                }
            }
            else if (contestModeStr == hamlibData::PH)
            {
                filterSettings.setModeFilter(USB_MODE, true);
                filterSettings.setModeFilter(LSB_MODE, true);
            }
            else if (contestModeStr == hamlibData::RY || contestModeStr == hamlibData::PSK  )
            {
                filterSettings.setModeFilter(RTTY_MODE, true);
                filterSettings.setModeFilter(PSK31_MODE, true);
            }
            else
            {
                filterSettings.setModeFilter(contestModeStr, true);
            }

            ct->setQSOMapFilterSettingsExist(true);
            ct->saveQSOMapFilter(filterSettings);
        }
        else
        {
            //filterSettings = ct->getQSOMapFilter();
        }

        bmonitor = monitor;
        startMap();

        doRedraw(c, grid, lines, spots, sl, tl, br, sn);
    }
    else
    {
        stopMap();
    }
}
void QSOMapFrame::onContestBandChanged(BaseContestLog *c)
{
    if (c == ct)
    {
        emit clearAll();
        locs.clear();

        doRedraw(ct, bdrawGrid, bdrawLines, drawSpots, showLoc, locTL, locBR, showNav);
    }
}
void QSOMapFrame::startMap()
{
#ifdef INC_MAP
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

        QHBoxLayout *qhb = new QHBoxLayout();

        QPushButton *filterButton = new QPushButton(this);
        filterButton->setText(tr("Filter..."));
        qhb->addWidget(filterButton);
        connect(filterButton, &QPushButton::clicked, this, &QSOMapFrame::filterButtonClicked);

        QSpacerItem *horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        qhb->addItem(horizontalSpacer);


        QLabel *clab = new QLabel(this);
        clab->setAlignment(Qt::AlignCenter);
        clab->setText(QString("<b>") + tr("Data and Map") + QString(" &copy;<a href=\"https://openstreetmap.org/\">openstreetmap.org</a>"));
        qhb->addWidget(clab);
        clab->setTextInteractionFlags(Qt::TextBrowserInteraction);
        connect(clab, &QLabel::linkActivated, this, &QSOMapFrame::onclab_linkActivated);

        qvb->addItem(qhb);

        QVBoxLayout *vbl = dynamic_cast<QVBoxLayout *>(qvb);
        vbl->setStretch(0, 199);
        vbl->setStretch(1, 1);
        adjustMargins(vbl, 0, 0, 0, 0, 0);

        // connect the C++ callSig signal to the QML slot

        connect(this, SIGNAL(callSig(QVariant)), qmlObj, SLOT(newCall(QVariant)), Qt::UniqueConnection);
        connect(this, SIGNAL(spotSig(QVariant)), qmlObj, SLOT(newSpot(QVariant)), Qt::UniqueConnection);
        connect(this, SIGNAL(homeSig(QVariant)), qmlObj, SLOT(newHome(QVariant)), Qt::UniqueConnection);

        connect(this, SIGNAL(drawLines(QVariant)), qmlObj, SLOT(setDrawLines(QVariant)), Qt::UniqueConnection);
        connect(this, SIGNAL(drawGrid(QVariant)), qmlObj, SLOT(setDrawGrid(QVariant)), Qt::UniqueConnection);
        connect(this, SIGNAL(showLocs(QVariant)), qmlObj, SLOT(setShowLocs(QVariant)), Qt::UniqueConnection);
        connect(this, SIGNAL(showLocsTL(QVariant)), qmlObj, SLOT(setShowLocsTL(QVariant)), Qt::UniqueConnection);
        connect(this, SIGNAL(showLocsBR(QVariant)), qmlObj, SLOT(setShowLocsBR(QVariant)), Qt::UniqueConnection);
        connect(this, SIGNAL(showNavb(QVariant)), qmlObj, SLOT(setShowNav(QVariant)), Qt::UniqueConnection);

        connect(this, SIGNAL(clearAll()), qmlObj, SLOT(clearAll()), Qt::UniqueConnection);

        connect(qmlObj, SIGNAL(qmlSignal(QVariant)), this, SLOT(onQmlSignal(QVariant)), Qt::UniqueConnection);
    }
#endif
}
void QSOMapFrame::onclab_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(QUrl(link));
}
void QSOMapFrame::filterButtonClicked()
{
    if (ct)
    {
        trace("QSOMapFrame::filterButtonClicked()");
        BandmapClientFilterSettings filterSettings = ct->getQSOMapFilter();
        trace(filterSettings.print());
        BandmapClientFilterDialog fd(filterSettings, tr("QSO Map Spot Filters"), this);

        if (fd.exec() == QDialog::Accepted)
        {
            trace("QSOMapFrame::filterButtonClicked() Accepted");
            if (fd.getSettingsChangedFlag())
            {
                filterSettings = fd.getFilterSettings();
                ct->saveQSOMapFilter(filterSettings);
                doRedraw(ct, bdrawGrid, bdrawLines, drawSpots, showLoc, locTL, locBR, showNav);
            }
        }
    }
}
void QSOMapFrame::stopMap()
{
    QLayout *lo = layout();
    delete lo;
}
void QSOMapFrame::onQmlSignal(QVariant v)
{
    QList<QVariant> gc = v.toList();

    QString reason = gc[0].toString();
    if (reason == "LeftPressed")
    {
        //   QString latitude = gc[1].toString();
        //   QString longitude = gc[2].toString();
        //   qDebug() << latitude << " " << longitude << " " << bearing;

        QString bearing = QString::number(gc[3].toInt());
        MinosLoggerEvents::SendBrgStrToRot(bearing);
    }
    else if (reason == "ZoomChanged")
    {
        if (bmonitor)
        {
            mZoom = gc[1].toString();
        }
        else
        {
            ct->zoomLevel.setValue(gc[1].toString());
        }
        trace(QString("ZoomChanged qmlSignal %1").arg(ct->zoomLevel.getValue()));

    }
    else if (reason == "CentreChanged")
    {
        if (bmonitor)
        {
            mCentreLat = gc[1].toString();
            mCentreLon = gc[2].toString();
        }
        else
        {
            ct->centreLat.setValue( gc[1].toString());
            ct->centreLon.setValue(gc[2].toString());
        }
        trace(QString("CentreChanged qmlSignal %1 %2").arg(ct->centreLat.getValue(), ct->centreLon.getValue()));

    }
    else
    {
        trace(QString("Unknown qmlSignal %1").arg(reason));
    }
}
void QSOMapFrame::saveParams()
{
    if (!bmonitor && ct)
    {
        if (ct->zoomLevel.isDirty() || ct->centreLat.isDirty() || ct->centreLon.isDirty())
        {
            ct->commonSave(false);
        }
    }
}
void QSOMapFrame::doRedraw(const BaseContestLog *ctest, bool grid, bool lines, bool spots
                           ,bool sl, QString tl, QString br, bool sn)
{
    trace(QString("QSOMapFrame doRedraw grid %1 lines %2 spots %3").arg(grid).arg(lines).arg(spots));
    if (ct == nullptr || ctest != ct)
    {
        return;
    }

    if (tl.size() != 4)
    {
        return;
    }
    Locator tlloc;
    tlloc.setLoc(tl);
    int tllocres = tlloc.getValRes();
    if (tllocres != LOC_OK && tllocres != LOC_PARTIAL)
    {
        return;
    }
    if (br.size() != 4)
    {
        return;
    }
    Locator brloc;
    brloc.setLoc(br);

    int brlocres = brloc.getValRes();
    if (brlocres != LOC_OK && brlocres != LOC_PARTIAL)
    {
        return;
    }

    bdrawGrid = grid;
    bdrawLines = lines;

    emit drawGrid(bdrawGrid);
    emit drawLines(bdrawLines);

    showLoc = sl;
    emit showLocs(showLoc);

    showNav = sn;
    emit showNavb(showNav);

    locTL = tl;

    // 1st letter, number increases East
    // 2nd letter, number increases North

    // we are positioning loc from top left
    QPair<double, double> pos = calcLoc(locTL);

    QStringList latlong; // [latitude, longitude]

    int tllat = pos.first + 1 + 1;
    int tllon = pos.second + 1 - 2;

    latlong << QString::number(tllat);
    latlong << QString::number(tllon);

    emit showLocsTL(latlong);

    locBR = br;

    pos = calcLoc(locBR);

    latlong.clear();

    int brlat = pos.first + 1;
    int brlon = pos.second + 1;
    latlong << QString::number(brlat);
    latlong << QString::number(brlon);

    emit showLocsBR(latlong);

    drawSpots = spots;

    QStringList callInfo; // [callsign, latitude, longitude]

    //    var monitor = callInfo[0]

    //    var zoom = callInfo[1]
    //    var clat = callInfo[2]
    //    var clon = callInfo[3]

    //    mapOfEurope.center = QtPositioning.coordinate(clat, clon)
    //    mapOfEurope.zoomLevel = zoom

    //    var call = callInfo[4]

    //    homeLat = callInfo[5];
    //    homeLon = callInfo[6];
    //    var hcoord = QtPositioning.coordinate(homeLat, homeLon)
    //    var loc = callInfo[7]

    callInfo << QString(bmonitor?"Monitor":"Logger");
    if (bmonitor)
    {
        if (mZoom.isEmpty())
        {
            mZoom = "5";
        }
        callInfo << mZoom;

        if (mCentreLat.isEmpty())
        {
            mCentreLat = QString::number(raddeg(ct->odna));
            mCentreLon = QString::number(raddeg(ct->odea));
        }
        callInfo << mCentreLat;
        callInfo << mCentreLon;
    }
    else
    {
        if (ct->zoomLevel.getValue().isEmpty())
        {
            callInfo << "5";
        }
        else
        {
            callInfo << ct->zoomLevel.getValue();
        }
        if (ct->centreLat.getValue().isEmpty())
        {
            callInfo << QString::number(raddeg(ct->odna));
            callInfo << QString::number(raddeg(ct->odea));
        }
        else
        {
            callInfo << ct->centreLat.getValue();
            callInfo << ct->centreLon.getValue();
        }
    }
    callInfo << ct->mycall.getFullCall();
    callInfo << QString::number(raddeg(ct->odna));
    callInfo << QString::number(raddeg(ct->odea));
    callInfo << ct->myloc.getLoc();
    emit homeSig(callInfo);

    locs.clear();

    for ( auto const &c: QASCONST(ct->ctList ))
    {
        QSharedPointer<BaseContact> cct = c.wt;

        if ( cct->notValidContact() )
        {
            continue;
        }

        if (cct->cs.getValRes() != CS_OK)    // duplicate?
        {
            continue;
        }

        showContact(ct, cct);
    }

    for( auto const &s: QASCONST(spotQueue))
    {
        drawSpot(s);
    }
}
QPair<double, double> QSOMapFrame::calcPosition(QString loc, bool &drawLine)
{
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
        if (lon < 0)
        {
            lon = lon -1;
        }
        lon = std::round(lon);
        int ilon = lon;
        lon = lon - ilon%2 + 1; // offset from square edge

        if (lat < 0)
        {
            lat = lat - 1;
        }
        lat = std::round(lat) + 0.5;  // offset from square edge

        // now offset from the centre position

        if (n > 1)
        {
            n -= 1;
            QPair<int, int> offsets[] = {
                {0, 1},
                {1, 1},
                {1, 0},
                {1, -1},
                {0, -1},
                {-1, -1},
                {-1, 0},
                {-1, 1}
            };

            int offset = n%8;
            int mult = (n + 7)/8;
            lon += 0.1 * mult * offsets[offset].first;
            lat += 0.1 * mult * offsets[offset].second;
            drawLine = false;
        }
        else
        {
            drawLine = true;
        }
    }
    return QPair<double, double>(lat, lon);
}
QPair<double, double> QSOMapFrame::calcLoc(QString loc)
{
    // 1st letter, number increases East
    // 2nd letter, number increases North

    double slat;
    double slon;
    loc = loc.left(4) + "AX";   // should be top right
    /*char v =*/ lonlat( loc, slon, slat, true);

    double lat = raddeg(slat);
    double lon = raddeg(slon);
    return QPair<double, double>(lat, lon);
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
        QString currBand = c->currentBand.getValue();

        if (lct->band != currBand)
        {
            return;
        }

        bool drawLine = true;
        QPair<double, double> pos = calcPosition(loc, drawLine);

        QStringList callInfo; // [callsign, latitude, longitude]

        callInfo << lct->cs.getFullCall();
        callInfo << QString::number(pos.first);
        callInfo << QString::number(pos.second);
        callInfo << loc;
        callInfo << (drawLine?"true":"false");
        emit callSig(callInfo);

        for( auto const &s: QASCONST(spotQueue))
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
        if (lct)
        {
            Callsign dxCallsign = lct->cs;
            //Frequency dxFreq = lct->getFrequency().getValue();

            if (spotQueue.count() != 0)
            {
                // check for repeat call
                for (int row = 0; row < spotQueue.count(); row++)
                {
                    QSharedPointer<ClusterSpotData> bsd = spotQueue[row];

                    Callsign rowCall = bsd->getDxCall();

                    if (dxCallsign == rowCall)
                    {
                        bsd->setSpotType(bandmapSpotType::DELETED);
                    }
                }
            }
        }
        if (!lct || lct->getHistory().size() > 1)
        {
            emit clearAll();
            locs.clear();
            doRedraw(c, bdrawGrid, bdrawLines, drawSpots, showLoc, locTL, locBR, showNav);
        }
        else
        {
            showContact(c, lct);
        }
    }
}

void QSOMapFrame::on_redrawQSOMap(bool grid, bool lines, bool spots,
                                    bool sl, QString tl, QString br, bool sn)
{
    if (ct != nullptr)
    {
        // clear map, redraw home and all QSOs

        emit clearAll();
        locs.clear();

        doRedraw(ct, grid, lines, spots, sl, tl, br, sn);
    }
}

//---------------------- Cluster Spots -------------------------------------
bool QSOMapFrame::matchMode(QSharedPointer<ClusterSpotData> bsd)
{
    QString mode = bsd->getMode();

    if (!mode.isEmpty())
    {
        return ct->getQSOMapFilter().getModeFilter(mode);
    }
    else
    {
        return false;
    }
}

bool QSOMapFrame::matchDistance(QSharedPointer<ClusterSpotData> bsd)
{
    if (!ct->getQSOMapFilter().getIgnoreDistanceFlag())
    {
        bool ok = false;

        bandmapSpotType::SPOT_TYPE savedSpot = bsd->getSpotType();
        QString distanceStr =bsd->getDxDist();
        if (distanceStr.isEmpty() && ct->getQSOMapFilter().getIgnoreEmptyDistanceFlag()
            && savedSpot == bandmapSpotType::CLUSTER )
        {
            return false;
        }

        int distance = distanceStr.toInt(&ok);
        if (ok)
        {
            return ct->getQSOMapFilter().testDistance(distance, false);
        }
    }
    return true;
}

void QSOMapFrame::drawSpot(QSharedPointer<ClusterSpotData> bsd)
{
    if (drawSpots && ct != nullptr)
    {
        if (bsd->getDxCall().getValRes() != CS_OK)    // duplicate?
        {
            return;
        }
        if (bsd->getSpotType() == bandmapSpotType::DELETED)
        {
            trace(QString("QSOMapFrame::drawSpot Deleted %1").arg(bsd->getDxCallStr()));
            return;
        }

        QString loc = bsd->getDxLocator();
        if (loc.isEmpty())
        {
            return;
        }
        QString currBand = ct->currentBand.getValue();

        if (bsd->getBand() != currBand)
        {
            trace(QString("QSOMapFrame::drawSpot not current band %1 %2").arg(bsd->getDxCallStr(), bsd->getMode()));
            return;
        }

        if (matchMode(bsd) && matchDistance(bsd))
        {
            trace(QString("QSOMapFrame::drawSpot filtered OK %1").arg(bsd->getDxCallStr()));
            bool drawLine = true;
            QPair<double, double> pos = calcPosition(loc, drawLine);

            QStringList callInfo; // [callsign, latitude, longitude]

            callInfo << bsd->getDxCall().getFullCall();
            callInfo << QString::number(pos.first);
            callInfo << QString::number(pos.second);
            callInfo << loc;
            callInfo << (drawLine?"true":"false");
            emit spotSig(callInfo);
        }
        else
        {
            trace(QString("QSOMapFrame::drawSpot Not filtered OK %1 %2"
                          "").arg(bsd->getDxCallStr(), bsd->getMode()));
        }
    }
}
bool QSOMapFrame::checkSpotInTable(QSharedPointer<ClusterSpotData> spot)
{
    Callsign dxCallsign = spot->getDxCall();
    Frequency dxFreq = spot->getFreq();

    if (spotQueue.count() != 0)
    {
        // check for repeat call
        for (int row = 0; row < spotQueue.count(); row++)
        {
            QSharedPointer<ClusterSpotData> bsd = spotQueue[row];

            Callsign rowCall = bsd->getDxCall();
            bandmapSpotType::SPOT_TYPE spotType = bsd->getSpotType();

            if (dxCallsign == rowCall)
            {
                if ( spotType == bandmapSpotType::LOGGED || spotType == bandmapSpotType::SAVED
                     || spotType == bandmapSpotType::CLUSTER_MARKED)
                {
                    return false; // don't save this spot to the spot list

                }
                else if (spotType == bandmapSpotType::CLUSTER)
                {
                    // yes, remove old spot
                    trace(QString("QSOMapFrame::checkSpotInTable CheckSpot In Table Remove - Cluster Spot %1").arg(rowCall.getFullCall()));
                    bsd->setSpotType(bandmapSpotType::DELETED);
                    // and this spot will be used instead
                }
            }
        }
    }
    return true;
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
            trace(QString("QSOMapFrame::dxSpots  retrieve cluster spot from queue - spot = %1 for loggeruuid = %2, this contest uuid = %3").arg(msg.getMessage(), msg.getLoggerUuid(), ct->uuid));

            // if loggerUuid is empty, message is for all frames
            if ((msg.getLoggerUuid().isEmpty() || msg.getLoggerUuid() == ct->uuid)
                    && (msg.getFrameId() == resendFrameId::BANDMAP_CLIENT
                        || msg.getFrameId() == resendFrameId::ALL_CLIENTS))
            {
                if (msg.getMessage().contains(DXSPOT) || msg.getMessage().contains(RESENTSPOT))
                {
                    trace(QString("QSOMapFrame::dxSpots Spot for this loggeruuid = %1, add to queue").arg(ct->uuid));
                    QSharedPointer<ClusterSpotData> sp = stringToDxSpot(msg.getMessage(), ct, timeToLive);
                    if (!sp || !checkSpotInTable(sp))
                    {
                        continue; // spot logged or marked and moved
                    }

                    bool fromNode = sp->getDxLocatorIsFromNode();
                    bool wkd = sp->getDxCallWorked();
                    bool locEmpty = sp->getDxLocator().isEmpty();
                    if (!fromNode && !wkd && !locEmpty)
                    {
                        trace(QString("QSOMapFrame::dxSpots draw cluster spot spot = %1").arg(msg.getMessage()));
                        spotQueue += sp;
                        drawSpot(sp);
                    }
                    else
                    {
                        trace(QString("QSOMapFrame::dxSpots don't draw cluster spot spot = %1 fromNode %2 WKD %3 locEmpty %4")
                              .arg(msg.getMessage())
                              .arg(fromNode)
                              .arg(wkd)
                              .arg(locEmpty)
                              );
                    }
                }
            }
            else
            {
                trace("QSOMapFrame::dxSpots Not for this contest");
            }
        }
    }
}
void QSOMapFrame::purgeSpots()
{
    bool needRedraw = false;
    if (spotQueue.count() > 0)
    {
        int idx = spotQueue.count() - 1;
        while (idx >= 0 && spotQueue.count() > 0)
        {
            if ((timeToLive > 0 && spotTimedOut(spotQueue[idx]->getRxTime(), timeToLive))
                    || spotQueue[idx]->getSpotType() == bandmapSpotType::DELETED)
            {
                QString pcall = spotQueue[idx]->getDxCall().getFullCall();
                spotQueue.remove(idx);
                trace(QString("QSOMapFrame purged spot = %1, count %2").arg(pcall).arg(spotQueue.count()));
                needRedraw = true;
            }
            idx--;
        }
    }
    if (needRedraw)
    {
        emit clearAll();
        locs.clear();
        doRedraw(ct, bdrawGrid, bdrawLines, drawSpots, showLoc, locTL, locBR, showNav);
    }
}

