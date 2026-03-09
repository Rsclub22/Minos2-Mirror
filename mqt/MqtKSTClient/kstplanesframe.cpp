#include <QSharedPointer>

#include "kstplanesframe.h"
#include "htmldelegate.h"
#include "kstactivechatsframe.h"
#include "kstcallgridmodel.h"
#include "kstcallsframe.h"
#include "kstmainwindow.h"
#include "kstmsgframe.h"
#include "regsettings.h"
#include "ui_kstplanesframe.h"

KSTPlanesFrame::KSTPlanesFrame(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::KSTPlanesFrame)
{
    ui->setupUi(this);

    mainWindow->kstActiveChatsFrame->setASBands(AirScoutLink::ASBandStrings);
    ui->planesView->installEventFilter(this);
    installEventFilter(this);   // so we pick up return, and implement the default button
}

KSTPlanesFrame::~KSTPlanesFrame()
{
    delete ui;
}

void KSTPlanesFrame::on_FontChanged()
{

}

void KSTPlanesFrame::setModel(KstPlanesModel &pkstPlanesModel, KstPlanesGridSortFilterModel &pkstPlanesFilterModel)
{
    kstPlanesModel = &pkstPlanesModel;
    ui->planesView->setModel(&pkstPlanesFilterModel);

    PlanesDelegate = QSharedPointer<HtmlDelegate>( new HtmlDelegate("PlanesDelegate", 1.0, 1.0)) ;
    ui->planesView->setItemDelegate(PlanesDelegate.data());

    QHeaderView *verticalHeader = ui->planesView->verticalHeader();
    verticalHeader->setVisible(false);
    verticalHeader->setDefaultSectionSize(10);
    verticalHeader->setMinimumSectionSize(10);
    verticalHeader->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->planesView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    connect( ui->planesView->horizontalHeader(), &QHeaderView::sectionResized,
            this, &KSTPlanesFrame::on_sectionResized, Qt::UniqueConnection);
}

void KSTPlanesFrame::on_sectionResized(int, int, int)
{
    RegSettings settings;
    QByteArray state;

    state = ui->planesView->horizontalHeader()->saveState();
    settings.getSettings().setValue("CSTable/state", state);
}

void KSTPlanesFrame::on_sectionMoved(int, int, int)
{
    on_sectionResized(0, 0, 0);
}

void KSTPlanesFrame::showPlanes(QSharedPointer<KstUser> user)
{
    planeActive = user;

    if (user->lastCalcTime.isEmpty())
    {
        ui->planeslabel->setText(tr(""));
    }
    else
    {
        QString l = QString("%1\n%2 at %3\n")
        .arg(user->lastCalcTime, user->fromCall, user->fromLoc)
            + QString("to %1 at %2")
                  .arg(user->toCall, user->toLoc);

        ui->planeslabel->setText(l);
    }
    kstPlanesModel->setPlanesVector(user->planes);

}
void KSTPlanesFrame::acChanged(QSharedPointer<KstUser> user)
{
    mainWindow->kstCallsFrame->acChanged(user);

    if (user == planeActive)
    {
        showPlanes(user);
    }
}
void KSTPlanesFrame::on_showInAS_clicked()
{
    mainWindow->asl->asSelected(planeActive);
}
void KSTPlanesFrame::on_showMPath_clicked()
{
    mainWindow->kstMsgFrame->showAirscoutPath();
}
