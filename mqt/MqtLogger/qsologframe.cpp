#include "base_pch.h"
#include "MinosLoggerEvents.h"

#include "ContestApp.h"
#include "LoggerContest.h"
#include "LoggerContacts.h"
#include "ListContact.h"
#include "tlogcontainer.h"
#include "tsinglelogframe.h"
#include "tqsoeditdlg.h"
#include "tforcelogdlg.h"
#include "SendRPCDM.h"
//#include "rigcontrolcommonconstants.h"
#include "rigcommon.h"
#include "bandmapcommon.h"
#include "rigutils.h"
#include "delayedaction.h"

#include "qsologframe.h"
#include "ui_qsologframe.h"


QSOLogFrame::QSOLogFrame(QWidget *parent) :
    QFrame(parent)
    , ui(new Ui::QSOLogFrame)
    , selectedContact(nullptr)
    , partialContact(nullptr)
    , oldTimeOK(true)
    , contest(nullptr)
    , overstrike(false)
    , current(nullptr)
    , edit(false)
    , rotatorLoaded(false)
    , radioLoaded(false)
    , bandMapLoaded(false)
    , logDataFromBandmapOrMemory(false)
    , keyerLoaded(false)
    , radioConnected(false)
    , radioError(false)
    , clusterClientLoaded(false)
    , clusterServerLoaded(false)
    , sendSpotToClusterOn(false)
    , clusterServerConnected(false)
    , runButtonOnFlag(false)
    , radioOffRunFreq(false)


{
    ui->setupUi(this);

    ui->BrgSt->setFixedSize(ui->BrgSt->size());
    ui->DistSt->setFixedSize(ui->DistSt->size());

    ui->CallsignFrame->setup("Call", this);
    CallsignLabelString = tr("Callsign (F1)");
    CallsignFW = new FocusWatcher(ui->CallsignFrame->getTextEditEdit());
    ui->CallsignFrame->getTextEditlabel()->setText("<b>" + CallsignLabelString);
    connect(ui->CallsignFrame->getTextEditEdit(), SIGNAL(textChanged(const QString &)), this, SLOT(onCallsignEdit_textChanged(const QString &)));

    ui->RSTTxFrame->setup("RstTx", this);
    RSTTXLabelString = tr("RS(T)Tx(F2)");
    RSTTXFW = new FocusWatcher(ui->RSTTxFrame->getTextEditEdit());
    ui->RSTTxFrame->getTextEditlabel()->setText("<b>" + RSTTXLabelString);
    connect(ui->RSTTxFrame->getTextEditEdit(), SIGNAL(textChanged(const QString &)), this, SLOT(onRSTTXEdit_textChanged(const QString &)));

    ui->SerTxFrame->setup("serTx", this);
    SerTXLabelString = tr("Serial Tx");
    SerTXFW = new FocusWatcher(ui->SerTxFrame->getTextEditEdit());
    ui->SerTxFrame->getTextEditlabel()->setText("<b>" + SerTXLabelString);
    ui->SerTxFrame->getTextEditEdit()->setFocusPolicy(Qt::ClickFocus);
    connect(ui->SerTxFrame->getTextEditEdit(), SIGNAL(textChanged(const QString &)), this, SLOT(onSerTXEdit_textChanged(const QString &)));

    ui->RSTRxFrame->setup("RstRx", this);
    RSTRXLabelString = tr("RS(T)Rx(F3)");
    RSTRXFW = new FocusWatcher(ui->RSTRxFrame->getTextEditEdit());
    ui->RSTRxFrame->getTextEditlabel()->setText("<b>" + RSTRXLabelString);
    connect(ui->RSTRxFrame->getTextEditEdit(), SIGNAL(textChanged(const QString &)), this, SLOT(onRSTRXEdit_textChanged(const QString &)));

    ui->SerRxFrame->setup("SerRx", this);
    SerRXLabelString = tr("Serial Rx (F4)");
    SerRXFW = new FocusWatcher(ui->SerRxFrame->getTextEditEdit());
    ui->SerRxFrame->getTextEditlabel()->setText("<b>" + SerRXLabelString);
    connect(ui->SerRxFrame->getTextEditEdit(), SIGNAL(textChanged(const QString &)), this, SLOT(onSerRXEdit_textChanged(const QString &)));

    ui->LocFrame->setup("Loc", this);
    LocLabelString = tr("Loc (F5)");
    LocFW = new FocusWatcher(ui->LocFrame->getTextEditEdit());
    ui->LocFrame->getTextEditlabel()->setText("<b>" + LocLabelString);
    connect(ui->LocFrame->getTextEditEdit(), SIGNAL(textChanged(const QString &)), this, SLOT(onLocEdit_textChanged(const QString &)));

    ui->QTHFrame->setup("QTH", this);
    QTHLabelString = tr("Exchange (F6)");
    QTHFW = new FocusWatcher(ui->QTHFrame->getTextEditEdit());
    ui->QTHFrame->getTextEditlabel()->setText("<b>" + QTHLabelString);
    connect(ui->QTHFrame->getTextEditEdit(), SIGNAL(textChanged(const QString &)), this, SLOT(onQTHEdit_textChanged(const QString &)));

    ui->commentsFrame->setup("Comments", this);
    CommentsLabelString = tr("Comments");
    CommentsFW = new FocusWatcher(ui->commentsFrame->getTextEditEdit());
    ui->commentsFrame->getTextEditlabel()->setText("<b>" + CommentsLabelString);

    MainOpFW = new FocusWatcher(ui->MainOpComboBox);
    ui->MainOpComboBox->setValidator(&ucValidator);
    Op1String = ui->OperatorLabel->text();
    ui->OperatorLabel->setText("<b>" + Op1String);

    SecondOpFW = new FocusWatcher(ui->SecondOpComboBox);
    ui->SecondOpComboBox->setValidator(&ucValidator);
    Op2String = ui->SecondOpLabel->text();
    ui->SecondOpLabel->setText("<b>" + Op2String);

    freqFW = new FocusWatcher(ui->frequencyEdit);

    connect(CallsignFW, SIGNAL(focusChanged(QObject *, bool, QFocusEvent * )), this, SLOT(focusChange(QObject *, bool, QFocusEvent *)));
    connect(RSTTXFW, SIGNAL(focusChanged(QObject *, bool, QFocusEvent * )), this, SLOT(focusChange(QObject *, bool, QFocusEvent *)));
    connect(SerTXFW, SIGNAL(focusChanged(QObject *, bool, QFocusEvent * )), this, SLOT(focusChange(QObject *, bool, QFocusEvent *)));
    connect(RSTRXFW, SIGNAL(focusChanged(QObject *, bool, QFocusEvent * )), this, SLOT(focusChange(QObject *, bool, QFocusEvent *)));
    connect(SerRXFW, SIGNAL(focusChanged(QObject *, bool, QFocusEvent * )), this, SLOT(focusChange(QObject *, bool, QFocusEvent *)));
    connect(LocFW, SIGNAL(focusChanged(QObject *, bool, QFocusEvent * )), this, SLOT(focusChange(QObject *, bool, QFocusEvent *)));
    connect(QTHFW, SIGNAL(focusChanged(QObject *, bool, QFocusEvent * )), this, SLOT(focusChange(QObject *, bool, QFocusEvent *)));
    connect(CommentsFW, SIGNAL(focusChanged(QObject *, bool, QFocusEvent * )), this, SLOT(focusChange(QObject *, bool, QFocusEvent *)));
    connect(MainOpFW, SIGNAL(focusChanged(QObject *, bool, QFocusEvent * )), this, SLOT(focusChange(QObject *, bool, QFocusEvent *)));
    connect(SecondOpFW, SIGNAL(focusChanged(QObject *, bool, QFocusEvent * )), this, SLOT(focusChange(QObject *, bool, QFocusEvent *)));
    connect(freqFW, SIGNAL(focusChanged(QObject *, bool, QFocusEvent * )), this, SLOT(focusChange(QObject *, bool, QFocusEvent *)));

    ui->timeEdit->installEventFilter(this);
    ui->dateEdit->installEventFilter(this);


    for (auto const &sm: supModeList)
    {
        ui->ModeComboBoxGJV->addItem(sm);
    }

    ui->ModeComboBoxGJV->setCurrentText(hamlibData::USB);
    ui->ModeButton->setText(hamlibData::CW);
    ui->MGMSubModeFrame->setVisible(ui->ModeComboBoxGJV->currentText() == hamlibData::MGM);

    connect(&MinosLoggerEvents::mle, SIGNAL(AfterTabFocusIn(QLineEdit*)), this, SLOT(on_AfterTabFocusIn(QLineEdit*)), Qt::QueuedConnection);
    connect(&MinosLoggerEvents::mle, SIGNAL(ValidateError(int)), this, SLOT(on_ValidateError(int)));
    connect(&MinosLoggerEvents::mle, SIGNAL(ShowOperators()), this, SLOT(on_ShowOperators()));
    connect(&MinosLoggerEvents::mle, SIGNAL(tabSandP()), this, SLOT(on_tabSandP()));
    connect(&MinosLoggerEvents::mle, SIGNAL(FontChanged()), this, SLOT(on_FontChanged()), Qt::QueuedConnection);
    connect(&MinosLoggerEvents::mle, SIGNAL(QSOMargins()), this, SLOT(on_QSOMargins()));

    connect(ui->tuningAddMapChkBox, SIGNAL(stateChanged(int)), this, SLOT(tuningAddMapChkBoxStateChange(int)));

    TContestApp::getContestApp() ->loggerBundle.getIntProfile( elpAddBandMapTuningTolerance, addToBandmapTuneTolerance );

    if (addToBandmapTuneTolerance < ADD_TUNING_BANDMAP_FREQ_DEFAULT_MIN_TOLERANCE || addToBandmapTuneTolerance > ADD_TUNING_BANDMAP_FREQ_DEFAULT_MAX_TOLERANCE)
    {
       addToBandmapTuneTolerance =  ADD_TUNING_BANDMAP_FREQ_DEFAULT_TOLERANCE;
        trace(QString("addToBandmapTuneTolerance read in out of range = %1 khz, set default").arg(addToBandmapTuneTolerance));
    }
    else
    {
        trace(QString("addToBandmapTuneTolerance read in = %1 khz").arg(addToBandmapTuneTolerance));
    }
    on_tabSandP();  // show (or not) the Call/S&P choice

    on_QSOMargins();
}

void QSOLogFrame::on_FontChanged()
{
    int lcf;
    TContestApp::getContestApp() ->getIntDisplayProfile(edpQSOFieldFont, lcf);

    QFont cf = QApplication::font();
    qreal fs = cf.pointSizeF();
    int fsi = static_cast<int>(fs * lcf/100.0);
    cf.setPointSize(fsi);
    ui->CallsignFrame->getTextEditEdit()->setFont(cf);

    ui->RSTTxFrame->getTextEditEdit()->setFont(cf);
    ui->SerTxFrame->getTextEditEdit()->setFont(cf);
    ui->RSTRxFrame->getTextEditEdit()->setFont(cf);
    ui->SerRxFrame->getTextEditEdit()->setFont(cf);
    ui->LocFrame->getTextEditEdit()->setFont(cf);
    ui->QTHFrame->getTextEditEdit()->setFont(cf);


    for (QMap<QWidget *, QString>::iterator i = widgetStyles.begin(); i != widgetStyles.end(); i++)
    {
        QWidget *w = i.key();
        w->setStyleSheet(i.value());
    }
}

void QSOLogFrame::on_QSOMargins()
{
    int lm;
    int ls;
    int cml;
    int cmt;
    int cmr;
    int cmb;

    TContestApp::getContestApp() ->getIntDisplayProfile(edplm, lm);
    TContestApp::getContestApp() ->getIntDisplayProfile(edpls, ls);
    TContestApp::getContestApp() ->getIntDisplayProfile(edpcml, cml);
    TContestApp::getContestApp() ->getIntDisplayProfile(edpcmt, cmt);
    TContestApp::getContestApp() ->getIntDisplayProfile(edpcmr, cmr);
    TContestApp::getContestApp() ->getIntDisplayProfile(edpcmb, cmb);

    adjustMargins(layout(), lm, ls, cml, cmt, cmr, cmb);
}

