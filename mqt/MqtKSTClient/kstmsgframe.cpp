#include <QHeaderView>
#include <QSettings>
#include <QKeyEvent>

#include "delayedaction.h"
#include "kstactivechatsframe.h"
#include "kstcallsframe.h"
#include "kstmainwindow.h"
#include "kstmessagegridmodel.h"
#include "kstmsgframe.h"
#include "kstsendmeepframe.h"
#include "regsettings.h"
#include "ui_kstmsgframe.h"

KSTMsgFrame::KSTMsgFrame(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::KSTMsgFrame)
{
    ui->setupUi(this);

    ui->messageFilter->installEventFilter(this);
    ui->messageTable->installEventFilter(this);

    installEventFilter(this);

    ui->messageFilter->setFocus();

}

KSTMsgFrame::~KSTMsgFrame()
{
    delete ui;
}

void KSTMsgFrame::setServices(QStringList services)
{
    ui->messageChatFilter->addItem(tr("Active"));
    ui->messageChatFilter->addItems(services);
    ui->messageChatFilter->setCurrentIndex(0);

}
void KSTMsgFrame::setModel(KstMessageGridModel &pkstMessageModel, KstMessageGridSortFilterModel &pkstMessageGridSortFilterModel)
{
    kstMessageModel = &pkstMessageModel;
    kstMessageFilterModel = &pkstMessageGridSortFilterModel;

    ui->messageTable->setModel(kstMessageFilterModel);
    ui->messageTable->horizontalHeader()->setStretchLastSection(true);

    messageDelegate = QSharedPointer<HtmlDelegate>( new HtmlDelegate("messageDelegate", 1.0, 1.0)) ;
    ui->messageTable->setItemDelegate(messageDelegate.data());

    kstMessageModel->delegate = messageDelegate;

    QHeaderView *verticalHeader = ui->messageTable->verticalHeader();
    verticalHeader->setVisible(false);
    verticalHeader->setMinimumSectionSize(10);
    verticalHeader->setDefaultSectionSize(10);
    //    verticalHeader->setSectionResizeMode(QHeaderView::ResizeToContents);
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);

    RegSettings rsettings;
    QByteArray state = rsettings.getSettings().value("messageTable/state").toByteArray();
    ui->messageTable->horizontalHeader()->restoreState(state);

    connect( ui->messageTable->horizontalHeader(), &QHeaderView::sectionResized,
            this, &KSTMsgFrame::on_sectionResized, Qt::UniqueConnection);

}
void KSTMsgFrame::on_sectionResized(int, int, int)
{
    RegSettings settings;
    QByteArray state;

    state = ui->messageTable->horizontalHeader()->saveState();
    settings.getSettings().setValue("messageTable/state", state);
}

void KSTMsgFrame::on_sectionMoved(int, int, int)
{
    on_sectionResized(0, 0, 0);
}
void KSTMsgFrame::on_FontChanged()
{
    int ls = 10;
    if (messageDelegate)
    {
        QString s = "Memxx";
        QSize r = messageDelegate->docSize(s);
        ls = r.height() *5/4;
    }

    QHeaderView *verticalHeader = ui->messageTable->verticalHeader();
    verticalHeader->setDefaultSectionSize(ls);

}
void KSTMsgFrame::on_clearMessageFilter_clicked()
{

    ui->messageChatFilter->setCurrentIndex(0);
    ui->messageFilter->clear();

    mainWindow->kstMeepFilterModel.invalidate();    // try to get rid of the colouring in the meep table

    scrollMesToBottom();
}
void KSTMsgFrame::scrollMesToBottom()
{
    if (!mouseInMessages)
    {
        delayedAction(this, [=]()
                      {
                          QModelIndex mesIndex = kstMessageFilterModel->index(kstMessageFilterModel->rowCount() - 1, 0);
                          if (mesIndex.isValid())
                          {
                              ui->messageTable->scrollTo(mesIndex, QAbstractItemView::PositionAtBottom);
                          }
                      });
    }
}

