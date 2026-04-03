#include <QKeyEvent>

#include "kstcallsframe.h"
#include "MinosRPC.h"
#include "QtUtils.h"
#include "RPCCommandConstants.h"

#include "kstcallgridmodel.h"
#include "kstloginframe.h"
#include "kstmainwindow.h"
#include "kstmsgframe.h"
#include "kstplanesframe.h"
#include "regsettings.h"
#include "kstsendmeepframe.h"
#include "ui_kstcallsframe.h"

KSTCallsFrame::KSTCallsFrame(QWidget *parent)
    : MinosPanel(parent)
    , ui(new Ui::KSTCallsFrame)
{
    ui->setupUi(this);

    ui->CSFilter->installEventFilter(this);
    installEventFilter(this);   // so we pick up return, and implement the default button
    ui->stringRb->setChecked(true);

    showAircout(mainWindow->getASActive());
    ui->maxDistanceEdit->setText(QString::number(mainWindow->getMaxDistance()));
    ui->maxDistanceEdit->setValidator(new QIntValidator(0, 0xffff, this));

    kstCallModel.setCallVector(mainWindow->callVector);

    kstCallFilterModel.setSourceModel(&kstCallModel);

    ui->CSTable ->setModel(&kstCallFilterModel);
    ui->CSTable->horizontalHeader()->setStretchLastSection(true);

    CSDelegate = QSharedPointer<HtmlDelegate>( new HtmlDelegate("CSDelegate", 1.0, 1.0)) ;
    ui->CSTable->setItemDelegate(CSDelegate.data());
    kstCallModel.delegate = CSDelegate;

    QHeaderView *verticalHeader = ui->CSTable->verticalHeader();
    verticalHeader->setVisible(false);
    verticalHeader->setDefaultSectionSize(10);
    verticalHeader->setMinimumSectionSize(10);
    verticalHeader->setSectionResizeMode(QHeaderView::ResizeToContents);

    RegSettings rsettings;
    QByteArray state = rsettings.getSettings().value("CSTable/state").toByteArray();
    ui->CSTable->horizontalHeader()->restoreState(state);
    ui->CSTable->horizontalHeader()->setStretchLastSection(true);
    ui->CSTable->horizontalHeader()->setSectionsMovable( true );

    connect( ui->CSTable->horizontalHeader(), &QHeaderView::sectionResized,
            this, &KSTCallsFrame:: on_sectionResized, Qt::UniqueConnection);

    connect( ui->CSTable->horizontalHeader(), &QHeaderView::sectionMoved,
            this, &KSTCallsFrame::on_sectionMoved);


    connect( ui->CSTable->horizontalHeader(), &QHeaderView::sortIndicatorChanged,
            this, &KSTCallsFrame::on_sortIndicatorChanged);

    connect(ui->CSTable->selectionModel(),&QItemSelectionModel::selectionChanged,
            this, &KSTCallsFrame::onCSTableSelectionChanged);

}

KSTCallsFrame::~KSTCallsFrame()
{
    delete ui;
}
void KSTCallsFrame::showAircout(bool s)
{
    if(s)
    {
        ui->CSTable->showColumn(ecscAirscout);
    }
    else
    {
        ui->CSTable->hideColumn(ecscAirscout);
    }
}
void KSTCallsFrame::on_FontChanged()
{

}
void KSTCallsFrame::on_CSFilter_textChanged(const QString &arg1)
{
    kstCallFilterModel.setFilterString(arg1.toUpper());
}
void KSTCallsFrame::on_stringRb_clicked()
{
    kstCallFilterModel.setStringDXCC(ui->countryRb->isChecked());
    kstCallFilterModel.invalidate();
}