bool QSOLogFrame::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *ke = dynamic_cast<QKeyEvent *>(event);
        return doKeyPressEvent(ke);
    }
    else if (event->type() == QEvent::MouseButtonDblClick)
    {
        if (obj == ui->SerTxFrame->getTextEditEdit() || (edit && (obj == ui->timeEdit || obj == ui->dateEdit)))
        {
            do_mouseDoubleClickEvent(obj);
        }
   }

   return false;
}
bool QSOLogFrame::doKeyPressEvent( QKeyEvent* event )
{
    if (!event)
        return false;

    int Key = event->key();

    Qt::KeyboardModifiers mods = event->modifiers();
    bool shift = mods & Qt::ShiftModifier;
    bool ctrl = mods & Qt::ControlModifier;
    //bool alt = mods & Qt::AltModifier;

    if (Key == Qt::Key_Return || Key == Qt::Key_Enter)
    {
        QMetaObject::invokeMethod(ui->GJVOKButton, "clicked", Qt::QueuedConnection);
        raise();
        return true;
    }
    else if (Key == Qt::Key_Tab)
    {
         // Do we want "call" tab order or "S and P" tab order?
         bool tabSandP;
         TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpTabforSandP, tabSandP );

         if (tabSandP)
         {
             tabSandP = ui->SandPrb->isChecked();
         }

         if (tabSandP && edit == false && catchup == false)
         {
             if (current == ui->CallsignFrame->getTextEditEdit() && !rrIl->valid( cmValidStatus, screenContact ))
             {
                 selectField( ui->RSTRxFrame->getTextEditEdit() );
                 return true;
             }
         }
         return false;
    }
    else if (Key == Qt::Key_Escape)
    {
        QMetaObject::invokeMethod(ui->GJVCancelButton, "clicked", Qt::QueuedConnection);
        raise();
        return true;
    }
    else if ( ( Key == Qt::Key_F1 || Key == Qt::Key_F2 || Key == Qt::Key_F3 || Key == Qt::Key_F4 || Key == Qt::Key_F5 || Key == Qt::Key_F6|| Key == Qt::Key_F12) )
    {
        setActiveControl( &Key );
        raise();
        return true;
    }

    bool doReturn = false;
    if ( ( Key == Qt::Key_Insert ) && !shift && !ctrl )
    {
        overstrike = !overstrike;
        doReturn = true;
    }
    bool ovr = overstrike;

    QLineEdit *ed = dynamic_cast<QLineEdit *>( current );

    MinosLoggerEvents::SendReportOverstrike(ovr, contest);  // queued

    if (doReturn)
        return true;

    if (ed)
    {

        if (ovr && ((Key&0xff) == Key))    // QT keeps 8 bit ASCII range for real keys
        {
            int cpos = ed->cursorPosition();
            int edLen = ed->text().size();
            if (cpos < edLen)
            {
                QString edText = ed->text();
                if ( Key != Qt::Key_Delete )
                {
                    // just delete the chracter at cursor pos, and let new char be inserted
                    edText = edText.left(cpos) + edText.right(edLen - cpos - 1);
                    ed->setText(edText);
                    ed->setCursorPosition(cpos);
                }
                else
                {
                    // need to overstrike with space, but move cursor back one
                    edText = edText.left(cpos) + " " + edText.right(edLen - cpos);
                    ed->setText(edText);
                    ed->setCursorPosition(cpos);
                }
            }
        }
        if (ed == ui->CallsignFrame->getTextEditEdit())
        {
            ui->CallsignFrame->getTextEditEdit()->setStyleSheet(ssLineEditOK);
            widgetStyles[ui->CallsignFrame->getTextEditEdit()] = ssLineEditOK;
        }
    }
    return false;
}

QSOLogFrame::~QSOLogFrame()
{
    delete ui;
    for ( auto const & vcp: qAsConst(vcs) )
    {
       delete vcp;
    }
    killPartial();
}
void QSOLogFrame::on_TimeDisplayTimer()
{
    updateQSOTime(true);

    checkQsoFrameColour();

    checkBandMapAndClusterLoaded();
}

void QSOLogFrame::focusChange(QObject *obj, bool in, QFocusEvent *event)
{
    if (in)
        EditControlEnter(obj, event);
    else
        EditControlExit(obj);

    QColor colour = in?Qt::blue:Qt::black;
    QString colStr = HtmlFontColour(colour) + "<b>";

    if (obj == ui->CallsignFrame->getTextEditEdit())
    {
        ui->CallsignFrame->getTextEditlabel()->setText(colStr + CallsignLabelString);
    }

    if (obj == ui->RSTTxFrame->getTextEditEdit())
    {
        ui->RSTTxFrame->getTextEditlabel()->setText(colStr + RSTTXLabelString);
    }

    if (obj == ui->SerTxFrame->getTextEditEdit())
    {
        ui->SerTxFrame->getTextEditlabel()->setText(colStr + SerTXLabelString);
    }

    if (obj == ui->RSTRxFrame->getTextEditEdit())
    {
        ui->RSTRxFrame->getTextEditlabel()->setText(colStr + RSTRXLabelString);
    }

    if (obj == ui->SerRxFrame->getTextEditEdit())
    {
        ui->SerRxFrame->getTextEditlabel()->setText(colStr + SerRXLabelString);
    }

    if (obj == ui->LocFrame->getTextEditEdit())
    {
        ui->LocFrame->getTextEditlabel()->setText(colStr + LocLabelString);
    }

    if (obj == ui->QTHFrame->getTextEditEdit())
    {
        ui->QTHFrame->getTextEditlabel()->setText(colStr + QTHLabelString);
    }

    if (obj == ui->commentsFrame->getTextEditEdit())
    {
        ui->commentsFrame->getTextEditlabel()->setText(colStr + CommentsLabelString);
    }
    if (obj == ui->MainOpComboBox)
    {
        ui->OperatorLabel->setText(colStr + Op1String);
        if (in == false)
        {
            MainOpComboBox_Exit();
        }
    }
    if (obj == ui->SecondOpComboBox)
    {
        ui->SecondOpLabel->setText(colStr + Op2String);
        if (in == false)
        {
            SecondOpComboBox_Exit();
        }
    }
    checkQsoFrameColour();
}
bool QSOLogFrame::frameHasFocus()
{
    if (ui->CallsignFrame->getTextEditEdit()->hasFocus()
            || ui->RSTTxFrame->getTextEditEdit()->hasFocus()
            || ui->SerTxFrame->getTextEditEdit()->hasFocus()
            || ui->RSTRxFrame->getTextEditEdit()->hasFocus()
            || ui->SerRxFrame->getTextEditEdit()->hasFocus()
            || ui->LocFrame->getTextEditEdit()->hasFocus()
            || ui->QTHFrame->getTextEditEdit()->hasFocus()
            || ui->commentsFrame->getTextEditEdit()->hasFocus()
            )
    {
        return true;
    }

    return false;

}
void QSOLogFrame::setAsEdit(bool s, QString b)
{
    baseName = b;
    if (s)
    {
        edit = true;
        ui->GJVCancelButton->setText(tr("Return to Log"));
        on_tabSandP();
        ui->SerTxFrame->getTextEditEdit()->setReadOnly(false);
        ui->SerTxFrame->getTextEditEdit()->setFocusPolicy(Qt::StrongFocus);
    }
}

void QSOLogFrame::setContest(BaseContestLog *pcontest)
{
    catchup = false;
    contest = pcontest;
    screenContact.initialise( contest ); // get ops etc correct
    setXferEnabled(false, contest, "Log");

    if (!pcontest)
    {
        return;
    }

    updateQSODisplay();
    refreshOps();
    MinosLoggerEvents::SendReportOverstrike(overstrike, contest);
}
void QSOLogFrame::initialise()
{

    csIl = new ValidatedControl( ui->CallsignFrame->getTextEditEdit(), vtCallsign );
    vcs.push_back( csIl );
    rsIl = new ValidatedControl( ui->RSTTxFrame->getTextEditEdit(), vtRST );
    vcs.push_back( rsIl );
    ssIl = new ValidatedControl( ui->SerTxFrame->getTextEditEdit(), vtSN );
    vcs.push_back( ssIl );
    rrIl = new ValidatedControl( ui->RSTRxFrame->getTextEditEdit(), vtRST );
    vcs.push_back( rrIl );
    srIl = new ValidatedControl( ui->SerRxFrame->getTextEditEdit(), vtSN );
    vcs.push_back( srIl );
    locIl = new ValidatedControl( ui->LocFrame->getTextEditEdit(), vtLoc );
    vcs.push_back( locIl );
    qthIl = new ValidatedControl( ui->QTHFrame->getTextEditEdit(), vtQTH );
    vcs.push_back( qthIl );
    cmntIl = new ValidatedControl( ui->commentsFrame->getTextEditEdit(), vtComments );
    vcs.push_back( cmntIl );

    if (edit)
    {
        freqIl = new ValidatedControl(ui->frequencyEdit, vtFreq );
        vcs.push_back( freqIl );
    }
    else
    {
        freqIl = nullptr;
    }

    ui->BrgSt->clear();
    ui->DistSt->clear();

    setTimeStyles();

    if (edit)
    {
        ui->radioDetailsFrame->setVisible(true);
        ui->EditFrame->setVisible(true);
        ui->SerTxFrame->getTextEditEdit()->setStyleSheet(ssLineEditOK);
        widgetStyles[ui->SerTxFrame->getTextEditEdit()] = ssLineEditOK;
    }
    else
    {
        ui->radioDetailsFrame->setVisible(false);
        ui->EditFrame->setVisible(false);
        ui->SerTxFrame->getTextEditEdit()->setStyleSheet(ssLineEditGreyBackground);
        widgetStyles[ui->SerTxFrame->getTextEditEdit()] = ssLineEditGreyBackground;
    }

    current = nullptr;
    oldTimeOK = true;
    connect(&MinosLoggerEvents::mle, SIGNAL(TimerDistribution()), this, SLOT(on_TimeDisplayTimer()));


    connect(ui->bandmapMarkFreqPb, SIGNAL(clicked()), this, SLOT(on_BandmapMarkFreqPbClicked()));
    connect(ui->bandmapSaveFreqPb, SIGNAL(clicked()), this, SLOT(on_bandmapSaveFreqPbClicked()));

    connect(ui->spotPb, SIGNAL(clicked()), this, SLOT(on_SpotPbClicked()));
    connect(ui->spotLastLoggedPb, SIGNAL(clicked()), this, SLOT(on_SpotLastLoggedPbClicked()));
    setClusterSendSpotControlsVisible(false);           // visibility controlled by txenable in clusterserver

    connect(this, SIGNAL(freqChanged(Frequency)), this, SLOT(on_FreqChanged(Frequency)));



}
void QSOLogFrame::setTimeStyles()
{
    if (!edit)
    {
        if (catchup)
        {
            ui->dateEdit->setEnabled(true);
            ui->timeEdit->setEnabled(true);

            ui->dateEdit->setStyleSheet(ssDtgWhite);
            ui->timeEdit->setStyleSheet(ssDtgWhite);
            widgetStyles[ui->dateEdit] = ssDtgWhite;
            widgetStyles[ui->timeEdit] = ssDtgWhite;

            ui->dateEdit->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
            ui->timeEdit->setButtonSymbols(QAbstractSpinBox::UpDownArrows);

            setDtgSection();
        }
        else
        {
            ui->dateEdit->setEnabled(false);
            ui->timeEdit->setEnabled(false);

            ui->dateEdit->setStyleSheet(ssDtgWhiteNoFrame);
            ui->timeEdit->setStyleSheet(ssDtgWhiteNoFrame);
            widgetStyles[ui->dateEdit] = ssDtgWhiteNoFrame;
            widgetStyles[ui->timeEdit] = ssDtgWhiteNoFrame;

            ui->dateEdit->setButtonSymbols(QAbstractSpinBox::NoButtons);
            ui->timeEdit->setButtonSymbols(QAbstractSpinBox::NoButtons);

            oldTimeOK = true;
        }
    }
    else
    {
        if (!unfilled)
        {
            ui->timeEdit->setEnabled(false);
            ui->dateEdit->setEnabled(false);
            ui->dateEdit->setButtonSymbols(QAbstractSpinBox::NoButtons);
            ui->timeEdit->setButtonSymbols(QAbstractSpinBox::NoButtons);
        }
        else
        {
            setDtgSection();
        }

    }
}

void QSOLogFrame::setXferEnabled(bool s, BaseContestLog *c, QString b)
{
    if (contest == c && b == baseName)
    {
        ui->MatchXferButton->setEnabled(s);
        QString ss;
        if (s)
            ss = ssRed;

        ui->MatchXferButton->setStyleSheet(ss);
        widgetStyles[ui->MatchXferButton] = ss;
    }
}

void QSOLogFrame::on_CatchupButton_clicked()
{
    if (catchup)
    {
        catchup = false;
        ui->CatchupButton->setStyleSheet("");
        widgetStyles[ui->CatchupButton] = "";
        ui->CatchupButton->setText(tr("Catch-up (Post Entry)"));

        oldTimeOK = true;
    }
    else
    {
        catchup = true;
        ui->CatchupButton->setStyleSheet("background-color : coral;");
        widgetStyles[ui->CatchupButton] = "background-color : coral;";
        ui->CatchupButton->setText(tr("End Catch-up"));

    }
    // set the screencontact dtg as not entered
    screenContact.time.setDate(QString(), DTGLOG);
    screenContact.time.setTime(QString(), DTGLOG);
    setTimeStyles();
    sortUnfilledCatchupTime();
    selectField( nullptr );
}

