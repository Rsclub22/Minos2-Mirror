#include <QVBoxLayout>
#include <QTimer>

#ifdef Q_OS_WIN
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QQuickItem>
#include <QQmlContext>
#endif
#endif

#include <QMouseEvent>

#include "contacts.h"
#include "latlong.h"
#include "contest.h"
#include "MinosLoggerEvents.h"
#include "qmlcpplink.h"
#include "qsomapframe.h"
#include "ui_qsomapframe.h"

QSOMapFrame::QSOMapFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::QSOMapFrame)
{
#ifdef Q_OS_WIN
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // This makes the resources accessible from the main executable
    Q_INIT_RESOURCE(qml);
#endif
#endif

    ui->setupUi(this);

    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::AfterLogContactToBandmap, this, &QSOMapFrame::on_AfterLogContact, Qt::UniqueConnection);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::redrawQSOMap, this, &QSOMapFrame::on_redrawQSOMap, Qt::QueuedConnection);

}

QSOMapFrame::~QSOMapFrame()
{
    delete ui;
}
void QSOMapFrame::startMap()
{
#ifdef Q_OS_WIN
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // make a reference to the QML window available to C++

    QLayout *qvb = layout();

    if (!qvb)
    {
        QQmlApplicationEngine* engine = new QQmlApplicationEngine(this);

        QmlCppLink* linkObject = new QmlCppLink(ct);
        QVariant vl;
        vl.setValue(linkObject);

        engine->rootContext()->setContextProperty("QmlCppLink", vl);

        qvb = new QVBoxLayout(this);

        QQuickView *view = new QQuickView(engine, nullptr);
        view->setSource(QUrl("qrc:/qsoview.qml"));

        QWidget *container = QWidget::createWindowContainer(view);
        auto qmlObj = view->rootObject();

        qvb->addWidget(container);

        // connect the C++ callSig signal to the QML slot

        connect(this, SIGNAL(callSig(QVariant)), qmlObj, SLOT(newCall(QVariant)), Qt::UniqueConnection);
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
//   qDebug() << latitude << " " << longitude << " " << bearing;
#endif
#endif
}
void QSOMapFrame::doRedraw(BaseContestLog *c, bool grid, bool lines)
{
    emit drawGrid(grid);
    emit drawLines(lines);

    QStringList callInfo; // [callsign, latitude, longitude]

    callInfo << ct->mycall.getFullCall();
    callInfo << QString::number(raddeg(ct->odna));
    callInfo << QString::number(raddeg(ct->odea));
    callInfo << ct->myloc.getLoc();
    emit homeSig(callInfo);

    for ( auto const &c: qAsConst(c->ctList ))
    {
        QSharedPointer<BaseContact> cct = c.wt;

        if ( cct->notValidContact() )
           continue;

        on_AfterLogContact(ct, cct);
    }
}

void QSOMapFrame::setContest(BaseContestLog *c, bool grid, bool lines)
{
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
void QSOMapFrame::on_AfterLogContact(const BaseContestLog *c, const QSharedPointer<BaseContact> lct)
{
    if (ct == c && !ct->isReadOnly())
    {
        if ( lct->notValidContact() )
            return;

        QStringList callInfo; // [callsign, latitude, longitude]

        callInfo << lct->cs.getFullCall();
        callInfo << QString::number(raddeg(lct->lat));
        callInfo << QString::number(raddeg(lct->lon));
        callInfo << lct->loc.getLoc();
        emit callSig(callInfo);
    }
}

void QSOMapFrame::on_redrawQSOMap(bool grid, bool lines)
{
    // clear map, redraw home and all QSOs

    emit clearAll();

    doRedraw(ct, grid, lines);
}
void QSOMapFrame::closeContest()
{
    setContest(nullptr, false, false);
}

