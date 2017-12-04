#include <QGridLayout>
#include <QToolButton>

#include "logger_pch.h"
#include "tlogcontainer.h"
#include "tsinglelogframe.h"
#include "rigmemdialog.h"
#include "rigutils.h"
#include "RigMemoryFrame.h"
#include "ui_RigMemoryFrame.h"

static QKeySequence memoryShortCut[] = {

    QKeySequence(Qt::CTRL + Qt::Key_1),
    QKeySequence(Qt::CTRL + Qt::Key_2),
    QKeySequence(Qt::CTRL + Qt::Key_3),
    QKeySequence(Qt::CTRL + Qt::Key_4),
    QKeySequence(Qt::CTRL + Qt::Key_5),
    QKeySequence(Qt::CTRL + Qt::Key_6),
    QKeySequence(Qt::CTRL + Qt::Key_7),
    QKeySequence(Qt::CTRL + Qt::Key_8),
    QKeySequence(Qt::CTRL + Qt::Key_9),
    QKeySequence(Qt::CTRL + Qt::Key_0)
};

static QKeySequence memoryShiftShortCut[] = {

    QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_1),
    QKeySequence(Qt::CTRL + Qt::SHIFT +  Qt::Key_2),
    QKeySequence(Qt::CTRL + Qt::SHIFT  + Qt::Key_3),
    QKeySequence(Qt::CTRL + Qt::SHIFT  + Qt::Key_4),
    QKeySequence(Qt::CTRL + Qt::SHIFT  + Qt::Key_5),
    QKeySequence(Qt::CTRL + Qt::SHIFT  + Qt::Key_6),
    QKeySequence(Qt::CTRL + Qt::SHIFT  + Qt::Key_7),
    QKeySequence(Qt::CTRL + Qt::SHIFT  + Qt::Key_8),
    QKeySequence(Qt::CTRL + Qt::SHIFT  + Qt::Key_9),
    QKeySequence(Qt::CTRL + Qt::SHIFT  + Qt::Key_0)

};

RigMemoryFrame::RigMemoryFrame(QWidget *parent) :
    QFrame(parent),
    ct(0),
    ui(new Ui::RigMemoryFrame)
{
    ui->setupUi(this);
    ui->memFrame->setStyleSheet("background-color:white;");
}

RigMemoryFrame::~RigMemoryFrame()
{
    delete ui;
}

void RigMemoryFrame::on_newMemoryButton_clicked()
{
    int n = -1;

    for (int i = 0; i < memButtonMap.size() + 1; i++)
    {
        if  (!memButtonMap.contains(i))
        {
            n = i;
            break;
        }
    }
    if (n == -1)
    {
        mShowMessage("Panic", this);
        return;
    }

    writeActionSelected(n); // which creates the button as well

}
void RigMemoryFrame::reInitialiseMemories()
{
    //??
    // Clear scroller
    // rebuild memory buttons

    loadMemoryButtonLabels();
}
void RigMemoryFrame::setContest( BaseContestLog *pct )
{
    ct = dynamic_cast<LoggerContestLog *>( pct);

    if (ct)
    {
        reInitialiseMemories();
    }
}
memoryData::memData RigMemoryFrame::getRigMemoryData(int memoryNumber)
{
    memoryData::memData m;
    if (ct->rigMemories.size() > memoryNumber)
        m = ct->rigMemories[memoryNumber].getValue();
    return m;
}
void RigMemoryFrame::setRigMemoryData(int memoryNumber, memoryData::memData m)
{
    ct->saveRigMemory(memoryNumber, m);
}
void RigMemoryFrame::readActionSelected(int buttonNumber)
{
    traceMsg(QString("Memory Read Selected = %1").arg(QString::number(buttonNumber +1)));
    memoryData::memData m = getRigMemoryData(buttonNumber);

    // send detail to rotator control frame, locator will give bearing
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    tslf->transferDetails(m);

}