void QSOLogFrame::on_FirstUnfilledButton_clicked()
{
    // Go to the first unfilled QSO
    // If there aren't any then it needs to be made invisible
    // ScanContest can work out how many there are - and we can display that on the button
       MinosLoggerEvents::SendNextUnfilled(contest);
}
void QSOLogFrame::setFirstUnfilledButtonEnabled(bool state)
{
    ui->FirstUnfilledButton->setEnabled(state);
}

void QSOLogFrame::MainOpComboBox_Exit()
{
    if (contest)
     {
        QString op1 = ui->MainOpComboBox->currentText();
        if (!edit)
        {
            contest->currentOp1.setValue( op1 );
        }
        if ( op1.size() )
        {
           contest->oplist.insert( op1, op1 );
        }
        contest->commonSave(false);
        refreshOps();
     }
}

void QSOLogFrame::SecondOpComboBox_Exit()
{
    if (contest)
    {
       QString op2 = ui->SecondOpComboBox->currentText();
       if (!edit)
       {
           contest->currentOp2.setValue( op2 );
       }
       if ( op2.size() )
       {
          contest->oplist.insert( op2, op2 );
       }
       contest->commonSave(false);
       refreshOps();
    }
}
void QSOLogFrame::on_GJVOKButton_clicked()
{
    if ( contest->isReadOnly() )
    {
       return;
    }
    ui->SerTxFrame->getTextEditEdit()->setReadOnly(!edit);
    ui->SerTxFrame->getTextEditEdit()->setFocusPolicy(edit?Qt::StrongFocus:Qt::ClickFocus);

    getScreenEntry(); // make sure it is saved

    if ( screenContact.contactFlags & ( LOCAL_COMMENT | DONT_PRINT | COMMENT_ONLY ) )
    {
        if ( !checkAndLogEntry() )  // if it is the same, then don't log
        {
           return;
        }
//       logCurrentContact( );
//       return;
    }
    // Do we want "call" tab order or "S and P" tab order?
    bool tabSandP;
    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpTabforSandP, tabSandP );

    if (tabSandP)
    {
        tabSandP = ui->SandPrb->isChecked();
    }

    // validate the entry; if still invalid, spin round the invalid
    // controls (this should really be the job of tab, but...)

    QWidget *currn = current;
    if ( !valid( cmCheckValid ) || ( currn == ui->RSTTxFrame->getTextEditEdit() ) || ( currn == ui->RSTRxFrame->getTextEditEdit() ) )
       // make sure all single and cross field
       // validation has been done
    {
       doAutofill();
    }
    bool was_unfilled = screenContact.contactFlags & TO_BE_ENTERED;
    if ( !valid( cmCheckValid ) )   // make sure all single and cross field
                                    // validation has been done
    {
       QWidget * firstInvalid = nullptr;
       QWidget *nextInvalid = nullptr;
       bool onCurrent = false;
       bool pastCurrent = false;
       for ( auto const &vcp: qAsConst(vcs) )
       {
          if ( !vcp ->wc->isVisible() || !vcp ->wc->isEnabled())
          {
             continue;
          }
          if ( onCurrent )
             pastCurrent = true;
          if ( vcp ->wc == current )
             onCurrent = true;
          if ( !vcp ->valid( cmValidStatus, screenContact ) )
          {
             if ( !firstInvalid )
                firstInvalid = vcp ->wc;
             if ( pastCurrent )
             {
                if ( !nextInvalid )
                {
                   nextInvalid = vcp->wc;
                   break;
                }
             }
          }
       }

       // make sure we go to the invalid field

       QWidget *nextf = ( nextInvalid ) ? nextInvalid : firstInvalid;

       if (tabSandP && edit == false && catchup == false)
       {
           if (current == ui->CallsignFrame->getTextEditEdit())
           {
               if (nextf == ui->RSTTxFrame->getTextEditEdit() && !rrIl->valid( cmValidStatus, screenContact ))
               {
                   nextf = ui->RSTRxFrame->getTextEditEdit();
               }
           }
       }

       // but if it is DTG, probably want CS instead (Unless post entry)

       if ( nextf )
       {
          if ( nextf == current )
          {
             if ( firstInvalid != nextf )
             {
                selectField( firstInvalid );
             }
             else
             {
                 if (dlgForced())              // repeated attack on same faulty field
                 {
                     emit QSOFrameCancelled();  // so edit dialog can close
                 }
             }
          }
          else
             selectField( nextf );
       }
       return;
    }
    else
    {
       screenContact.contactFlags &= ~( TO_BE_ENTERED | FORCE_LOG );
    }



    // all is OK (or we will have executed a return statement)
    // so do it!

    // we have to check if we need to save it
    // checkAndLogEntry does the log action as well

    if ( !was_unfilled && !catchup && selectedContact )  // AND if we are logging "current" then we don't want to do this
    {
       if ( !checkAndLogEntry() )  // if it is the same, then don't log
       {
          return;
       }
    }
    else
    {
       logCurrentContact( );
    }

    if (edit)
    {
        if (!was_unfilled)
        {
            emit QSOFrameCancelled();
        }
        else if (unfilled)
        {
           // If Uri mode then continue to the next...
           QSharedPointer<BaseContact> nuc = contest->findNextUnfilledContact( );
           if (nuc)
           {
               selectEntryForEdit(nuc);
               selectField( nullptr );             // make sure we move off the "Log" default button
           }
           else
           {
               emit QSOFrameCancelled();
           }
        }
    }
    else
    {
        sortUnfilledCatchupTime();
    }
    return;

}
bool QSOLogFrame::dlgForced()
{
    getScreenEntry();
    valid( cmCheckValid );       // This adds errors to the MAIN dialog error list, not our own

    TForceLogDlg ForceDlg(this);

    int res = ForceDlg.doexec(contest, screenContact, errs);
    if ( res == QDialog::Accepted )
    {
        // make sure marked on main screen

        ui->NonScoreCheckBox->setChecked(screenContact.contactFlags & NON_SCORING);
        ui->DeletedCheckBox->setChecked(screenContact.contactFlags & DONT_PRINT);
        if ( screenContact.contactFlags & NON_SCORING )
        {
            screenContact.multCount = 0;
            screenContact.bonus = 0;
        }

        // if no dtg then autofill dtg

        logCurrentContact( );
        return true;
    }
    return false;
}
//---------------------------------------------------------------------------
void QSOLogFrame::on_GJVForceButton_clicked()
{
    if ( contest->isReadOnly() )
    {
       return;
    }
    ui->SerTxFrame->getTextEditEdit()->setReadOnly(!edit);

    if (dlgForced())
        emit QSOFrameCancelled();

}
bool QSOLogFrame::savePartial()
{
   if ( !partialContact )
   {
      getScreenEntry();
      partialContact = new ScreenContact( );
      partialContact->copyFromArg( screenContact );
      return true;
   }
   return false;
}
bool QSOLogFrame::restorePartial( )
{
   if ( partialContact )
   {
      screenContact.copyFromArg( *( partialContact ) );
      showScreenEntry();

      killPartial();
      return true;
   }
   return false;
}
void QSOLogFrame::killPartial( )
{
   if ( partialContact )
   {
      delete partialContact;
      partialContact = nullptr;
   }
}

void QSOLogFrame::startNextEntry( )
{
    if (contest == nullptr)
    {
        return;
    }
    if (!contest)
        return;
   if (contest->unfilledCount <= 0 || contest->isReadOnly())
   {
      ui->FirstUnfilledButton->setVisible(false);
   }
   else
   {
      ui->FirstUnfilledButton->setVisible(true);
   }

   if ( !restorePartial() )
   {
       // no partial to restore
      screenContact.initialise( contest );
   }

   updateQSOTime();
   showScreenEntry();

   MinosLoggerEvents::SendAfterSelectContact(QSharedPointer<BaseContact>(), contest);
}
void QSOLogFrame::doGJVCancelButton_clicked()
{
    if (edit)
    {
        checkAndLogEntry();

        emit QSOFrameCancelled();

    }
    else
    {
        ui->SerTxFrame->getTextEditEdit()->setReadOnly(!edit);

        ScreenContact *temp = nullptr;
        if ( !partialContact )
        {
           savePartial();
           temp = partialContact;
           partialContact = nullptr;
        }
        startNextEntry();
        ui->CallsignFrame->getTextEditEdit()->setFocus();

        if ( temp )
           partialContact = temp;
    }
}

void QSOLogFrame::on_GJVCancelButton_clicked()
{
    doGJVCancelButton_clicked();
}

void QSOLogFrame::on_MatchXferButton_clicked()
{
    MinosLoggerEvents::sendXferPressed(contest, baseName);
}

void QSOLogFrame::onQTHEdit_textChanged(const QString &/*arg1*/)
{
    doGJVEditChange( ui->QTHFrame->getTextEditEdit() );
}

void QSOLogFrame::onCallsignEdit_textChanged(const QString &text)
{
    doGJVEditChange( ui->CallsignFrame->getTextEditEdit() );

    if (logDataFromBandmapOrMemory)
    {
       if (ui->CallsignFrame->getTextEditEdit()->text().isEmpty())
       {
           logDataFromBandmapOrMemory = false;
       }

    }

    if (text.count() > 0)
    {
        callsignEnterTextFreq = curFreq;
    }
    else
    {
        callsignEnterTextFreq.clear();
    }

}

void QSOLogFrame::onLocEdit_textChanged(const QString &/*arg1*/)
{
   // force bearing calc
   getScreenEntry();
   calcLoc();
   doGJVEditChange( ui->LocFrame->getTextEditEdit() );
}

void QSOLogFrame::onRSTTXEdit_textChanged(const QString &/*arg1*/)
{
    doGJVEditChange( ui->RSTTxFrame->getTextEditEdit() );
}

void QSOLogFrame::onRSTRXEdit_textChanged(const QString &/*arg1*/)
{
    doGJVEditChange( ui->RSTRxFrame->getTextEditEdit() );
}
void QSOLogFrame::onSerRXEdit_textChanged(const QString &/*arg1*/)
{
    doGJVEditChange( ui->SerRxFrame->getTextEditEdit() );
}

void QSOLogFrame::onSerTXEdit_textChanged(const QString &/*arg1*/)
{
    doGJVEditChange( ui->SerTxFrame->getTextEditEdit() );

}
void QSOLogFrame::on_frequencyEdit_textChanged(const QString &/*arg1*/)
{
    doGJVEditChange( ui->frequencyEdit );

}
void QSOLogFrame::do_mouseDoubleClickEvent(QObject *w)
{
    // Don't let the dtg be changed when the contest is protected
    if (contest->isReadOnly())
        return;

    if (w == ui->SerTxFrame->getTextEditEdit())
    {
        ui->SerTxFrame->getTextEditEdit()->setReadOnly(false);
    }
    if (edit && (w == ui->timeEdit || w == ui->dateEdit))
    {
        ui->timeEdit->setEnabled(true);
        ui->dateEdit->setEnabled(true);
        ui->dateEdit->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
        ui->timeEdit->setButtonSymbols(QAbstractSpinBox::UpDownArrows);

        if (w == ui->timeEdit)
            ui->timeEdit->setFocus();
        else
            ui->dateEdit->setFocus();

        setDtgSection();
    }
}