void KSTCallsFrame::on_CSChatFilter_currentIndexChanged(int index)
{
    if (mainWindow->started)
    {
        int CSChatFilter = index;

        kstCallFilterModel.setChatFilter(CSChatFilter);

        QSettings settings(mainWindow->iniName, QSettings::IniFormat);
        settings.setValue("CSChatFilter", QString::number(CSChatFilter));
    }
}
void KSTCallsFrame::on_countryRb_clicked()
{
    kstCallFilterModel.setStringDXCC(ui->countryRb->isChecked());
    kstCallFilterModel.invalidate();
}
void KSTCallsFrame::setServices(QStringList services)
{
    ui->CSChatFilter->addItem(tr("Active"));
    ui->CSChatFilter->addItems(services);
    ui->CSChatFilter->setCurrentIndex(0);
}
void KSTCallsFrame::on_sectionResized(int, int, int)
{
    RegSettings settings;
    QByteArray state;

    state = ui->CSTable->horizontalHeader()->saveState();
    settings.getSettings().setValue("CSTable/state", state);

}
void KSTCallsFrame::on_sectionMoved(int, int, int)
{
    on_sectionResized(0, 0, 0);
}
void KSTCallsFrame::on_sortIndicatorChanged(int /*logicalIndex*/, Qt::SortOrder /*order*/)
{
    on_sectionResized(0, 0, 0);
}
void KSTCallsFrame::onCSTableSelectionChanged(const QItemSelection &/*selected*/, const QItemSelection &/*deselected*/)
{
    QModelIndexList mil = ui->CSTable->selectionModel()->selectedRows();

    QString mselstring;
    for(auto &mi: mil)
    {
        QModelIndex m = kstCallFilterModel.mapToSource(mi);
        int r = m.row();
        if (r >= 0 && r < mainWindow->callVector->size())
        {
            QSharedPointer<KstUser> user = mainWindow->callVector->at(r);
            if (!mselstring.isEmpty())
            {
                mselstring += " ";
            }
            mselstring += user->call.getFullCall();
        }
    }
    mainWindow->kstMsgFrame->setFilter(mselstring);
    if (mil.count() == 1)
    {
        QModelIndex m = kstCallFilterModel.mapToSource(mil[0]);
        int r = m.row();
        if (r >= 0 && r < mainWindow->callVector->size())
        {
            QSharedPointer<KstUser> user = mainWindow->callVector->at(r);

            // probably implement as a signal via mainForm, picked up
            // by those interested

            if (!ui->noSetCallcb->isChecked())
            {
                mainWindow->kstSendMeepFrame->setNameFromCall(user->call, user->chat);    //send meep, msgEdit "Hi Fred"

                mainWindow->kstLoginFrame->setActive(user->chat);
                mainWindow->kstMsgFrame->setActive(user->chat);
            }
            // Planes
            mainWindow->kstPlanesFrame->showPlanes(user);
            ui->loggerXferButton->setDefault(true);
        }
    }
    else if (mil.count() == 0)
    {
        mainWindow->kstMsgFrame->on_clearMessageFilter_clicked();
    }
}
void KSTCallsFrame::acChanged(QSharedPointer<KstUser> user)
{
    int row = mainWindow->callVector->indexOf(user);
    emit kstCallModel.dataChanged(kstCallModel.index(row, ecscAirscout), kstCallModel.index(row, ecscAirscout));
}
void KSTCallsFrame::setDefaultButton(bool s)
{
    ui->loggerXferButton->setDefault(s);
}

void KSTCallsFrame::setFilter(Callsign &c)
{
    ui->CSFilter->setText(c.getFullCall());
    ui->CSTable->selectRow(0);

}

void KSTCallsFrame::on_loggerXferButton_clicked()
{
    QModelIndexList mil = ui->CSTable->selectionModel()->selectedRows();

    if (mil.size() == 1)
    {

        auto &mi = mil[0];
        QModelIndex m = kstCallFilterModel.mapToSource(mi);
        int r = m.row();
        QSharedPointer<KstUser> user = mainWindow->callVector->at(r);
        QString call = user->call.getFullCall();
        QString loc = user->loc;

        int hyphen = call.indexOf("-");
        if (hyphen > 0)
        {
            call = call.left(hyphen);
        }

        QStringList rList = mainWindow->routerList();
        for(const auto &router: QASCONST(rList))
        {
            RPCGeneralClient rpc(rpcConstants::KSTTransfer);
            QSharedPointer<RPCParam>st(new RPCParamStruct);
            st->addMember( call, rpcConstants::KSTTransferCall );
            st->addMember( loc, rpcConstants::KSTTransferLocator );
            rpc.getCallArgs() ->addParam( st );
            rpc.queueCall( router );
        }
    }
}
void KSTCallsFrame::on_clearUserFilter_clicked()
{
    ui->CSChatFilter->setCurrentIndex(0);
    ui->CSFilter->clear();
    ui->CSTable->clearSelection();
    mainWindow->kstMsgFrame->on_clearMessageFilter_clicked();
    ui->CSFilter->setFocus();
    mainWindow->kstSendMeepFrame->on_clearMessageButton_clicked();
}
void KSTCallsFrame::on_awayCallscb_stateChanged(int)
{
    if (mainWindow->started)
    {
        kstCallFilterModel.setAwayFilter(ui->awayCallscb->isChecked());
    }
}


void KSTCallsFrame::on_inactiveCallscb_stateChanged(int)
{
    if (mainWindow->started)
    {
        kstCallFilterModel.setInactiveFilter(ui->inactiveCallscb->isChecked());
    }
}

void KSTCallsFrame::on_workedCallscb_stateChanged(int)
{
    if (mainWindow->started)
    {
        kstCallFilterModel.setWorkedFilter(ui->workedCallscb->isChecked());
    }
}

void KSTCallsFrame::on_maxDistanceEdit_editingFinished()
{
    mainWindow->setMaxDistance( ui->maxDistanceEdit->text().toInt());
    QSettings settings(mainWindow->iniName, QSettings::IniFormat);
    settings.setValue("maxDistance", mainWindow->getMaxDistance());

    kstCallFilterModel.invalidate();
    mainWindow->kstMsgFrame->kstMessageFilterModel.invalidate();
}
bool KSTCallsFrame::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress )
    {
        QKeyEvent *ke = dynamic_cast<QKeyEvent *>(event);
        if (ke->key() == Qt::Key_Escape)
        {
            if (obj == ui->CSFilter)
            {
                ui->CSFilter->clear();
            }
        }
        if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter)
        {
            if (ui->loggerXferButton->isDefault())
            {
                ui->loggerXferButton->click();
            }
        }
    }

    return false;    // pass the event on
}