void RigMemoryFrame::writeActionSelected(int buttonNumber)
{
    traceMsg(QString("Memory Write Selected %1 = ").arg(QString::number(buttonNumber +1)));

    // get contest information
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    ScreenContact sc = tslf->getScreenEntry();

    memoryData::memData logData;
    tslf->getDetails(logData);

    RigMemDialog memDialog(this);
    memDialog.setLogData(&logData, buttonNumber);
    memDialog.setWindowTitle(QString("M%1 - Write").arg(QString::number(buttonNumber + 1)));
   if ( memDialog.exec() == QDialog::Accepted)
   {
       setRigMemoryData(buttonNumber, logData);
       memoryUpdate(buttonNumber);
       clean();
   }
}


void RigMemoryFrame::editActionSelected(int buttonNumber)
{
    traceMsg(QString("Memory Edit Selected = %1").arg(QString::number(buttonNumber + 1)));

    memoryData::memData logData = getRigMemoryData(buttonNumber);

    RigMemDialog memDialog(this);

    memDialog.setLogData(&logData, buttonNumber);
    memDialog.setWindowTitle(QString("M%1 - Edit").arg(QString::number(buttonNumber + 1)));

    if ( memDialog.exec() == QDialog::Accepted)
    {
        setRigMemoryData(buttonNumber, logData);
        memoryUpdate(buttonNumber);
        clean();
    }

}


void RigMemoryFrame::clearActionSelected(int buttonNumber)
{

    traceMsg(QString("Memory Clear Selected = %1").arg(QString::number(buttonNumber + 1)));

    memoryData::memData m;
    setRigMemoryData(buttonNumber, m);

     if (memButtonMap.contains(buttonNumber))
     {
         RigMemoryButton *rmb = memButtonMap[buttonNumber];

         QGridLayout *gl = qobject_cast<QGridLayout *>(ui->memFrame->layout());

         int i = 0;
         QLayoutItem *child1;
         while(( child1 = gl->itemAt(i)) != 0)
         {
             if (child1->widget() == rmb->memButton)
             {
                 QLayoutItem *child2 = gl->takeAt(i);
                 delete child2->widget();
                 delete child2;
                 break;
             }
             i++;
         }
         memButtonMap.remove(buttonNumber);
         delete rmb;

         clean();
     }
}
void RigMemoryFrame::loadMemoryButtonLabels()
{
    int mcount = ct->rigMemories.size();
    for (int i = 0; i < mcount; i ++)
    {
        memoryUpdate(i);
    }
    clean();
}

void RigMemoryFrame::clean()
{
    QGridLayout *gl = qobject_cast<QGridLayout *>(ui->memFrame->layout());
    int i = 0;
    QLayoutItem *child1;
    while(( child1 = gl->itemAt(i)) != 0)
    {
        QLayoutItem *child2 = gl->takeAt(i);
//        delete child2->widget();              // don't delete it - it may be the membutton
        delete child2;
        i++;
    }

    i = 0;
    foreach(RigMemoryButton *rmb, memButtonMap)
    {
        QToolButton *rmbb = rmb->memButton;
        rmbb->setStyleSheet("border: 1px solid black; background-color: #DFDFDF");
        int row = i/2;
        int col = i%2;
        gl->addWidget(rmbb, row, col);
        i++;
    }
}
void RigMemoryFrame::memoryUpdate(int buttonNumber)
{
    memoryData::memData m = getRigMemoryData(buttonNumber);

    if (!memButtonMap.contains(buttonNumber) && m.callsign != memDefData::DEFAULT_CALLSIGN)
    {
        int row = buttonNumber/2;
        int col = buttonNumber%2;
        memButtonMap[buttonNumber] = new RigMemoryButton(ui->scrollArea, this, buttonNumber);
        connect( memButtonMap[buttonNumber], SIGNAL( clearActionSelected(int)) , this, SLOT(clearActionSelected(int)), Qt::QueuedConnection );

        QGridLayout *gl = qobject_cast<QGridLayout *>(ui->memFrame->layout());
        gl->addWidget(memButtonMap[buttonNumber]->memButton, row, col);

        memButtonMap[buttonNumber]->memButton->setStyleSheet("border: 1px solid black; background-color: #DFDFDF");

        // This works - recipe from internet - but not sure why!

        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(false);
        memButtonMap[buttonNumber]->memButton->setSizePolicy(sizePolicy);
    }

    if (memButtonMap.contains(buttonNumber))
    {

        QToolButton *mb = memButtonMap[buttonNumber]->memButton;

        mb->setText("M" + QString::number(buttonNumber + 1) + ": " + m.callsign);

        QString tTipStr = "Callsign: " + m.callsign + "\n"
                + "Freq: " + convertFreqStrDisp(m.freq) + "\n"
                + "Mode: " + m.mode + "\n"
                + "Locator: " + m.locator + "\n"
                + "Bearing: " + QString::number(m.bearing) + "\n"
                + "Time: " + m.time;
        mb->setToolTip(tTipStr);
    }

}