void QSOLogFrame::setActiveControl( int *Key )
{
   switch ( *Key )
   {
      case Qt::Key_F1:
         selectField( ui->CallsignFrame->getTextEditEdit() );
         *Key = 0;
         break;
      case Qt::Key_F2:
         selectField( ui->RSTTxFrame->getTextEditEdit() );
         *Key = 0;
         break;
      case Qt::Key_F3:
         selectField( ui->RSTRxFrame->getTextEditEdit() );
         *Key = 0;
         break;
      case Qt::Key_F4:
         selectField( ui->SerRxFrame->getTextEditEdit() );
         *Key = 0;
         break;
      case Qt::Key_F5:
         selectField( ui->LocFrame->getTextEditEdit() );
         *Key = 0;
         break;
      case Qt::Key_F6:
         selectField( ui->QTHFrame->getTextEditEdit() );
         *Key = 0;
         break;
   case Qt::Key_F12:
       MinosLoggerEvents::sendXferPressed(contest, baseName);
         *Key = 0;
         break;
   }
}
//---------------------------------------------------------------------------
void QSOLogFrame::getScreenEntry()
{
   getScreenContactTime();
   getScreenRigData();
   getscreenRotatorData();
   screenContact.cs.setFullCall( ui->CallsignFrame->getTextEditEdit()->text() );

   screenContact.reps = ui->RSTTxFrame->getTextEditEdit()->text().trimmed();
   screenContact.serials = ui->SerTxFrame->getTextEditEdit()->text().trimmed();
   screenContact.repr = ui->RSTRxFrame->getTextEditEdit()->text().trimmed();
   screenContact.serialr = ui->SerRxFrame->getTextEditEdit()->text().trimmed();

   QString loc = ui->LocFrame->getTextEditEdit()->text();
   screenContact.loc.setLoc( loc );

   QString extra = ui->QTHFrame->getTextEditEdit()->text().trimmed();
   screenContact.extraText = extra;

   QString comments = ui->commentsFrame->getTextEditEdit()->text().trimmed();
   screenContact.comments = comments;
   if (edit)
   {
       screenContact.rigName = ui->radioEdit->text().trimmed();

       QString f = ui->frequencyEdit->text().trimmed().remove( QRegularExpression("^[0]*")); //remove leading zeros
       f = convertSinglePeriodFreqToMultiPeriod(convertSinglePeriodFreqToFullDigit(f));
       screenContact.frequency = f;

       //screenContact.frequency = ui->frequencyEdit->text().trimmed();
       screenContact.rotatorHeading = ui->rotatorHeadingEdit->text().trimmed();

       screenContact.op1 = ui->MainOpComboBox->currentText();
       screenContact.op2 = ui->SecondOpComboBox->currentText();
   }
   else
   {
       screenContact.cqResponse = runButtonOnFlag && !radioOffRunFreq;
   }
   screenContact.mode = ui->ModeComboBoxGJV->currentText().trimmed();
   screenContact.mgmSubmode = ui->MGMSubModeEdit->text().trimmed();
   screenContact.contactFlags &= ~NON_SCORING;

   // op1/op2 get set when the attached combos change - I hope :)

   if ( ui->NonScoreCheckBox->isChecked() )
   {
      screenContact.contactFlags |= NON_SCORING;
   }
   screenContact.contactFlags &= ~DONT_PRINT;
   if ( ui->DeletedCheckBox->isChecked() )
   {
      screenContact.contactFlags |= DONT_PRINT;
   }
}
//---------------------------------------------------------------------------
void QSOLogFrame::logTabChanged()
{
    MinosLoggerEvents::SendScreenContactChanged(&screenContact, contest, baseName);
}
void QSOLogFrame::showScreenEntry( )
{
   // display the contents of the contest->screenContact

   if ( contest )
   {
      // we only validate this contact up to the validation point
       ui->SerTxFrame->getTextEditEdit()->setReadOnly(!edit);

      contest->validationPoint = selectedContact?selectedContact->getLogSequence():0;
      ScreenContact temp;
      temp.copyFromArg( screenContact ); // as screen contact gets corrupted by auto changes
      // op1, op2 in ScreenContact get corrupted as well
      showScreenContactTime();
      ui->CallsignFrame->getTextEditEdit()->setText(temp.cs.getFullCall());
      ui->RSTTxFrame->getTextEditEdit()->setText(temp.reps.trimmed());
      ui->SerTxFrame->getTextEditEdit()->setText(temp.serials.trimmed());
      ui->RSTRxFrame->getTextEditEdit()->setText(temp.repr.trimmed());
      ui->SerRxFrame->getTextEditEdit()->setText(temp.serialr.trimmed());
      ui->LocFrame->getTextEditEdit()->setText(temp.loc.getLoc());  // also forces update of score etc
      ui->QTHFrame->getTextEditEdit()->setText(temp.extraText.trimmed());
      ui->commentsFrame->getTextEditEdit()->setText(temp.comments.trimmed());
      ui->NonScoreCheckBox->setChecked(temp.contactFlags & NON_SCORING);
      ui->DeletedCheckBox->setChecked(temp.contactFlags & DONT_PRINT);
      if (edit)
      {
          ui->radioEdit->setText(temp.rigName);

          ui->frequencyEdit->setText(temp.frequency.convertFreqStrDispSingle());
          ui->rotatorHeadingEdit->setText(temp.rotatorHeading);
      }


      if (mode.isEmpty()) // use contest mode
      {
        setMode(temp.mode.trimmed());
      }
      else
      {
          setMode(mode);    // use mode set in the frame
      }

      ui->MGMSubModeEdit->setText(temp.mgmSubmode);

      // and now we want to put the selection on each at the END of the text
      for ( auto const &vcp: qAsConst(vcs) )
      {
         int selpt = vcp->wc->text().length();
         vcp->wc ->setSelection(selpt, 0);
      }

      valid( cmCheckValid ); // make sure contact is valid - display any errors
      if ( temp.contactFlags & ( COMMENT_ONLY | LOCAL_COMMENT ) )
      {
         selectField( ui->commentsFrame->getTextEditEdit() );
      }
      else
         selectField( nullptr );

      MinosLoggerEvents::SendScreenContactChanged(&screenContact, contest, baseName);
   }
}
//---------------------------------------------------------------------------
void QSOLogFrame::EditControlEnter(QObject *Sender , QFocusEvent *event)
{
   current = dynamic_cast<QWidget *>(Sender);
   QLineEdit *tle = dynamic_cast<QLineEdit *>(Sender);

   if (tle && (event->reason() == Qt::TabFocusReason || event->reason() == Qt::BacktabFocusReason) )
   {
       MinosLoggerEvents::SendAfterTabFocusIn(tle);
   }
}
void QSOLogFrame::on_AfterTabFocusIn(QLineEdit *tle)
{
    int endpos = tle->text().size();
    if (endpos < 0)
    {
        endpos = 0;
    }
    tle->setCursorPosition(endpos);
    tle->deselect();

    bool ovr = overstrike;
    MinosLoggerEvents::SendReportOverstrike(ovr, contest);
}

//---------------------------------------------------------------------------

void QSOLogFrame::EditControlExit( QObject * /*Sender*/ )
{
   if (!contest)
   {
      return;
   }
   ui->SerTxFrame->getTextEditEdit()->setReadOnly(!edit);

   if ( current == ui->LocFrame->getTextEditEdit() )
   {
      // do any required character substitutions, but only when we have a full
      // locator

      // This does something to correct sloppy typing

      QString loc = ui->LocFrame->getTextEditEdit()->text().trimmed().toUpper();
      if (loc.size() == 6 || loc.size() == 8)
      {
         if (loc[0] == '1')
         {
            loc[0] = 'I';
         }
         if (loc[0] == '0')
         {
            loc[0] = '1';
         }

         if (loc[1] == '1')
         {
            loc[1] = 'I';
         }
         if (loc[1] == '0')
         {
            loc[1] = 'O';
         }

         if (loc[2] == 'I')
         {
            loc[2] = '1';
         }
         if (loc[2] == 'O')
         {
            loc[2] = '0';
         }

         if (loc[3] == 'I')
         {
            loc[3] = '1';
         }
         if (loc[3] == 'O')
         {
            loc[3] = '0';
         }

         if (loc[4] == '1')
         {
            loc[4] = 'I';
         }
         if (loc[4] == '0')
         {
            loc[4] = 'O';
         }

         if (loc[5] == '1')
         {
            loc[5] = 'I';
         }
         if (loc[5] == '0')
         {
            loc[5] = 'O';
         }
         if (loc != ui->LocFrame->getTextEditEdit()->text().trimmed())
         {
            ui->LocFrame->getTextEditEdit()->setText(loc);
            doGJVEditChange( ui->LocFrame->getTextEditEdit() );   // start the match thread
         }
      }
   }
   if ( ( current == ui->CallsignFrame->getTextEditEdit() ) || ( current == ui->LocFrame->getTextEditEdit() )
        || ( current == ui->SerRxFrame->getTextEditEdit() ) || ( current == ui->SerTxFrame->getTextEditEdit() )
        || ( current == ui->frequencyEdit ))
   {
      getScreenEntry(); // make sure it is saved
      valid( cmCheckValid ); // make sure all single and cross field
      doAutofill();           // should only be time to be filled

      if (current == ui->CallsignFrame->getTextEditEdit())
      {
          MinosLoggerEvents::sendCallsignLookup(contest, ui->CallsignFrame->getTextEditEdit()->text());
      }
   }
}
    //---------------------------------------------------------------------------
void QSOLogFrame::setScoreText( int dist, bool partial, bool xband )
{
   QString b;
   QString s;
   s =  QString::number(dist );

   if ( partial )
      b += "(";
   b += s;
   if ( partial )
      b += ")";
   if ( xband )
      b += "X";
   b += "  ";
   ui->DistSt->setText(b);
}
//---------------------------------------------------------------------------
void QSOLogFrame::calcLoc( )
{
    QString gridref = ui->LocFrame->getTextEditEdit()->text().trimmed();

    if ( gridref.compare(oldloc, Qt::CaseInsensitive ) != 0 )
    {
        oldloc = gridref;
        locValid = true;
        double latitude;
        double longitude;
        double dist;
        int brg = 0;

        int locValres = lonlat( gridref, longitude, latitude, MinosParameters::getMinosParameters() ->getAllowLoc4() );
        if ( ( locValres ) != LOC_OK )
            locValid = false;

        ScreenContact &sct = screenContact;
        if ( ( sct.contactFlags & MANUAL_SCORE ) &&
             !( sct.contactFlags & DONT_PRINT ) )
        {
            int thisscore = sct.contactScore;
            setScoreText( thisscore, false, sct.contactFlags & XBAND );
            ui->BrgSt->setText("MANUAL");

        }
        else
            if ( ( locValid
                   || ( locValres == LOC_PARTIAL ) )
                 && contest->locValid )
            {
                if (contest->MGMContestRules.getValue())
                {
                    dist = contest->CalcCentres ( gridref, brg );
                    if ( almost_equal(dist, 1.0, 2))
                        dist = 50;  // MGM same square == 50 points
                }
                else
                {
                    contest->disbeara( longitude, latitude, dist, brg );
                }

                if ( locValid )  	// just 4CHAR not enough
                {
                    sct.bearing = brg;

                    if ( !( sct.contactFlags & ( MANUAL_SCORE | NON_SCORING | LOCAL_COMMENT | COMMENT_ONLY | DONT_PRINT ) ) )
                    {
                        sct.contactScore = static_cast<int>(dist);
                    }

                }

                int offset = contest->bearingOffset.getValue();

                QString rev;
                if (offset)
                {
                    rev += "O";
                }
                rev += ( MinosParameters::getMinosParameters() ->getMagneticVariation() ) ? "M" : "T" ;
                int vb = varBrg( brg + offset);
                if ( TContestApp::getContestApp() ->reverseBearing )
                {
                    vb = normBrg( vb - 180 );
                    rev += "R";
                }
                setScoreText( static_cast< int> ( dist), ( locValres == LOC_PARTIAL ), sct.contactFlags & XBAND );
                QString brgbuff;
                const QChar degreeChar(0260); // octal value
                if ( locValres == LOC_PARTIAL )
                {
                    brgbuff = QString( "(%1%2%3)").arg(vb).arg(degreeChar).arg(rev );
                }
                else
                {
                    brgbuff = QString( "%1%2%3").arg(vb).arg(degreeChar).arg(rev );
                }
                ui->BrgSt->setText(brgbuff);
                MinosLoggerEvents::SendBrgStrToRot(brgbuff);
            }
            else
            {
                ui->DistSt->clear();
                ui->BrgSt->clear();
                sct.contactScore = -1;
                sct.bearing = -1;
                MinosLoggerEvents::SendBrgStrToRot("");
            }
    }
}
//---------------------------------------------------------------------------
bool QSOLogFrame::validateControls( validTypes command )   // do control validation
{
   // spin round all controls, and validate them
   // return true if all valid

    bool ret = true;

    for ( auto const &vcp: qAsConst(vcs) )
   {
        QString ss = ssLineEditOK;
        if (!edit && vcp == ssIl)
            ss = ssLineEditGreyBackground;

        if (
                (
                    vcp->wc->isEnabled()
                    || (vcp == ssIl && contest->serialMandatoryField.getValue())
                    )
                && (!vcp ->valid( command, screenContact ) )
            )
        {
            if (!vcp->tIfValid)
            {
                QString text = vcp->wc->text().trimmed();
                if (!text.isEmpty())
                {
                    if (vcp == csIl)
                    {
                        if ( screenContact.cs.getValRes() == ERR_DUPCS)
                        {
                            ss = ssLineEditFrRedBkRed;
                        }
                        else
                        {
                            ss = ssLineEditFrRedBkWhite;
                        }
                    }
                    else if (vcp == rsIl && text == "5")
                    {
                        // leave as no error
                    }
                    else if (vcp == rrIl && text == "5")
                    {
                        // leave as no error
                    }
                    else if (vcp == locIl)
                    {
                        // leave as no error
                        if (screenContact.loc.getValRes() == ERR_LOC_RANGE && screenContact.loc.getLoc().size() > 4)
                        {
                            ss = ssLineEditFrRedBkRed;
                        }
                        else if (screenContact.loc.getValRes() != LOC_OK)
                        {
                            ss = ssLineEditFrRedBkWhite;
                        }
                    }
                    else if (vcp == qthIl)
                    {
                        // leave as no error except if exchange is wrong??
                    }
                    else if (vcp == srIl)
                    {
                        if (QApplication::focusWidget () == vcp->wc)
                        {
                            QString t = vcp->wc->text();
                            int val = toInt(t, -1);
                            if (val == -1)
                            {
                                ss = ssLineEditFrRedBkRed;
                            }
                            else if (val == 0)
                            {
                                ss = ssLineEditFrRedBkWhite;
                            }
                        }
                        else
                        {
                            ss = ssLineEditFrRedBkRed;
                        }
                    }
                    else
                    {
                        ss = ssLineEditFrRedBkRed;
                    }
                }
                ret = false;
            }
        }

        if (vcp->wc->isEnabled())
        {
            vcp->wc->setStyleSheet(ss);
            widgetStyles[vcp->wc] = ss;
        }
        else
        {
            QString ss = QString("[readOnly=\"true\"] { background-color: %0 }").arg(qApp->palette().color(QPalette::Window).name(QColor::HexRgb));
            vcp->wc->setStyleSheet(ss);
            widgetStyles[vcp->wc] = ss;
        }
   }
   return ret;
}
//---------------------------------------------------------------------------
bool QSOLogFrame::valid( validTypes command )
{
   lgTraceerr(-1); // clear the error list

   if ( contest->isReadOnly() )
      return true;

   bool pvalid = validateControls( command ); // do control validation

   if ( command == cmCheckValid )   // our own command!
      contactValid();

   // re-validate, as we may have changed things
   pvalid = validateControls( cmValidStatus ); // look at current validity

   return pvalid;
}

