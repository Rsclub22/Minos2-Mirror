#include <QQuickView>
#include <QQuickItem>

#include <QVBoxLayout>
#include <QTimer>

#include "contacts.h"
#include "latlong.h"
#include "contest.h"
#include "MinosLoggerEvents.h"
#include "qsomapframe.h"
#include "ui_qsomapframe.h"

QSOMapFrame::QSOMapFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::QSOMapFrame)
{
    ui->setupUi(this);

    // make a reference to the QML window available to C++

    QVBoxLayout *qvb = new QVBoxLayout(this);

    QQuickView *view = new QQuickView(QUrl("qrc:/qsoview.qml"));
    QWidget *container = QWidget::createWindowContainer(view);

    auto qmlObj = view->rootObject();

    qvb->addWidget(container);

    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::AfterLogContactToBandmap, this, &QSOMapFrame::on_AfterLogContact);

    // connect the C++ callSig signal to the QML slot

    connect(this, SIGNAL(callSig(QVariant)), qmlObj, SLOT(newCall(QVariant)));
    connect(this, SIGNAL(homeSig(QVariant)), qmlObj, SLOT(newHome(QVariant)));
}

QSOMapFrame::~QSOMapFrame()
{
    delete ui;
}

void QSOMapFrame::setContest(BaseContestLog *c)
{
    ct = c;
    if (c)
    {
        QStringList callInfo; // [callsign, latitude, longitude]

        callInfo << ct->mycall.getFullCall();
        callInfo << QString::number(raddeg(ct->odna));
        callInfo << QString::number(raddeg(ct->odea));
        emit homeSig(callInfo);
    }
}
void QSOMapFrame::on_AfterLogContact(BaseContestLog *c, QSharedPointer<BaseContact> lct)
{
    if (ct == c && !ct->isReadOnly())
    {
        if ( lct->notValidContact() )
            return;

        QStringList callInfo; // [callsign, latitude, longitude]

        callInfo << lct->cs.getFullCall();
        callInfo << QString::number(raddeg(lct->lat));
        callInfo << QString::number(raddeg(lct->lon));
        emit callSig(callInfo);
    }
}

void QSOMapFrame::closeContest()
{

}
void QSOMapFrame::timeout()
{
    // test code...

    static int n = 0;
    QStringList callInfo; // [callsign, latitude, longitude]
    QString s;

    s = QString::number(n);

    if (n == 0)
        callInfo << "G5QQQ" << "51.9" << "0.7542";
    else if (n < 10)
        callInfo << "G" + s + "QNN" << "52." + s << "-0." + s;
    else
        callInfo << "G5QMM/" + s << "52." + s << "0." + s[1];

   // emit callSig(QVariant(callInfo));

    n++;

}