void KSTMsgFrame::setFilter(QString s)
{
    ui->messageFilter->setText(s);

}

void KSTMsgFrame::setActive(int chat)
{
    ui->messageChatFilter->setCurrentIndex(chat);

}
void KSTMsgFrame::setFocus()
{
    ui->messageFilter->setFocus();
}
void KSTMsgFrame::on_messageChatFilter_currentIndexChanged(int index)
{
    if (mainWindow->started)
    {
        int messageChatFilter = index;

        kstMessageFilterModel->setChatFilter(messageChatFilter);

        QSettings settings(mainWindow->iniName, QSettings::IniFormat);
        settings.setValue("messageChatFilter", QString::number(messageChatFilter));
    }
}
void KSTMsgFrame::on_messageTable_clicked(const QModelIndex &index)
{
    QModelIndex sourceIndex = kstMessageFilterModel->mapToSource(index);
    int row = sourceIndex.row();
    if (row >= mainWindow->messageVector->size())
        return;
    QSharedPointer<KstMessageLine> line = mainWindow->messageVector->at(row);
    Callsign call = line->call;
    if (mainWindow->myCallsign == call)
    {
        call = line->otherCall;
    }

    mainWindow->kstSendMeepFrame->setNameFromCall(call, line->chat);
    mainWindow->kstActiveChatsFrame->setActive(line->chat);

    QString t = line->message;
    ui->bodyLabel->setText(t);
}


void KSTMsgFrame::on_messageTable_doubleClicked(const QModelIndex &index)
{
    if (index.isValid())
    {
        QModelIndex sourceIndex = kstMessageFilterModel->mapToSource(index);
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
void KSTMsgFrame::on_clearSelectedMessage_clicked()
{
    ui->bodyLabel->clear();
}
void KSTMsgFrame::on_messageFilter_textChanged(const QString &arg1)
{
    kstMessageFilterModel->setFilterString(arg1.toUpper());
    scrollMesToBottom();
}

bool KSTMsgFrame::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->messageTable)
    {
        if (event->type() == QEvent::Enter)
        {
            QModelIndex mesIndex = kstMessageFilterModel->index(kstMessageModel->rowCount() - 1, 0);
            mouseInMessages = true;
            kstMessageFilterModel->setMousePausePoint(mesIndex.row());
            ui->messageTable->update();
            ui->pauseLabel->setText(HtmlFontColour(Qt::red) + tr("Message updates paused"));
        }
        else if (event->type() == QEvent::Leave)
        {
            mouseInMessages = false;
            kstMessageFilterModel->setMousePausePoint(-1);
            ui->messageTable->update();
            ui->pauseLabel->clear();
            scrollMesToBottom();
        }
        if (event->type() == QEvent::KeyPress )
        {
            QKeyEvent *ke = dynamic_cast<QKeyEvent *>(event);
            if (ke->key() == Qt::Key_Escape)
            {
                if (obj == ui->messageFilter)
                {
                    ui->messageFilter->clear();
                }
            }
        }

    }
    return false;
}
void KSTMsgFrame::showAirscoutPath()
{
    QModelIndex index = ui->messageTable->currentIndex();
    QModelIndex sourceIndex = kstMessageFilterModel->mapToSource(index);
    int row = sourceIndex.row();
    if (row < 0 || row >= mainWindow->messageVector->size())
        return;
    QSharedPointer<KstMessageLine> line = mainWindow->messageVector->at(row);

    QSharedPointer<KstUser> user = mainWindow->getUser(KstUser(line->call, mainWindow->getActiveChat()));
    QSharedPointer<KstUser> other = mainWindow->getUser(KstUser(line->otherCall, mainWindow->getActiveChat()));

    if (user && other)
    {
        mainWindow->asl->asShowPath(user, other);
    }
}