ScreenContact *QSOLogFrame::getPartialContact() const
{
    return partialContact;
}

void QSOLogFrame::setPartialContact(ScreenContact *value)
{
    partialContact = value;
}
//---------------------------------------------------------------------------
void QSOLogFrame::selectField( QWidget *v )
{
    if ( v == nullptr )
    {
        v = ui->CallsignFrame->getTextEditEdit();

        if ( catchup || screenContact.contactFlags & TO_BE_ENTERED )
        {
            v = ui->timeEdit;
        }
    }
    setDtgSection();

    if ( current == v )
    {
        v->setFocus();
        return ;
    }

    if ( ( current == ui->CallsignFrame->getTextEditEdit() ) || ( current == ui->LocFrame->getTextEditEdit() )
         || ( current == ui->SerRxFrame->getTextEditEdit() ) || ( current == ui->SerTxFrame->getTextEditEdit() )
         || (current == ui->frequencyEdit)
         )
    {
        valid( cmCheckValid ); // make sure all single and cross field
        doAutofill();
    }
    if ( v == ui->SerTxFrame->getTextEditEdit() )
    {
        ( static_cast< QLineEdit * > (v) ) ->setReadOnly(false);
    }
    if ( v->isEnabled() )
    {
        v->setFocus();
        current = v;
    }
}
//==============================================================================
// check for embedded space or empty number

void QSOLogFrame::fillRst( QLineEdit *rIl, QString &rep, const QString &fmode )
{
   QString newrep;
   if ( current == rIl )
   {
      bool autoFill;
      TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpAutoFill, autoFill );
      if (autoFill)
      {
         if ( !Validator::validateRST( rIl->text().trimmed() ) )
         {
            if ( ( rep.size() == 1 ) && rep[ 0 ].isNumber() )
            {
               newrep = rep[ 0 ];
               newrep += "9";
               rIl->setText(newrep);
               rep = newrep;
            }
         }
      }
      if ( ( fmode.compare( hamlibData::CW, Qt::CaseInsensitive ) == 0 ) && Validator::validateRST( rIl->text().trimmed() )
           && ( rep.size() == 2 ) )
      {
         newrep = rep[ 0 ];
         newrep += rep[ 1 ];
         newrep += "9";
         rIl->setText(newrep);
         rep = newrep;
      }
   }
}
//==============================================================================
void QSOLogFrame::doAutofill()
{
   if ( contest->isReadOnly() )
      return ;
   ScreenContact *vcct = &screenContact;

   //rst sent (autofill S9)

   fillRst( ui->RSTTxFrame->getTextEditEdit(), vcct->reps, vcct->mode );
   fillRst( ui->RSTRxFrame->getTextEditEdit(), vcct->repr, vcct->mode );
}
//==============================================================================
void QSOLogFrame::lgTraceerr( int err )
{
   MinosLoggerEvents::SendValidateError(err);
}
//==============================================================================
void QSOLogFrame::contactValid( )
{
   // this is where we need to do all of our cross field validation

   getScreenEntry();
   ScreenContact *vcct = &screenContact;

   if ( vcct->contactFlags & DONT_PRINT )
   {
      lgTraceerr(-1);
      lgTraceerr( ERR_26 );
   }
   else
   {
      if ( vcct->contactFlags & ( LOCAL_COMMENT | COMMENT_ONLY ) )
         lgTraceerr( ERR_25 );
      if ( vcct->contactFlags & NON_SCORING )
         lgTraceerr( ERR_24 );
   }
   if ( vcct->contactFlags & ( LOCAL_COMMENT | COMMENT_ONLY | NON_SCORING | DONT_PRINT ) )
   {
      return ;
   }
   // we only validate this contact up to the validation point of the contest
   contest->validationPoint = selectedContact?selectedContact->getLogSequence():0 ;

   contest->DupSheet.clearCurDup();
//   int csret = vcct->cs.reValidate();
   int csret = vcct->cs.getValRes();
   if ( csret == CS_OK )
   {
      if ( contest->DupSheet.checkCurDup( vcct, contest->validationPoint, false ) )
      {
         if ( contest->DupSheet.isCurDup( vcct ) )      // But vcct is screen contact... so it won't be curdup
         {
            vcct->cs.setValRes(ERR_DUPCS);
            csret = ERR_DUPCS;
         }
      }
   }
   if ( csret != CS_OK )
   {
      csIl->tIfValid = false;
      switch ( csret )
      {
         case CS_NOT_VALIDATED:
            lgTraceerr( ERR_10 );
            break;

         case ERR_NOCS:
            lgTraceerr( ERR_11 );
            break;

         case ERR_DUPCS:
            lgTraceerr( ERR_12 );
            break;

         default:
            lgTraceerr( ERR_13 );
            break;
      }
   }

   // locator received

   int locrep = vcct->loc.getValRes();
   if ( locrep != LOC_OK )
   {
      if ( contest->locatorMandatoryField.getValue() )
         locIl->tIfValid = false;
      if ( contest->locatorMandatoryField.getValue() && ( locrep == ERR_NOLOC ) )
      {
         lgTraceerr( ERR_18 );
      }
      else
         if ( locrep == ERR_LOC_RANGE )
         {
            lgTraceerr( ERR_19 );
         }
   }

   // If multiplier ContestLog, multiplier (e.g. district if needed)
   // check anyway, this keeps country and loc charts up to date

   vcct->checkScreenContact( );  // QSOLogFrame::contactValid, check multiplier, don't log it yet!

   if (vcct->ctryMult)
   {

   // and look up in squares list for country
   // look for square against main prefix in LocSquares.ini

      QString sloc = vcct->loc.getLoc().left(4);
      if (sloc.size())
      {
         bool LocOK;

         QString prefix = vcct->ctryMult->getBasePrefix();
         TContestApp::getContestApp() ->locsBundle.openSection(prefix);
         if (TContestApp::getContestApp() ->locsBundle.isCurrSectionPresent() )
         {
            TContestApp::getContestApp() ->locsBundle.getBoolProfile( sloc, LocOK, false );
            if (!LocOK)
            {
               lgTraceerr( ERR_15 );
               locIl->tIfValid = false;
            }
         }
      }
   }
   if ( contest->districtMult.getValue() && !vcct->screenQSOValid )
   {
      // no district when required
      // No CS means we should go to QTH, as its likely to be needed
      if ( csret == ERR_NOCS || ( vcct->ctryMult && vcct->ctryMult->hasDistricts() && !vcct->districtMult ) )
      {
         lgTraceerr( ERR_20 );    // "Invalid district multiplier"
         qthIl->tIfValid = false;
      }
      // What do we allow as valid? Should we error wrong postcode/country combination?
   }
   else
      if ( contest->otherExchange .getValue() && !contest->districtMult.getValue() && ( vcct->extraText.trimmed().size() == 0 ) )
      {
         // no QTH info if required

         lgTraceerr( ERR_21 );            // QTH required
         qthIl->tIfValid = false;
      }
}

//---------------------------------------------------------------------------
bool QSOLogFrame::checkAndLogEntry()
{

   if ( contest->isReadOnly() )
      return true;

   // check if the screen contact and selected log contact differ
   bool retval = true;
   getScreenEntry();
   QSharedPointer<BaseContact> sct = selectedContact ;
   if ( sct->ne( screenContact ) )
   {
      bool mresp = true;

      // Dont check with op if not entered, and e.g. ESC pressed
      // Also allows for partial saving when in Uri mode
      if ( !( screenContact.contactFlags & TO_BE_ENTERED ) && !catchup )
      {
         mresp = mShowYesNoMessage( this,
                             tr("This Contact has changed: Shall I log the changes?\n"
                             "\n"
                             "Yes         - Log as shown\n"
                             "No          - Discard changes")
                              );
      }
      if ( mresp )
      {
         //Yes - log and continue
         logScreenEntry( );
         retval = true;
      }
      else
      {
         //Cancel - Discard changes, continue action
         screenContact.copyFromArg( selectedContact );  // we have to ACTUALLY revert, as the action may not conmplete
         showScreenEntry();
         retval = false;	// stay where we are
      }
   }
   return retval;
}
//---------------------------------------------------------------------------
void QSOLogFrame::setMode(QString m)
{

    QString myOldMode = ui->ModeComboBoxGJV->currentText();

    if (myOldMode == m)
        return;         // all is OK

    ui->ModeComboBoxGJV->setCurrentText(m);
    mode = m;
    oldMode = myOldMode;


    // make sure the mode button shows the correct "flip" value


   if (ui->ModeComboBoxGJV->currentText() == hamlibData::CW || ui->ModeComboBoxGJV->currentText() == hamlibData::MGM)
   {
      ui->ModeButton->setText(oldMode);
   }
   else
   {
      ui->ModeButton->setText(hamlibData::CW);
   }

    ui->MGMSubModeFrame->setVisible(ui->ModeComboBoxGJV->currentText() == hamlibData::MGM);
}
//---------------------------------------------------------------------------
void QSOLogFrame::setFreq(Frequency f)
{
    if (curFreq != f)
    {
        curFreq = f;
        emit freqChanged(f);

    }
}
void QSOLogFrame::sendFreq(Frequency f)
{
    emit sendFreqControl(f);
}
//---------------------------------------------------------------------------
void QSOLogFrame::setRadioName(QString radioName)
{
    curRadioName = radioName;
}
//---------------------------------------------------------------------------
QString QSOLogFrame::getRadioName()
{
    return curRadioName;
}
//---------------------------------------------------------------------------
void QSOLogFrame::setRadioState(QString s)
{
    if (s != "")
    {
        if (s == RIG_STATUS_CONNECTED)
        {
            radioConnected = true;
        }
        else if (s == RIG_STATUS_DISCONNECTED)
        {
           radioConnected = false;
           radioError = false;
        }
        else if (s == RIG_STATUS_ERROR)
        {
           radioError = true;
        }
    }
}
//---------------------------------------------------------------------------
void QSOLogFrame::setRotatorBearing(const QString &s)
{
    QStringList sl = s.split(':');
    if (sl.size() < 3)
        return;

    QString brg;
    //int len = bearing.length();
    int len = sl[0].length();
    if (len < 2)
    {
        brg = QString("%1%2")
        .arg("00").arg(sl[0]);
    }
    else if (len < 3)
    {
        brg = QString("%1%2")
        .arg("0").arg(sl[0]);
    }
    else
    {
        brg = sl[0];
    }


    if (curRotatorBearing != brg)
    {
        curRotatorBearing = brg;
    }
}

//---------------------------------------------------------------------------
void QSOLogFrame::clearCurrentField()
{
   current = nullptr;
}
//---------------------------------------------------------------------------
void QSOLogFrame::checkQsoFrameColour()
{
    if (!contest)
    {
        return;
    }
    QString ssQsoFrame = ssQsoFrameBlue;
    if (contest->isReadOnly())
    {
        ssQsoFrame = ssQsoFrameRed;
        if (contest->isUnwriteable())
        {
            ui->protectionLabel->setText(HtmlFontColour(Qt::red) + "<h3><b>  " + tr("Read Only"));
        }
        else if (contest->getProtectedState().getValue() && !contest->isProtectedSuppressed())
        {
            ui->protectionLabel->setText(HtmlFontColour(Qt::red) + "<h3><b>  " + tr("Protected"));
        }
        else if (contest->isAgeProtected())
        {
            ui->protectionLabel->setText(HtmlFontColour(Qt::red) + "<h3><b>  " + tr("Protected by age of contest"));
        }
    }
    else
    {
        if (!frameHasFocus())
        {
            ssQsoFrame = ssQsoFrameRed;
            ui->protectionLabel->setText(HtmlFontColour(Qt::red) + "<h3><b>  " + tr("QSO Entry Frame not focussed!"));
        }
        else
        {
            ui->protectionLabel->setText("");
        }
    }
    ui->qsoFrame->setStyleSheet(ssQsoFrame);
    widgetStyles[ui->qsoFrame] = ssQsoFrame;
 }