void RigMemoryFrame::traceMsg(QString msg)
{
    trace("RigMemoryFrame: " + msg);
}

RigMemoryButton::RigMemoryButton(QWidget *parent, RigMemoryFrame *rcf, int no)
{
    memNo = no;
    rigMemoryFrame = rcf;

    memButton = new QToolButton(parent);

    memoryMenu = new QMenu(memButton);

    memButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    memButton->setPopupMode(QToolButton::MenuButtonPopup);
    memButton->setFocusPolicy(Qt::NoFocus);
    memButton->setText("M" + QString::number(memNo + 1) );

    readAction = new QAction("&Read", memButton);
    writeAction = new QAction("&Write",memButton);
    editAction = new QAction("&Edit", memButton);
    clearAction = new QAction("&Clear",memButton);
    memoryMenu->addAction(readAction);
    memoryMenu->addAction(writeAction);
    memoryMenu->addAction(editAction);
    memoryMenu->addAction(clearAction);
    memButton->setMenu(memoryMenu);

    if (memNo  < 10)
    {
        shortKey = new QShortcut(QKeySequence(memoryShortCut[memNo]), memButton);
        connect(shortKey, SIGNAL(activated()), this, SLOT(readActionSelected()));
        // shift shortcut
        shiftShortKey = new QShortcut(QKeySequence(memoryShiftShortCut[memNo]), memButton);
        connect(shiftShortKey, SIGNAL(activated()), this, SLOT(memoryShortCutSelected()));
    }
    connect(memButton, SIGNAL(clicked(bool)), this, SLOT(readActionSelected()));
    connect( readAction, SIGNAL( triggered() ), this, SLOT(readActionSelected()) );
    connect( writeAction, SIGNAL( triggered() ), this, SLOT(writeActionSelected()) );
    connect( editAction, SIGNAL( triggered() ), this, SLOT(editActionSelected()) );
    connect( clearAction, SIGNAL( triggered() ), this, SLOT(clearActionSelected()) );
}
RigMemoryButton::~RigMemoryButton()
{
//    delete memButton;
}
void RigMemoryButton::memoryUpdate()
{
    rigMemoryFrame->memoryUpdate(memNo);
}

void RigMemoryButton::memoryShortCutSelected()
{
//    rigControlFrame->memoryShortCutSelected(memNo);
    memButton->showMenu();
}
void RigMemoryButton::readActionSelected()
{
    rigMemoryFrame->readActionSelected(memNo);
}
void RigMemoryButton::editActionSelected()
{
    rigMemoryFrame->editActionSelected(memNo);
}
void RigMemoryButton::writeActionSelected()
{
    rigMemoryFrame->writeActionSelected(memNo);
}
void RigMemoryButton::clearActionSelected()
{
    emit clearActionSelected(memNo);
}
