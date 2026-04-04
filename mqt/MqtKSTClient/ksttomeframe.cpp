#include <QHeaderView>
#include <QSettings>
#include <QKeyEvent>

#include "delayedaction.h"
#include "kstmsgframe.h"
#include "kstcallsframe.h"
#include "kstloginframe.h"
#include "kstmainwindow.h"
#include "kstmessagegridmodel.h"
#include "kstsendmeepframe.h"
#include "regsettings.h"

#include "ksttomeframe.h"
#include "ui_ksttomeframe.h"

KSTTomeFrame::KSTTomeFrame(QWidget *parent)
    : MinosPanel(parent)
    , ui(new Ui::KSTTomeFrame)
{
    ui->setupUi(this);

    ui->meepTable->setModel(&kstMeepFilterModel);
    ui->meepTable->horizontalHeader()->setStretchLastSection(true);

    meepDelegate = QSharedPointer<HtmlDelegate>( new HtmlDelegate("meepDelegate", 1.0, 1.0, this)) ;
    ui->meepTable->setItemDelegate(meepDelegate.data());

    QHeaderView *verticalHeader = ui->meepTable->verticalHeader();
    verticalHeader->setVisible(false);
    verticalHeader->setMinimumSectionSize(10);
    verticalHeader->setDefaultSectionSize(10);
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);

    RegSettings rsettings;
    QByteArray state = rsettings.getSettings().value("meepTable/state").toByteArray();
    ui->meepTable->horizontalHeader()->restoreState(state);

    connect( ui->meepTable->horizontalHeader(), &QHeaderView::sectionResized,
            this, &KSTTomeFrame::on_sectionResized, Qt::UniqueConnection);

    kstMeepFilterModel.setSourceModel(&mainWindow->kstMsgFrame->kstMessageModel);

}

KSTTomeFrame::~KSTTomeFrame()
{
    delete ui;
}

void KSTTomeFrame::on_sectionResized(int, int, int)
{
    RegSettings settings;
    QByteArray state;

    state = ui->meepTable->horizontalHeader()->saveState();
    settings.getSettings().setValue("meepTable/state", state);
}

void KSTTomeFrame::on_sectionMoved(int, int, int)
{
    on_sectionResized(0, 0, 0);
}

void KSTTomeFrame::on_FontChanged()
{
    int ls = 10;
    if (meepDelegate)
    {
        QString s = "Memxx";
        QSize r = meepDelegate->docSize(s);
        ls = r.height() *5/4;
    }

    QHeaderView *verticalHeader = ui->meepTable->verticalHeader();
    verticalHeader->setDefaultSectionSize(ls);
}
void KSTTomeFrame::on_showReadcb_stateChanged(int /*arg1*/)
{
    kstMeepFilterModel.setShowRead(ui->showReadcb->isChecked());
}
bool KSTTomeFrame::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress )
    {
        QKeyEvent *ke = dynamic_cast<QKeyEvent *>(event);
        if (ke->key() == Qt::Key_Escape)
        {
            if (obj == ui->toMeFilter)
            {
                ui->toMeFilter->clear();
            }
        }
    }

    return false;    // pass the event on
}
void KSTTomeFrame::scrollMeepToBottom()
{
    delayedAction(this, [=]()
                  {
                      QModelIndex meepIndex = kstMeepFilterModel.index(kstMeepFilterModel.rowCount() - 1, 0);
                      if (meepIndex.isValid())
                      {
                          ui->meepTable->scrollTo(meepIndex, QAbstractItemView::PositionAtBottom);
                      }
                  });
}
void KSTTomeFrame::on_meepTable_clicked(const QModelIndex &index)
{
    if (index.isValid())
    {
        QModelIndex sourceIndex = kstMeepFilterModel.mapToSource(index);
        QSharedPointer<KstMessageLine> line = mainWindow->messageVector->at(sourceIndex.row());
        Callsign call = line->call;
        if (call == mainWindow->myCallsign)
        {
            call = line->otherCall;
        }
        mainWindow->kstSendMeepFrame->setNameFromCall(call, line->chat);
        mainWindow->kstLoginFrame->setActive(line->chat);
    }
}

void KSTTomeFrame::on_meepTable_doubleClicked(const QModelIndex &index)
{
    if (index.isValid())
    {
        QModelIndex sourceIndex = kstMeepFilterModel.mapToSource(index);
        int row = sourceIndex.row();
        QSharedPointer<KstMessageLine> line = mainWindow->messageVector->at(row);
        Callsign call = line->call;
        if (call == mainWindow->myCallsign)
        {
            call = line->otherCall;
        }
        mainWindow->kstCallsFrame->setFilter(call);
    }
}
void KSTTomeFrame::on_includeMeCb_stateChanged(int /*arg1*/)
{
    setMeepFilters();
}
void KSTTomeFrame::on_toFromMecb_stateChanged(int /*arg1*/)
{
    setMeepFilters();
}

void KSTTomeFrame::on_toMeFilter_textChanged(const QString &/*arg1*/)
{
    setMeepFilters();
}
void KSTTomeFrame::setMeepFilters()
{
    kstMeepFilterModel.setMyCsFilterString(mainWindow->myCallsign.getFullCall());

    if (ui->includeMeCb->isChecked())
    {
        kstMeepFilterModel.setMyCsFilterString(mainWindow->myCallsign.getFullCall());
    }
    else
    {
        kstMeepFilterModel.setMyCsFilterString(QString());
    }
    kstMeepFilterModel.setToFromFilter(ui->toFromMecb->isChecked());
    kstMeepFilterModel.setFilterString(ui->toMeFilter->text().trimmed());

    scrollMeepToBottom();
}

void KSTTomeFrame::on_clearMeepFiltersButton_clicked()
{
    ui->toMeFilter->clear();
    ui->includeMeCb->setChecked(true);
    setMeepFilters();
}
void KSTTomeFrame::setConnected(bool c)
{
    ui->includeMeCb->setText(c?tr("Including %1").arg(mainWindow->myCallsign.getFullCall()):tr("my call"));

    ui->includeMeCb->setChecked(c);
    //ui->includeMeCb->setText(QString());
    kstMeepFilterModel.setMyCsFilterString(QString());
}