void QSOLogFrame::updateQSODisplay()
{
    if (contest == nullptr)
    {
        return;
    }
   if ( contest->districtMult.getValue() )
   {
//      ui->QTHEdit->CharCase = ecUpperCase;
   }
   else
   {
//      ui->QTHEdit->CharCase = ecNormal;
   }
   //CallsignEdit->Enabled = false; // leave this enabled in protected to allow searching
   bool notProtected = !contest->isReadOnly();
   ui->RSTTxFrame->setEnabled(notProtected && contest->RSTMandatoryField.getValue());
   ui->RSTTxFrame->setVisible(contest->RSTMandatoryField.getValue());
   ui->SerTxFrame->setEnabled(notProtected && contest->serialMandatoryField.getValue());
   ui->SerTxFrame->setVisible(contest->serialMandatoryField.getValue());
   ui->RSTRxFrame->setEnabled(notProtected && contest->RSTMandatoryField.getValue());
   ui->RSTRxFrame->setVisible(contest->RSTMandatoryField.getValue());
   ui->SerRxFrame->setEnabled(notProtected && contest->serialMandatoryField.getValue());
   ui->SerRxFrame->setVisible(contest->serialMandatoryField.getValue());
   ui->LocFrame->getTextEditEdit()->setEnabled(contest->locatorMandatoryField.getValue());  // loc remains enabled in protected to enable searching
   ui->LocFrame->setVisible(contest->locatorMandatoryField.getValue());
   bool exchangeNeeded = contest->otherExchange .getValue() || contest->districtMult.getValue() || contest->otherOptionalExchange.getValue();
   ui->QTHFrame->setEnabled( exchangeNeeded );
   ui->QTHFrame->setVisible(exchangeNeeded);
   ui->commentsFrame->setEnabled(notProtected);


   ui->ModeComboBoxGJV->setEnabled(notProtected);
   ui->MGMSubModeFrame->setEnabled(notProtected);
   ui->NonScoreCheckBox->setEnabled(notProtected);
   ui->DeletedCheckBox->setEnabled(notProtected);
   ui->GJVOKButton->setEnabled(notProtected);
   ui->GJVForceButton->setEnabled(notProtected);
   ui->radioEdit->setEnabled(notProtected);
   ui->frequencyEdit->setEnabled(notProtected);
   ui->rotatorHeadingEdit->setEnabled(notProtected);

   ui->ModeButton->setEnabled(notProtected);
   ui->SecondOpComboBox->setEnabled(notProtected);
   ui->MainOpComboBox->setEnabled(notProtected);

   ui->InsertBeforeButton->setEnabled(notProtected);
   ui->InsertAfterButton->setEnabled(notProtected);

   ui->CatchupButton->setEnabled(notProtected);
   ui->FirstUnfilledButton->setEnabled(notProtected);

   bool mgm = contest->MGMContestRules.getValue();

   ui->ModeComboBoxGJV->setEnabled(!mgm);
   ui->ModeButton->setEnabled(!mgm);

   checkQsoFrameColour();

   on_FontChanged();    // do all style sheets again
}

//---------------------------------------------------------------------------

void QSOLogFrame::refreshOps()
{
   // refill the op combo boxes from the current contest, and select the correct op

   if (contest)
   {

       QString mainOp = contest->currentOp1.getValue();
       QString secondOp = contest->currentOp2.getValue();

       ui->MainOpComboBox->clear();
       ui->SecondOpComboBox->clear();

       QStringList ops;
       for ( auto const &i: qAsConst(contest->oplist) )
       {
           if (!i.isEmpty())
             ops.append(i);
       }
       ops.append(mainOp);
       ops.append(secondOp);

       ops.append("");

       ops.sort();
       ops.removeDuplicates();

       ui->MainOpComboBox->addItems(ops);
       ui->SecondOpComboBox->addItems(ops);

       ui->MainOpComboBox->setCurrentText(mainOp);
       ui->SecondOpComboBox->setCurrentText(secondOp);

   }
}
void QSOLogFrame::refreshOps( ScreenContact &screenContact )
{
    if (contest)
    {
        QString mainOp = screenContact.op1;
        QString secondOp = screenContact.op2;

        ui->MainOpComboBox->clear();
        ui->SecondOpComboBox->clear();

        BaseContestLog * contest = TContestApp::getContestApp() ->getCurrentContest();
        QStringList ops;
        for ( auto const &i: qAsConst(contest->oplist ))
        {
            if (!i.isEmpty())
              ops.append(i);
        }
        ops.append(mainOp);
        ops.append(secondOp);

        ops.append("");

        ops.sort();
        ops.removeDuplicates();

        ui->MainOpComboBox->addItems(ops);
        ui->SecondOpComboBox->addItems(ops);

        ui->MainOpComboBox->setCurrentText(mainOp);
        ui->SecondOpComboBox->setCurrentText(secondOp);

       // and if this is the last contact, the ops should also propogate into the contest
       // for the NEXT contact
    }
}
void QSOLogFrame::on_ShowOperators ( )
{
   bool so = LogContainer->isShowOperators();
   ui->SecondOpComboBox->setVisible(so);
   ui->SecondOpLabel->setVisible(so);
   ui->MainOpComboBox->setVisible(so);
   ui->OperatorLabel->setVisible(so);
}
void QSOLogFrame::on_tabSandP()
{
    bool tabSandP;
    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpTabforSandP, tabSandP );

    if (edit || catchup)
    {
        tabSandP = false;
    }
    ui->tabSandPframe->setVisible(tabSandP);

    if (!tabSandP || (runButtonOnFlag && !radioOffRunFreq))
    {
        ui->callRb->setChecked(true);
    }
    else
    {
        ui->SandPrb->setChecked(true);
    }
}
//---------------------------------------------------------------------------
void QSOLogFrame::closeContest()
{
   BaseContestLog * currentContest = TContestApp::getContestApp() ->getCurrentContest();
   if (contest == currentContest)
   {
      ui->GJVCancelButton->setEnabled(true);
      ui->GJVCancelButton->setFocus();
   }
   contest = nullptr;
}
//---------------------------------------------------------------------------
void QSOLogFrame::doGJVEditChange( QObject *Sender )
{
   // sensitive field changed - trigger match scan
   if ( contest )
   {
      getScreenEntry();

      if ( current == ui->CallsignFrame->getTextEditEdit() || Sender == ui->CallsignFrame->getTextEditEdit() )
      {
         // clear the error list
         contest->DupSheet.clearCurDup();	// as edited, no longer a dup(?)

      }
      if ( current == ui->LocFrame->getTextEditEdit() || Sender == ui->LocFrame->getTextEditEdit() )
      {
         // force bearing calc
         calcLoc();
      }
      MinosLoggerEvents::SendScreenContactChanged(&screenContact, contest, baseName);
      valid( cmCheckValid ); // make sure all single and cross field

      // someone has changed one of the controls - which is the requirement for killing partial
      killPartial();
   }
}

void QSOLogFrame::on_ModeButton_clicked()
{
    if (isRadioLoaded() && radioConnected && !radioError)
    {
        qsoLogModeFlag = true;  // stop updates from rigcontrol
        // send mode change to radio
        emit sendModeControl(ui->ModeButton->text());
    }

    QString myOldMode = ui->ModeComboBoxGJV->currentText();
    ui->ModeComboBoxGJV->setCurrentText(ui->ModeButton->text());
    mode = ui->ModeButton->text();
    oldMode = myOldMode;
    ui->ModeButton->setText(oldMode);
    ui->MGMSubModeFrame->setVisible(ui->ModeComboBoxGJV->currentText() == hamlibData::MGM);
    EditControlExit(ui->ModeButton);
}

void QSOLogFrame::modeSentFromRig(QString m)
{
    if (qsoLogModeFlag)
    {
         qsoLogModeFlag = false;
         return;
    }
    QStringList mlist = m.split(':');
    if (mlist.length() != 2 )
    {
        return;
    }
    QString newMode = mlist[0];

    for (auto const &sm: supModeList)
    {
        if (newMode == sm)
        {
            oldMode = ui->ModeComboBoxGJV->currentText();
            if (newMode == ui->ModeComboBoxGJV->currentText())
            {
                return;
            }
            {
                // set index to new mode
                ui->ModeComboBoxGJV->setCurrentIndex(ui->ModeComboBoxGJV->findText( newMode));
                mode = newMode;
            }

            // ensure flip mode is shown on mode button
            if (ui->ModeComboBoxGJV->currentText() == hamlibData::CW || ui->ModeComboBoxGJV->currentText() == hamlibData::MGM)
            {
               ui->ModeButton->setText(oldMode);
            }
            else
            {
               ui->ModeButton->setText(hamlibData::CW);
            }
            ui->MGMSubModeFrame->setVisible(ui->ModeComboBoxGJV->currentText() == hamlibData::MGM);
            // finished..
            return;
        }
    }
}

void QSOLogFrame::logScreenEntry( )
{
   if (!contest || contest->isReadOnly() )
      return ;

   int ctmax = screenContact.serials.toInt();

   if ( ctmax > contest->maxSerial )
      contest->maxSerial = ctmax;

   LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( contest );
   if ( !ct )
   {
      return ;
   }
   QSharedPointer<BaseContact> lct = selectedContact;
   if (!lct)
   {
        lct = ct->addContact( ctmax, 0, false, false, screenContact.mode, screenContact.mgmSubmode, dtg(true) );	// "current" doesn't get flag, don't save ContestLog yet
   }

   if ( screenContact.mode.compare( hamlibData::MGM, Qt::CaseInsensitive ) != 0 )
   {
       bool contactmodeCW = ( screenContact.reps.size() == 3 && screenContact.repr.size() == 3 );
       bool curmodeCW = ( screenContact.mode.compare( hamlibData::CW, Qt::CaseInsensitive ) == 0 );

       if ( !edit && contactmodeCW != curmodeCW )
       {
          // ask if change...
          if ( !curmodeCW )
          {
             if ( MinosParameters::getMinosParameters() ->yesNoMessage( this, tr("Change mode to CW?") ) )
             {
                screenContact.mode = hamlibData::CW;
             }
          }
          else
          {
             if ( MinosParameters::getMinosParameters() ->yesNoMessage( this, tr("Change mode to USB?") ) )
             {
                screenContact.mode = hamlibData::USB;
             }
          }
       }
   }
   ct->currentMode.setValue( screenContact.mode );
   screenContact.op1 = ct->currentOp1.getValue() ;
   screenContact.op2 = ct->currentOp2.getValue();

   lct->copyFromArg( screenContact );
   lct->time.setDirty(); // As we may have created the contact with the same time as the screen contact
                         // This then becomes "not dirty", so we end up not saving the dtg.
                         // But this only happens when seconds are :00, as the main log
                         // is only to a minute resolution

   lct->commonSave(lct);
   ct->commonSave(false);

   killPartial();

   MinosLoggerEvents::SendAfterLogContact(ct);
   MinosLoggerEvents::SendAfterLogContactToCluster(ct, lct->cs, lct->loc.getLoc());

   MinosLoggerEvents::SendAfterLogContactToBandmap(ct, lct );

   // save for send spot to DX cluster
   lastLoggedCallsign = lct->cs;
   ui->spotLastLoggedPb->setText(tr("Spot Last Logged (%1) ").arg(lct->cs.getFullCall()));
   lastLoggedLocator = lct->loc.getLoc();
   lastLoggedFreq = lct->frequency.getValue();


   if (!edit )
        startNextEntry( );	// select the "next"
}
//---------------------------------------------------------------------------
void QSOLogFrame::getScreenContactTime()
{
   updateQSOTime();
   screenContact.time.setDate( ui->dateEdit->date() );
   screenContact.time.setTime( ui->timeEdit->time() );
}
//---------------------------------------------------------------------------
void QSOLogFrame::showScreenContactTime()
{
   ui->dateEdit->setDate(screenContact.time.getDate( ));
   ui->timeEdit->setTime(screenContact.time.getTime( ));

   setDtgSection();
}
void QSOLogFrame::getScreenRigData()
{
    if (!edit && !catchup && isRadioLoaded())
    {
        screenContact.rigName = curRadioName;
        screenContact.frequency = curFreq;
    }
    else
    {
        screenContact.rigName.clear();
        screenContact.frequency.clear();
    }
}
void QSOLogFrame::getscreenRotatorData()
{
    if (!edit && !catchup && isRotatorLoaded())
    {
        screenContact.rotatorHeading = curRotatorBearing;
    }
    else
    {
        screenContact.rotatorHeading.clear();
    }
}

//==============================================================================
void QSOLogFrame::logCurrentContact( )
{
   if (!contest || contest->isReadOnly() )
      return ;

   // copy the display into the correct log entry, etc

   getScreenEntry();

   // first check for extra lines to be added
   int ctno = screenContact.serials.toInt();
   if ( ctno > contest->maxSerial + 1 )
   {

      if ( mShowYesNoMessage( this, tr("\"Serial sent\" is too high - please confirm that this is correct?")) )
      {
         if ( mShowYesNoMessage( this, tr("Do you want to enter the missing contacts later?") ) )
         {
             dtg ctTime(screenContact.time);
             QSharedPointer<BaseContact> pct;
             if (contest->getContactCount() > 0)
                pct = contest->pcontactAt(contest->getContactCount() - 1);
             if ( pct )
             {
                ctTime = pct->time;
             }
             else
             {
                // use contest start time
                QDateTime DTGStart = CanonicalToTDT(contest->DTGStart.getValue());
                QString d = DTGStart.toString("dd/MM/yy");
                QString t = DTGStart.toString("HH:mm");
                ctTime.setDate( d, DTGDISP );
                ctTime.setTime( t.left(5), DTGDISP );
             }

             unsigned short orflag = TO_BE_ENTERED;

             int nct_no = contest->maxSerial + 1;
             do
             {
                // last child is "current contact", and we need to add TO IT
                LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( contest );
                QString currmode = ui->ModeComboBoxGJV->currentText();
                QString currSubmode = ui->MGMSubModeEdit->text().trimmed();
                ct->addContact( nct_no, orflag, true, false, currmode, currSubmode, ctTime ); // last contact
                nct_no++;
             }
             while ( nct_no < ctno ) ;
         }
      }
      else
      {
         // reset focus to serial sent
         //????????????? But its disabled...
         selectField( ui->SerTxFrame->getTextEditEdit() );
         return ;
      }
   }

   logScreenEntry( );  // true => move on to next contact
}
void QSOLogFrame::updateQSOTime(bool fromTimer)
{
    sortUnfilledCatchupTime();
    // Check if dtg is within the contest time
    // If not we wish to show as red
    // We need to do this in log displays as well
    if (!edit && !catchup)
    {
        dtg tnow( true );
        ui->dateEdit->setDate(tnow.getDate( ));
        ui->timeEdit->setTime(tnow.getTime( ));
    }
    dtg time(false);
    time.setDate( ui->dateEdit->date() );
    time.setTime( ui->timeEdit->time() );

    bool timeOK = false;

    if (contest)
    {
        timeOK = contest->checkTime(time);
    }

    if (fromTimer && timeOK != oldTimeOK)
    {
        oldTimeOK = timeOK;
        if (timeOK)
        {
            ui->dateEdit->setStyleSheet(ssDtgWhiteNoFrame);
            ui->timeEdit->setStyleSheet(ssDtgWhiteNoFrame);
            widgetStyles[ui->dateEdit] = ssDtgWhiteNoFrame;
            widgetStyles[ui->timeEdit] = ssDtgWhiteNoFrame;
        }
        else
        {
            QString ss = ssDtgRedNoFrame;
            if (ui->timeEdit->isEnabled())
            {
                ss = ssDtgRed;
            }
            ui->dateEdit->setStyleSheet(ss);
            ui->timeEdit->setStyleSheet(ss);
            widgetStyles[ui->dateEdit] = ss;
            widgetStyles[ui->timeEdit] = ss;
        }
    }
}
void QSOLogFrame::setDtgSection()
{
    delayedAction(this, [=]()
    {
        // NB a lambda function
        ui->timeEdit->setCurrentSection(QDateTimeEdit::MinuteSection);
        ui->dateEdit->setCurrentSection(QDateTimeEdit::DaySection);
    }
    );
}


void QSOLogFrame::transferDetails(const QSharedPointer<BaseContact> lct, const BaseContestLog *matct )
{
   ui->CallsignFrame->getTextEditEdit()->setText(lct->cs.getFullCall());
   ui->LocFrame->getTextEditEdit()->setText(lct->loc.getLoc());  // also forces update of score etc

   // only transfer qth info if required for this ContestLog
   // and it might be valid...
   if ( contest->districtMult.getValue() || contest->otherExchange.getValue() || contest->otherOptionalExchange.getValue() )
   {
      if ( ( contest->districtMult.getValue() && matct->districtMult.getValue() ) ||
           ( contest->otherExchange.getValue() && matct->otherExchange.getValue() ) ||
           ( contest->otherOptionalExchange.getValue() && matct->otherOptionalExchange.getValue() )
         )
      {
        QString exch = lct->extraText.getValue();
        if (exch.size())
        {
           ui->QTHFrame->getTextEditEdit()->setText(exch);
        }
      }
   }
   valid( cmCheckValid ); // make sure all single and cross field
   // validation has been done

   // make sure dtg gets entered if needed
   ui->CallsignFrame->getTextEditEdit()->setFocus();
   doAutofill();
   ui->CallsignFrame->getTextEditEdit()->setFocus();

   doGJVEditChange(ui->CallsignFrame->getTextEditEdit());
   doGJVEditChange(ui->LocFrame->getTextEditEdit());
   doGJVEditChange(ui->QTHFrame->getTextEditEdit());
}
void QSOLogFrame::transferDetails( const ListContact *lct, const ContactList * /*matct*/ )
{
   ui->CallsignFrame->getTextEditEdit()->setText(lct->cs.getFullCall());
   ui->LocFrame->getTextEditEdit()->setText(lct->loc.getLoc());

   // only transfer qth info if required for this ContestLog
   // and it might be valid...
   if ( contest->districtMult.getValue() || contest->otherExchange.getValue() || contest->otherOptionalExchange.getValue() )
   {
      if ( contest->districtMult.getValue() || contest->otherExchange.getValue() )
      {
        QString exch = lct->extraText;
        if (exch.size())
        {
            ui->QTHFrame->getTextEditEdit()->setText(exch);
        }
      }
   }
   valid( cmCheckValid ); // make sure all single and cross field
   // validation has been done

   // make sure dtg gets entered if needed
   ui->CallsignFrame->getTextEditEdit()->setFocus();
   doAutofill();
   ui->CallsignFrame->getTextEditEdit()->setFocus();

   doGJVEditChange(ui->CallsignFrame->getTextEditEdit());
   doGJVEditChange(ui->LocFrame->getTextEditEdit());
   doGJVEditChange(ui->QTHFrame->getTextEditEdit());
}

void QSOLogFrame::transferDetails(QString cs, const QString loc, QString exchange, const bool fromBandmapOrMemory )
{
    if (fromBandmapOrMemory)
    {
        logDataFromBandmapOrMemory = fromBandmapOrMemory;
    }

    ui->CallsignFrame->getTextEditEdit()->setText(cs);
    ui->LocFrame->getTextEditEdit()->setText(loc);
    if ( contest->districtMult.getValue() || contest->otherExchange.getValue() || contest->otherOptionalExchange.getValue() )
    {
       if ( contest->districtMult.getValue() || contest->otherExchange.getValue() )
       {
           if (exchange.size())
           {
                ui->QTHFrame->getTextEditEdit()->setText(exchange);
           }
       }
    }

    valid( cmCheckValid ); // make sure all single and cross field
    // validation has been done

    // make sure dtg gets entered if needed
    ui->CallsignFrame->getTextEditEdit()->setFocus();
    doAutofill();
    ui->CallsignFrame->getTextEditEdit()->setFocus();

    doGJVEditChange(ui->CallsignFrame->getTextEditEdit());
    doGJVEditChange(ui->LocFrame->getTextEditEdit());
    doGJVEditChange(ui->QTHFrame->getTextEditEdit());
}
void QSOLogFrame::transferFromWSJTX(QString call)
{
    ui->CallsignFrame->getTextEditEdit()->setText(call);
    doGJVEditChange(ui->CallsignFrame->getTextEditEdit());
}
void QSOLogFrame::sortUnfilledCatchupTime( )
{
    if (contest && !contest->isReadOnly() && ((screenContact.contactFlags & TO_BE_ENTERED) || catchup))
    {
        // Uri Mode - catchuping QSOs from paper while logging current QSOs
        // catchup - post contest entry of QSOs
        // and we need to set the date/time from the previous contact
        /*
        ui->timeEdit->setEnabled(true);
        ui->dateEdit->setEnabled(true);
        ui->dateEdit->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
        ui->timeEdit->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
        */

        int tne = screenContact.time.notEntered(); // partial dtg will give +fe
        // full dtg gives -ve, none gives 0
        if ( tne == 0 )
        {
            QSharedPointer<BaseContact> pct;
            if (catchup)
                pct = getLastContact();
            else
                pct = getPriorContact();
            if ( pct )
            {
                screenContact.time = pct->time;
                ui->dateEdit->setDate(screenContact.time.getDate( ));
                ui->timeEdit->setTime(screenContact.time.getTime( ));
            }
            else
            {
                // use contest start time
                QDateTime DTGStart = CanonicalToTDT(contest->DTGStart.getValue());
                ui->dateEdit->setDate(DTGStart.date());
                ui->timeEdit->setTime(DTGStart.time());
                dtg time(false);
                time.setDate( ui->dateEdit->date());
                time.setTime( ui->timeEdit->time() );
                screenContact.time = time;
            }

            setDtgSection();
        }
    }
    else if (!edit)
    {
    }

}
void QSOLogFrame::selectEntryForEdit( QSharedPointer<BaseContact> slct )
{
   selectedContact = slct;   // contact from log list selected

   ui->FirstUnfilledButton->setVisible(false);
   ui->CatchupButton->setVisible(false);

   screenContact.copyFromArg( slct );
   showScreenEntry();

   ui->PriorButton->setEnabled(getPriorContact());
   ui->NextButton->setEnabled(getNextContact());
   ui->InsertAfterButton->setEnabled(getNextContact() && !contest->isReadOnly());  // dont allow insert after last contact
   ui->InsertBeforeButton->setEnabled(getPriorContact() && !contest->isReadOnly());  // dont allow insert after last contact

   ui->MainOpComboBox->setCurrentText(slct->op1.getValue());
   ui->SecondOpComboBox->setCurrentText(slct->op2.getValue());
   sortUnfilledCatchupTime();
   ui->SerTxFrame->getTextEditEdit()->setReadOnly(!edit);

   int tne = screenContact.time.notEntered(); // partial dtg will give +fe
   // full dtg gives -ve, none gives 0

   if (tne < 0)
   {
      dtg time(false);
      time.setDate( ui->dateEdit->date() );
      time.setTime( ui->timeEdit->time() );
   }

   MinosLoggerEvents::SendAfterSelectContact(catchup?QSharedPointer<BaseContact>():slct, contest);
   selectField( nullptr );
}
//---------------------------------------------------------------------------
QSharedPointer<BaseContact> QSOLogFrame::getLastContact()
{
    LogIterator i = contest->ctList.end();
    while (i != contest->ctList.begin())
    {
        i--;
        if (i->wt->contactFlags.getValue() & DONT_PRINT)
            continue;
        return ( i->wt ) ;
    }
    return QSharedPointer<BaseContact>();
}
//---------------------------------------------------------------------------
QSharedPointer<BaseContact> QSOLogFrame::getPriorContact()
{
   for ( LogIterator i = contest->ctList.begin(); i != contest->ctList.end(); i++ )
   {
      if ( i->wt->getLogSequence() == screenContact.getLogSequence() )
      {
         if ( i != contest->ctList.begin() )
         {
            i--;
            return ( i->wt ) ;
         }
         return QSharedPointer<BaseContact>();
      }
   }
   return QSharedPointer<BaseContact>();
}


void QSOLogFrame::on_PriorButton_clicked()
{
   current = nullptr;            // make sure the focus moves off this button
   if ( !checkAndLogEntry() )
   {
      return ;
   }
   QSharedPointer<BaseContact> lct = getPriorContact();
   if ( lct )
   {
      selectEntryForEdit( lct );
   }
   else
   {
      mShowMessage( tr("Start of QSOs"), this );
   }
}
//---------------------------------------------------------------------------
QSharedPointer<BaseContact> QSOLogFrame::getNextContact()
{
   for ( LogIterator i = contest->ctList.begin(); i != contest->ctList.end(); i++ )
   {
      if ( i->wt->getLogSequence() == screenContact.getLogSequence() )
      {
         i++;
         if ( i != contest->ctList.end() )
         {
            return i->wt;
         }
         return QSharedPointer<BaseContact>();
      }
   }
   return QSharedPointer<BaseContact>();
}

void QSOLogFrame::on_NextButton_clicked()
{
   current = nullptr;            // make sure the focus moves off this button
   if ( !checkAndLogEntry() )
   {
      return ;
   }
   QSharedPointer<BaseContact> lct = getNextContact();
   if ( lct )
   {
      selectEntryForEdit( lct );
   }
   else
   {
      mShowMessage( tr("End of QSOs"), this );
   }
}

void QSOLogFrame::on_InsertBeforeButton_clicked()
{
    QSharedPointer<BaseContact> pct = getPriorContact();
    LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( contest );

    dtg ctTime = selectedContact->time;

    if (pct)
        ctTime = pct->time;

    QSharedPointer<BaseContact> newct = ct->addContactBetween(pct, selectedContact, ctTime);
    newct->contactFlags.setValue(newct->contactFlags.getValue()|TO_BE_ENTERED);
    selectEntryForEdit(newct);
}

void QSOLogFrame::on_InsertAfterButton_clicked()
{
    QSharedPointer<BaseContact> nct = getNextContact();
    LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( contest );
    dtg ctTime = selectedContact->time;

    QSharedPointer<BaseContact> newct = ct->addContactBetween(selectedContact, nct, ctTime);
    newct->contactFlags.setValue(newct->contactFlags.getValue()|TO_BE_ENTERED);
    selectEntryForEdit(newct);
}


//void QSOLogFrame::on_ModeComboBoxGJV_currentIndexChanged(int index)
void QSOLogFrame::on_ModeComboBoxGJV_activated(int index)
{

    if (ui->ModeComboBoxGJV->currentText() == mode)
        return;
    oldMode = mode;
    if (index < supModeList.count())
    {
        mode = supModeList[index];

        // send mode change to radio
        if (isRadioLoaded() && radioConnected && !radioError)
        {

            qsoLogModeFlag = true;  // stop updates from radio here
            emit sendModeControl(supModeList[index]);
        }
    }

    if (ui->ModeComboBoxGJV->currentText() == hamlibData::CW || ui->ModeComboBoxGJV->currentText() == hamlibData::MGM)
    {
       ui->ModeButton->setText(oldMode);
    }
    else
    {
       ui->ModeButton->setText(hamlibData::CW);
    }
    if (ui->ModeComboBoxGJV->currentText() == hamlibData::MGM)
    {
        if (ui->RSTTxFrame->getTextEditEdit()->text().trimmed() == "5")
        {
            ui->RSTTxFrame->getTextEditEdit()->clear();
        }
        if (ui->RSTRxFrame->getTextEditEdit()->text().trimmed() == "5")
        {
            ui->RSTRxFrame->getTextEditEdit()->clear();
        }
    }
    ui->MGMSubModeFrame->setVisible(ui->ModeComboBoxGJV->currentText() == hamlibData::MGM);
}

void QSOLogFrame::on_ValidateError (int mess_no )
{
      if ( mess_no == -1 )
      {
         errs.clear();
         return ;
      }

      // add the message into the error list
      errs.insert( &Validator::errDefs[ mess_no ], &Validator::errDefs[ mess_no ] );
}

//--------------------------------------------------------------

// keyer


void QSOLogFrame::setKeyerLoaded()
{
    keyerLoaded = true;
}
bool QSOLogFrame::isKeyerLoaded()
{
    return keyerLoaded;
}

//---------------------------------------------------------


void QSOLogFrame::setBandMapLoaded(bool loaded)
{
    bandMapLoaded = loaded;
}
bool QSOLogFrame::isBandMapLoaded()
{
    return bandMapLoaded;
}

void QSOLogFrame::setBandMapControlsVisible(bool visible)
{

    ui->bandmapMarkFreqPb->setVisible(visible);
    ui->bandmapSaveFreqPb->setVisible(visible);
    ui->tuningAddMapChkBox->setVisible(visible);


}

void QSOLogFrame::setBandMapControlsDisabled(bool disabled)
{

    ui->bandmapMarkFreqPb->setDisabled(disabled);
    ui->bandmapMarkFreqPb->setDisabled(disabled);
    ui->bandmapSaveFreqPb->setDisabled(disabled);
    ui->tuningAddMapChkBox->setDisabled(disabled);
}


void QSOLogFrame::tuningAddMapChkBoxStateChange(int state)
{


    if (state == Qt::Checked)
    {
        if (!getTuneAddBandMapSetting())
        {
            setTuneAddBandMapSetting(true);
        }

    }
    else
    {
        if (getTuneAddBandMapSetting())
        {
            setTuneAddBandMapSetting(false);
        }

    }
}

void QSOLogFrame::setTuningAddMapChkBoxState()
{
    bool state = getTuneAddBandMapSetting();
    if (state != ui->tuningAddMapChkBox->isChecked())
    {
        if (state)
        {
            ui->tuningAddMapChkBox->setCheckState(Qt::Checked);
        }
        else
        {
            ui->tuningAddMapChkBox->setCheckState(Qt::Unchecked);
        }
    }

}

bool QSOLogFrame::getTuneAddBandMapSetting()
{
    bool state = false;
    if (bandMapLoaded)
    {
        TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
        if (tslf)
        {
            state = tslf->getTuneAddBandMapSetting();
        }
    }

    return state;
}

void QSOLogFrame::setTuneAddBandMapSetting(bool state)
{
    if (bandMapLoaded)
    {
        TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
        tslf->setTuneAddBandMapSetting(state);
    }
}

void QSOLogFrame::on_SpotLastLoggedPbClicked()
{
    if (!lastLoggedCallsign.realCall.isEmpty() && !lastLoggedFreq.isClear())     // don't send a spot when running a freq
    {
        // send last spot logged
        trace(QString("spotButton: send last logged call %1 to dxCluster").arg(lastLoggedCallsign.realCall));
        emit sendSpotToClusterServer( lastLoggedFreq, lastLoggedCallsign.realCall, lastLoggedLocator );
        ui->lastSpotSentLbl->setText(lastLoggedCallsign.realCall + " " + lastLoggedFreq.convertFreqStrDisp());
    }
    else
    {
        trace(QString("spotLastLoggedButton: don't send spot, no valid call/frequency"));
    }

}
void QSOLogFrame::on_SpotPbClicked()
{
    if ((runButtonOnFlag && radioOffRunFreq) || !runButtonOnFlag)     // don't send a spot when running a freq
    {
        memoryData::memData logData;
        int valRes = -1;
        getLogDetails(logData, valRes);
        if (valRes == CS_OK)
        {
            // callsign valid
            if (!logData.callsign.isEmpty() || !logData.freq.isClear())
            {
               trace(QString("spotButton: send logged call %1 to dxCluster").arg(logData.callsign));
                emit sendSpotToClusterServer(logData.freq, logData.callsign, logData.locator);
               ui->lastSpotSentLbl->setText(logData.callsign + " " + logData.freq.convertFreqStrDisp());
            }
            else
            {
                trace(QString("spotButton: callsign - %1 or freq - %2 is empty").arg(logData.callsign).arg(curFreq.traceStr()));
            }
        }
        else
        {
            trace(QString("spotButton: callsign - %1 invalid").arg(logData.callsign));
        }
    }
    else
    {
        trace(QString("spotButton: don't send spot, on run freq"));
    }


}

void QSOLogFrame::on_BandmapMarkFreqPbClicked()
{
    memoryData::memData logData;
    int valRes = -1;
    getLogDetails(logData, valRes);

    trace(QString("bandmapMark: mark clicked callsign %1").arg(logData.callsign));
    emit bandmapMarkFreq(logData.callsign, logData.freq, logData.locator, QString::number(logData.bearing), logData.exchange);

}


void QSOLogFrame::on_bandmapSaveFreqPbClicked()
{
    memoryData::memData logData;
    int callRes = -1;
    getLogDetails(logData, callRes);
    if (callRes == CS_OK)
    {
        logData.freq = callsignEnterTextFreq;
        callsignEnterTextFreq.clear();

        doGJVCancelButton_clicked();

        trace(QString("bandmapSave: save clicked callsign %1").arg(logData.callsign));
        emit bandmapSaveFreq(logData.callsign, logData.freq, logData.locator, QString::number(logData.bearing), logData.exchange);


    }
    else
    {
       trace(QString("bandmapSave: save clicked callsign %1 not valid %2").arg(logData.callsign).arg(callRes));
    }


}




void QSOLogFrame::getLogDetails(memoryData::memData &logData, int& callRes)
{
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    getScreenEntry();
    calcLoc();
    callRes = screenContact.cs.getValRes();
    if (callRes != CS_OK)
    {
        logData.callsign = screenContact.cs.getFullCall();

    }

    logData.callsign = screenContact.cs.getFullCall();
    logData.freq = curFreq;
    logData.locator = screenContact.loc.getLoc();
    logData.mode = screenContact.mode;
    logData.exchange = screenContact.extraText;
    if (screenContact.loc.getLoc().isEmpty())
    {
        logData.bearing = tslf->getCurrentBearing();
    }
    else
    {
        logData.bearing = tslf->getBearingFrmQSOLog();
    }



}


//---------------------------------------------------------


void QSOLogFrame::setClusterClientLoaded(bool loaded)
{
    clusterClientLoaded = loaded;
}
bool QSOLogFrame::isClusterClientLoaded()
{
    return clusterClientLoaded;
}

void QSOLogFrame::setClusterServerLoaded(bool loaded)
{
    clusterServerLoaded = loaded;
}
bool QSOLogFrame::isClusterServerLoaded()
{
    return clusterServerLoaded;
}

void QSOLogFrame::setClusterTXSpotEnableState(bool txEnableState)
{
    setClusterSendSpotControlsVisible(txEnableState);
    setClusterSendSpotControlsDisabled(!txEnableState);
    sendSpotToClusterOn = txEnableState;
}

void QSOLogFrame::setClusterSendSpotControlsVisible(bool visible)
{

    ui->spotLastLoggedPb->setVisible(visible);
    ui->spotPb->setVisible(visible);
    ui->lastSpotSentTitleLbl->setVisible(visible);
    ui->lastSpotSentLbl->setVisible(visible);


}

void QSOLogFrame::setClusterSendSpotControlsDisabled(bool disabled)
{

    //ui->lastLoggedChkBx->setDisabled(disabled);
    ui->spotPb->setDisabled(disabled);
    ui->lastSpotSentTitleLbl->setDisabled(disabled);
    ui->lastSpotSentLbl->setDisabled(disabled);

}

void QSOLogFrame::setClusterServerState(QString stateMsg)
{
    QStringList s = stateMsg.split("<>");
    if (s.size() < 2)
        return;

    if (s[0].contains("Connected"))
    {
         clusterServerConnected = true;
    }
    else
    {
         clusterServerConnected = false;
    }
}


//--------------------------------------------------------

void QSOLogFrame::checkBandMapAndClusterLoaded()
{

    if (contest && !contest->isReadOnly() && isBandMapLoaded())
    {
        setBandmapControlsState();
        setTuningAddMapChkBoxState();
    }
    else
    {
        setBandMapControlsVisible(false);
    }

}





//---------------------------------------------------------
void QSOLogFrame::setRadioLoaded()
{
    radioLoaded = true;

}

bool QSOLogFrame::isRadioLoaded()
{
    return radioLoaded;
}

//----------------------------------------------------------------

void QSOLogFrame::setRotatorLoaded()
{
    rotatorLoaded = true;
}

bool QSOLogFrame::isRotatorLoaded()
{
    return rotatorLoaded;
}



//----------------------------------------------------------------
QString QSOLogFrame::getBearing()
{
    return ui->BrgSt->text();
}

//-------------------------------------------------------------------



void QSOLogFrame::setRunOnFlag(bool runModeOn)
{
    if (runButtonOnFlag != runModeOn)
    {
        runButtonOnFlag = runModeOn;
        setBandmapControlsState();
        setClusterSendSpotControlsState();

    }
    on_tabSandP();  // show (or not) the Call/S&P choiceon_
}

void QSOLogFrame::setRunOffFreqFlag(bool offRunFreq)
{
    if (radioOffRunFreq != offRunFreq)
    {

        radioOffRunFreq = offRunFreq;
        setBandmapControlsState();
        setClusterSendSpotControlsState();

    }
    on_tabSandP();  // show (or not) the Call/S&P choice
}


void QSOLogFrame::setBandmapControlsState()
{
    if (runButtonOnFlag)
    {
        if (radioOffRunFreq)
        {
            setBandMapControlsDisabled(false);
        }
        else
        {
            setBandMapControlsDisabled(true);
        }
    }
    else
    {
        if (isBandMapLoaded())
        {
            setBandMapControlsVisible(true);
            setBandMapControlsDisabled(false);
        }

    }
}

void QSOLogFrame::setClusterSendSpotControlsState()
{
    if (runButtonOnFlag)
    {
        if (radioOffRunFreq)
        {
            setClusterSendSpotControlsDisabled(false);
        }
        else
        {
            setClusterSendSpotControlsDisabled(true);
        }
    }
    else
    {
        if (isClusterServerLoaded() && sendSpotToClusterOn)
        {
            setClusterSendSpotControlsVisible(true);
            setClusterSendSpotControlsDisabled(false);
        }
    }
}



void QSOLogFrame::on_FreqChanged(Frequency f)
{

    if (!logDataFromBandmapOrMemory && isBandMapLoaded() && ui->tuningAddMapChkBox->isChecked())
    {
        qint64 dialFreq = qint64(f) / 1000;
        qint64 callsignEnterFreq = qint64(callsignEnterTextFreq) / 1000;
        int toleranceF = 0;
        if (callsignEnterFreq != 0)
        {
            if (callsignEnterFreq != dialFreq)
            {
                if (callsignEnterFreq < dialFreq)
                {
                    toleranceF = dialFreq - callsignEnterFreq;

                }
                else
                {
                    toleranceF = callsignEnterFreq - dialFreq;

                }

                if (toleranceF >= addToBandmapTuneTolerance)
                {

                    on_bandmapSaveFreqPbClicked();
                }
            }
        }
    }


}
