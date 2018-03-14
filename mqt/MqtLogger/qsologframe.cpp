#include "logger_pch.h"

#include "tlogcontainer.h"
#include "tqsoeditdlg.h"
#include "tforcelogdlg.h"
#include "SendRPCDM.h"
#include "rigcontrolcommonconstants.h"
#include "rigutils.h"
#include "qsologframe.h"
#include "ui_qsologframe.h"
#include <QDebug>
//#include <QStringList>

QSOLogFrame::QSOLogFrame(QWidget *parent) :
    QFrame(parent)
    , ui(new Ui::QSOLogFrame)
    , contest(0)
    , selectedContact(0)
    , current(0)
    , partialContact(0)
    , edit(false)
    , overstrike(false)
    , oldTimeOK(true)
    , rotatorLoaded(false)
    , bandMapLoaded(false)
    , keyerLoaded(false)
    , radioLoaded(false)
    , curFreq("00000000000")
    , radioConnected(false)
    , radioError(false)
{
    ui->setupUi(this);

    ui->BrgSt->setFixedSize(ui->BrgSt->size());
    ui->DistSt->setFixedSize(ui->DistSt->size());

    CallsignFW = new FocusWatcher(ui->CallsignEdit);
    CallsignLabelString = ui->Callsignlabel->text();
    ui->CallsignEdit->setValidator(new UpperCaseValidator(true));
    ui->CallsignEdit->installEventFilter(this);

    RSTTXFW = new FocusWatcher(ui->RSTTXEdit);
    RSTTXLabelString = ui->RSTTXLabel->text();
    ui->RSTTXEdit->setValidator(new UpperCaseValidator(true));
    ui->RSTTXEdit->installEventFilter(this);

    SerTXFW = new FocusWatcher(ui->SerTXEdit);
    SerTXLabelString = ui->SerTXLabel->text();
    ui->SerTXEdit->setValidator(new UpperCaseValidator(true));
    ui->SerTXEdit->installEventFilter(this);

    RSTRXFW = new FocusWatcher(ui->RSTRXEdit);
    RSTRXLabelString = ui->RSTRXLabel->text();
    ui->RSTRXEdit->setValidator(new UpperCaseValidator(true));
    ui->RSTRXEdit->installEventFilter(this);

    SerRXFW = new FocusWatcher(ui->SerRXEdit);
    SerRXLabelString = ui->SerRXLabel->text();
    ui->SerRXEdit->setValidator(new UpperCaseValidator(true));
    ui->SerRXEdit->installEventFilter(this);

    LocFW = new FocusWatcher(ui->LocEdit);
    LocLabelString = ui->LocLabel->text();
    ui->LocEdit->setValidator(new UpperCaseValidator(true));
    ui->LocEdit->installEventFilter(this);

    QTHFW = new FocusWatcher(ui->QTHEdit);
    QTHLabelString = ui->QTHLabel->text();
    ui->QTHEdit->setValidator(new UpperCaseValidator(true));
    ui->QTHEdit->installEventFilter(this);

    CommentsFW = new FocusWatcher(ui->CommentsEdit);
    CommentsLabelString = ui->CommentsLabel->text();
    ui->CommentsEdit->installEventFilter(this);

    MainOpFW = new FocusWatcher(ui->MainOpComboBox);
    ui->MainOpComboBox->setValidator(new UpperCaseValidator(false));
    SecondOpFW = new FocusWatcher(ui->SecondOpComboBox);
    ui->SecondOpComboBox->setValidator(new UpperCaseValidator(false));

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
    connect(ui->frequencyEdit, SIGNAL(editingFinished()), this, SLOT(on_FreqEditFinished()));

    ui->timeEdit->installEventFilter(this);
    ui->dateEdit->installEventFilter(this);


    for (int i = 0; i < hamlibData::supModeList.count(); i++)
    {
        ui->ModeComboBoxGJV->addItem(hamlibData::supModeList[i]);
    }

    ui->ModeComboBoxGJV->setCurrentText(hamlibData::USB);
    ui->ModeButton->setText(hamlibData::CW);

    connect(&MinosLoggerEvents::mle, SIGNAL(AfterTabFocusIn(QLineEdit*)), this, SLOT(on_AfterTabFocusIn(QLineEdit*)), Qt::QueuedConnection);
    connect(&MinosLoggerEvents::mle, SIGNAL(Validated()), this, SLOT(on_Validated()));
    connect(&MinosLoggerEvents::mle, SIGNAL(ValidateError(int)), this, SLOT(on_ValidateError(int)));
    connect(&MinosLoggerEvents::mle, SIGNAL(ShowOperators()), this, SLOT(on_ShowOperators()));
    connect(&MinosLoggerEvents::mle, SIGNAL(FontChanged()), this, SLOT(on_FontChanged()), Qt::QueuedConnection);

    ui->qsoFrame->setStyleSheet(ssQsoFrameBlue);
    widgetStyles[ui->qsoFrame] = ssQsoFrameBlue;
}

void QSOLogFrame::on_FontChanged()
{
    for (QMap<QWidget *, QString>::iterator i = widgetStyles.begin(); i != widgetStyles.end(); i++)
    {
        QWidget *w = i.key();
        w->setStyleSheet(i.value());
    }
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
        if (obj == ui->SerTXEdit || (edit && (obj == ui->timeEdit || obj == ui->dateEdit)))
        {
            mouseDoubleClickEvent(obj);
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
        return true;
    }
    if (Key == Qt::Key_Escape)
    {
        QMetaObject::invokeMethod(ui->GJVCancelButton, "clicked", Qt::QueuedConnection);
        return true;
    }

    if ( ( Key == Qt::Key_F1 || Key == Qt::Key_F2 || Key == Qt::Key_F3 || Key == Qt::Key_F4 || Key == Qt::Key_F5 || Key == Qt::Key_F6|| Key == Qt::Key_F12) )
    {
        setActiveControl( &Key );
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
        if (ed == ui->CallsignEdit)
        {
            ui->CallsignEdit->setStyleSheet(ssLineEditOK);
            widgetStyles[ui->CallsignEdit] = ssLineEditOK;
        }
    }
    return false;
}

QSOLogFrame::~QSOLogFrame()
{
    delete ui;
    for ( QVector <ValidatedControl *>::iterator vcp = vcs.begin(); vcp != vcs.end(); vcp++ )
    {
       delete ( *vcp );
    }
}
void QSOLogFrame::on_TimeDisplayTimer()
{
    updateQSOTime(true);
}

void QSOLogFrame::focusChange(QObject *obj, bool in, QFocusEvent *event)
{
    if (in)
        EditControlEnter(obj, event);
    else
        EditControlExit(obj);

    QColor colour = in?Qt::blue:Qt::black;
    QString colStr = HtmlFontColour(colour);

    if (obj == ui->CallsignEdit)
    {
        ui->Callsignlabel->setText(colStr + CallsignLabelString);
    }

    if (obj == ui->RSTTXEdit)
    {
        ui->RSTTXLabel->setText(colStr + RSTTXLabelString);
    }

    if (obj == ui->SerTXEdit)
    {
        ui->SerTXLabel->setText(colStr + SerTXLabelString);
    }

    if (obj == ui->RSTRXEdit)
    {
        ui->RSTRXLabel->setText(colStr + RSTRXLabelString);
    }

    if (obj == ui->SerRXEdit)
    {
        ui->SerRXLabel->setText(colStr + SerRXLabelString);
    }

    if (obj == ui->LocEdit)
    {
        ui->LocLabel->setText(colStr + LocLabelString);
    }

    if (obj == ui->QTHEdit)
    {
        ui->QTHLabel->setText(colStr + QTHLabelString);
    }

    if (obj == ui->CommentsEdit)
    {
        ui->CommentsLabel->setText(colStr + CommentsLabelString);
    }
    if (obj == ui->MainOpComboBox && in == false)
    {
        MainOpComboBox_Exit();
    }
    if (obj == ui->SecondOpComboBox && in == false)
    {
        SecondOpComboBox_Exit();
    }
}
void QSOLogFrame::setAsEdit(bool s, QString b)
{
    baseName = b;
    if (s)
    {
        edit = true;
        ui->GJVCancelButton->setText("Return to Log");
    }
}

void QSOLogFrame::initialise( BaseContestLog * pcontest )
{
    catchup = false;

    contest = pcontest;
    screenContact.initialise( contest ); // get ops etc correct

    csIl = new ValidatedControl( ui->CallsignEdit, vtCallsign );
    vcs.push_back( csIl );
    rsIl = new ValidatedControl( ui->RSTTXEdit, vtRST );
    vcs.push_back( rsIl );
    ssIl = new ValidatedControl( ui->SerTXEdit, vtSN );
    vcs.push_back( ssIl );
    rrIl = new ValidatedControl( ui->RSTRXEdit, vtRST );
    vcs.push_back( rrIl );
    srIl = new ValidatedControl( ui->SerRXEdit, vtSN );
    vcs.push_back( srIl );
    locIl = new ValidatedControl( ui->LocEdit, vtLoc );
    vcs.push_back( locIl );
    qthIl = new ValidatedControl( ui->QTHEdit, vtQTH );
    vcs.push_back( qthIl );
    cmntIl = new ValidatedControl( ui->CommentsEdit, vtComments );
    vcs.push_back( cmntIl );


    ui->BrgSt->clear();
    ui->DistSt->clear();

    setTimeStyles();

    if (edit)
    {
        ui->radioDetailsFrame->setVisible(true);
        ui->EditFrame->setVisible(true);
        ui->SerTXEdit->setStyleSheet(ssLineEditOK);
        widgetStyles[ui->SerTXEdit] = ssLineEditOK;
    }
    else
    {
        ui->radioDetailsFrame->setVisible(false);
        ui->EditFrame->setVisible(false);
        ui->SerTXEdit->setStyleSheet(ssLineEditGreyBackground);
        widgetStyles[ui->SerTXEdit] = ssLineEditGreyBackground;
    }

    updateQSODisplay();
    refreshOps();


    current = 0;
    oldTimeOK = true;
    connect(&MinosLoggerEvents::mle, SIGNAL(TimerDistribution()), this, SLOT(on_TimeDisplayTimer()));
    MinosLoggerEvents::SendReportOverstrike(overstrike, contest);

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

void QSOLogFrame::setXferEnabled(bool s)
{
    ui->MatchXferButton->setEnabled(s);
    QString ss;
    if (s)
        ss = ssRed;

    ui->MatchXferButton->setStyleSheet(ss);
    widgetStyles[ui->MatchXferButton] = ss;
}

void QSOLogFrame::on_CatchupButton_clicked()
{
    if (catchup)
    {
        catchup = false;
        ui->CatchupButton->setStyleSheet("");
        widgetStyles[ui->CatchupButton] = "";
        ui->CatchupButton->setText("Catch-up (Post Entry)");

        oldTimeOK = true;
    }
    else
    {
        catchup = true;
        ui->CatchupButton->setStyleSheet("background-color : coral;");
        widgetStyles[ui->CatchupButton] = "background-color : coral;";
        ui->CatchupButton->setText("End Catch-up");

    }
    // set the screencontact dtg as not entered
    screenContact.time.setDate(QString(), DTGLOG);
    screenContact.time.setTime(QString(), DTGLOG);
    setTimeStyles();
    sortUnfilledCatchupTime();
    selectField( 0 );
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
        contest->currentOp1.setValue( op1 );
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
       contest->currentOp2.setValue( op2 );
       if ( op2.size() )
       {
          contest->oplist.insert( op2, op2 );
       }
       contest->commonSave(false);
       refreshOps();
    }
}


void QSOLogFrame::on_FreqEditFinished()
{
    QString f = ui->frequencyEdit->text().trimmed().remove( QRegExp("^[0]*"));
    if (f != "")
    {
        if (f.count('.') == 1)
        {
            QStringList fl = f.split('.');
            fl[1] = fl[1] + "000000";
            fl[1].truncate(6);
            f = fl[0] + "." + fl[1];
        }
        if (!validateFreqTxtInput(f))
        {
            // error
            QMessageBox msgBox;
            msgBox.setText(FREQ_EDIT_ERR_MSG);
            msgBox.exec();

        }
    }
}

void QSOLogFrame::on_GJVOKButton_clicked()
{
    if ( contest->isReadOnly() )
    {
       return;
    }
    ui->SerTXEdit->setReadOnly(!edit);

    getScreenEntry(); // make sure it is saved

    if ( screenContact.contactFlags & ( LOCAL_COMMENT | DONT_PRINT | COMMENT_ONLY ) )
    {
       logCurrentContact( );
       return;
    }

    // validate the entry; if still invalid, spin round the invalid
    // controls (this should really be the job of tab, but...)

    QWidget *currn = current;
    if ( !valid( cmCheckValid ) || ( currn == ui->RSTTXEdit ) || ( currn == ui->RSTRXEdit ) )
       // make sure all single and cross field
       // validation has been done
    {
       doAutofill();
    }
    bool was_unfilled = screenContact.contactFlags & TO_BE_ENTERED;
    if ( !valid( cmCheckValid ) )   // make sure all single and cross field
                                    // validation has been done
    {
       QWidget * firstInvalid = 0;
       QWidget *nextInvalid = 0;
       bool onCurrent = false;
       bool pastCurrent = false;
       for ( QVector <ValidatedControl *>::iterator vcp = vcs.begin(); vcp != vcs.end(); vcp++ )
       {
          if ( !( *vcp ) ->wc->isVisible() || !( *vcp ) ->wc->isEnabled())
          {
 //         #error but if date or time are invalid...
             continue;
          }
          if ( onCurrent )
             pastCurrent = true;
          if ( ( *vcp ) ->wc == current )
             onCurrent = true;
          if ( !( *vcp ) ->valid( cmValidStatus, screenContact ) )
          {
             if ( !firstInvalid )
                firstInvalid = ( *vcp ) ->wc;
             if ( pastCurrent )
             {
                if ( !nextInvalid )
                {
                   nextInvalid = ( *vcp ) ->wc;
                   break;
                }
             }
          }
       }

       // make sure we go to the invalid field

       QWidget *nextf = ( nextInvalid ) ? nextInvalid : firstInvalid;

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
               selectField( 0 );             // make sure we move off the "Log" default button
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
    ui->SerTXEdit->setReadOnly(!edit);

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
bool QSOLogFrame::restorePartial( void )
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
void QSOLogFrame::killPartial( void )
{
   if ( partialContact )
   {
      delete partialContact;
      partialContact = 0;
   }
}
void QSOLogFrame::on_Validated()
{
    killPartial();
}

void QSOLogFrame::startNextEntry( )
{
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
        ui->SerTXEdit->setReadOnly(!edit);

        ScreenContact *temp = 0;
        if ( !partialContact )
        {
           savePartial();
           temp = partialContact;
           partialContact = 0;
        }
        startNextEntry();
        ui->CallsignEdit->setFocus();

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
    emit xferPressed();;
}
/*
void QSOLogFrame::on_BandMapButton_clicked()
{
    MinosLoggerEvents::SendBandMapPressed();
}
*/
void QSOLogFrame::on_QTHEdit_textChanged(const QString &/*arg1*/)
{
    doGJVEditChange( ui->QTHEdit );
}

void QSOLogFrame::on_CallsignEdit_textChanged(const QString &/*arg1*/)
{
    doGJVEditChange( ui->CallsignEdit );
}

void QSOLogFrame::on_LocEdit_textChanged(const QString &/*arg1*/)
{
   // force bearing calc
   getScreenEntry();
   calcLoc();
   doGJVEditChange( ui->LocEdit );
}

void QSOLogFrame::on_RSTTXEdit_textChanged(const QString &/*arg1*/)
{
    doGJVEditChange( ui->RSTTXEdit );
}

void QSOLogFrame::on_RSTRXEdit_textChanged(const QString &/*arg1*/)
{
    doGJVEditChange( ui->RSTRXEdit );
}

void QSOLogFrame::mouseDoubleClickEvent(QObject *w)
{
    // Don't let the dtg be changed when the contest is protected
    if (contest->isReadOnly())
        return;

    if (w == ui->SerTXEdit)
    {
        ui->SerTXEdit->setReadOnly(false);
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
         selectField( ui->CallsignEdit );
         *Key = 0;
         break;
      case Qt::Key_F2:
         selectField( ui->RSTTXEdit );
         *Key = 0;
         break;
      case Qt::Key_F3:
         selectField( ui->RSTRXEdit );
         *Key = 0;
         break;
      case Qt::Key_F4:
         selectField( ui->SerRXEdit );
         *Key = 0;
         break;
      case Qt::Key_F5:
         selectField( ui->LocEdit );
         *Key = 0;
         break;
      case Qt::Key_F6:
         selectField( ui->QTHEdit );
         *Key = 0;
         break;
   case Qt::Key_F12:
         emit xferPressed();
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
   screenContact.cs.fullCall.setValue( ui->CallsignEdit->text().trimmed() );
   screenContact.cs.valRes = CS_NOT_VALIDATED;
   screenContact.cs.validate( );

   screenContact.reps = ui->RSTTXEdit->text().trimmed();
   screenContact.serials = ui->SerTXEdit->text().trimmed();
   screenContact.repr = ui->RSTRXEdit->text().trimmed();
   screenContact.serialr = ui->SerRXEdit->text().trimmed();

   QString loc = ui->LocEdit->text().trimmed();
   screenContact.loc.loc.setValue( loc );
   screenContact.bearing = -1;		// force a recalc
   screenContact.loc.validate();

   QString extra = ui->QTHEdit->text().trimmed();
   screenContact.extraText = extra;

   QString comments = ui->CommentsEdit->text().trimmed();
   screenContact.comments = comments;
   if (edit)
   {
       screenContact.rigName = ui->radioEdit->text().trimmed();

       QString f = ui->frequencyEdit->text().trimmed().remove( QRegExp("^[0]*")); //remove leading zeros
       f = convertSinglePeriodFreqToMultiPeriod(convertSinglePeriodFreqToFullDigit(f));
       screenContact.frequency = f;

       //screenContact.frequency = ui->frequencyEdit->text().trimmed();
       screenContact.rotatorHeading = ui->rotatorHeadingEdit->text().trimmed();
   }
   screenContact.mode = ui->ModeComboBoxGJV->currentText().trimmed();
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
void QSOLogFrame::showScreenEntry( void )
{
   // display the contents of the contest->screenContact

   if ( contest )
   {
      // we only validate this contact up to the validation point

      contest->validationPoint = selectedContact?selectedContact->getLogSequence():0;
      ScreenContact temp;
      temp.copyFromArg( screenContact ); // as screen contact gets corrupted by auto changes
      // op1, op2 in ScreenContact ge corrupted as well
      showScreenContactTime();
      ui->CallsignEdit->setText(temp.cs.fullCall.getValue().trimmed());
      ui->RSTTXEdit->setText(temp.reps.trimmed());
      ui->SerTXEdit->setText(temp.serials.trimmed());
      ui->RSTRXEdit->setText(temp.repr.trimmed());
      ui->SerRXEdit->setText(temp.serialr.trimmed());
      ui->LocEdit->setText(temp.loc.loc.getValue().trimmed());;  // also forces update of score etc
      ui->QTHEdit->setText(temp.extraText.trimmed());;
      ui->CommentsEdit->setText(temp.comments.trimmed());;
      ui->NonScoreCheckBox->setChecked(temp.contactFlags & NON_SCORING);
      ui->DeletedCheckBox->setChecked(temp.contactFlags & DONT_PRINT);
      if (edit)
      {
          ui->radioEdit->setText(temp.rigName);

          QString freq;

          QString newfreq = temp.frequency.trimmed().remove('.');
          if (!newfreq.isEmpty())
          {
              double dfreq = convertStrToFreq(newfreq);
              dfreq = dfreq/1000000.0;  // MHz

              freq = QString::number(dfreq, 'f', 6); //MHz to 6 decimal places
          }
          ui->frequencyEdit->setText(freq.remove( QRegExp("0+$"))); //remove trailing zeros
          ui->rotatorHeadingEdit->setText(temp.rotatorHeading);
      }
      setMode(temp.mode.trimmed());

      // and now we want to put the selection on each at the END of the text
      for ( QVector <ValidatedControl *>::iterator vcp = vcs.begin(); vcp != vcs.end(); vcp++ )
      {
         int selpt = ( *vcp ) ->wc->text().length();
         ( *vcp )->wc ->setSelection(selpt, 0);
      }

      valid( cmCheckValid ); // make sure contact is valid - display any errors
      if ( temp.contactFlags & ( COMMENT_ONLY | LOCAL_COMMENT ) )
      {
         selectField( ui->CommentsEdit );
      }
      else
         selectField( 0 );

      ui->SerTXEdit->setReadOnly(!edit);
      MinosLoggerEvents::SendScreenContactChanged(&screenContact, contest, baseName);
   }
}
//---------------------------------------------------------------------------
void QSOLogFrame::EditControlEnter(QObject *Sender , QFocusEvent *event)
{
   current = dynamic_cast<QWidget *>(Sender);
   QLineEdit *tle = dynamic_cast<QLineEdit *>(Sender);

   if (event->reason() == Qt::TabFocusReason || event->reason() == Qt::BacktabFocusReason)
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
   ui->SerTXEdit->setReadOnly(!edit);

   if ( current == ui->LocEdit )
   {
      // do any required character substitutions, but only when we have a full
      // locator

      // This does something to correct sloppy typing

      QString loc = ui->LocEdit->text().trimmed().toUpper();
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
         if (loc != ui->LocEdit->text().trimmed())
         {
            ui->LocEdit->setText(loc);
            doGJVEditChange( ui->LocEdit );   // start the match thread
         }
      }
   }
   if ( ( current == ui->CallsignEdit ) || ( current == ui->LocEdit ) )
   {
      getScreenEntry(); // make sure it is saved
      valid( cmCheckValid ); // make sure all single and cross field
      doAutofill();           // should only be time to be filled
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
   QString gridref = ui->LocEdit->text().trimmed();

   if ( gridref.compare(oldloc, Qt::CaseInsensitive ) != 0 )
   {
      oldloc = gridref;
      locValid = true;
      double latitude;
      double longitude;
      double dist;
      int brg;

      int locValres = lonlat( gridref, longitude, latitude );
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
            contest->disbear( longitude, latitude, dist, brg );

            if ( locValid )  	// just 4CHAR not enough
            {
               sct.bearing = brg;

               if ( !( sct.contactFlags & ( MANUAL_SCORE | NON_SCORING | LOCAL_COMMENT | COMMENT_ONLY | DONT_PRINT ) ) )
               {
                  sct.contactScore = dist;
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

    for ( QVector <ValidatedControl *>::iterator vcp = vcs.begin(); vcp != vcs.end(); vcp++ )
   {
        QString ss = ssLineEditOK;
        if (!edit && (*vcp) == ssIl)
            ss = ssLineEditGreyBackground;

        if (((*vcp)->wc->isEnabled() || ((*vcp) == ssIl && contest->serialField.getValue())) && !( *vcp ) ->valid( command, screenContact ) )
        {
            QString text = (*vcp)->wc->text().trimmed();
            if (!text.isEmpty())
            {
                if ((*vcp) == csIl)
                {
                    if ( screenContact.cs.valRes == ERR_DUPCS)
                    {
                        ss = ssLineEditFrRedBkRed;
                    }
                    else
                    {
                        ss = ssLineEditFrRedBkWhite;
                    }
                }
                else if ((*vcp) == rsIl && text == "5")
                {
                    // leave as no error
                }
                else if ((*vcp) == rrIl && text == "5")
                {
                    // leave as no error
                }
                else if ((*vcp) == locIl)
                {
                    // leave as no error
                    if (screenContact.loc.valRes == ERR_LOC_RANGE && screenContact.loc.loc.getValue().size() > 4)
                    {
                        ss = ssLineEditFrRedBkRed;
                    }
                    else if (screenContact.loc.valRes != LOC_OK)
                    {
                        ss = ssLineEditFrRedBkWhite;
                    }
                }
                else if ((*vcp) == qthIl)
                {
                    // leave as no error except if exchange is wrong??
                }
                else
                {
                    ss = ssLineEditFrRedBkRed;
                }
            }
            ret = false;
        }

        if ((*vcp)->wc->isEnabled())
        {
            (*vcp)->wc->setStyleSheet(ss);
            widgetStyles[(*vcp)->wc] = ss;
        }
        else
        {
            QString ss = QString("[readOnly=\"true\"] { background-color: %0 }").arg(qApp->palette().color(QPalette::Window).name(QColor::HexRgb));
            (*vcp)->wc->setStyleSheet(ss);
            widgetStyles[(*vcp)->wc] = ss;
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
//---------------------------------------------------------------------------
void QSOLogFrame::selectField( QWidget *v )
{
    if ( v == 0 )
    {
        v = ui->CallsignEdit;

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

    if ( ( current == ui->CallsignEdit ) || ( current == ui->LocEdit ) )
    {
        valid( cmCheckValid ); // make sure all single and cross field
        doAutofill();
    }
    if ( v == ui->SerTXEdit )
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

   fillRst( ui->RSTTXEdit, vcct->reps, vcct->mode );
   fillRst( ui->RSTRXEdit, vcct->repr, vcct->mode );
}
//==============================================================================
void QSOLogFrame::lgTraceerr( int err )
{
   MinosLoggerEvents::SendValidateError(err);
}
//==============================================================================
void QSOLogFrame::contactValid( void )
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
   // cs worked

   vcct->cs.valRes = CS_NOT_VALIDATED;

   // we only validate this contact up to the validation point of the contest
   contest->validationPoint = selectedContact?selectedContact->getLogSequence():0 ;

   int csret = vcct->cs.validate( );
   contest->DupSheet.clearCurDup();
   if ( csret == CS_OK )
   {
      if ( contest->DupSheet.checkCurDup( vcct, contest->validationPoint, false ) )
      {
         if ( contest->DupSheet.isCurDup( vcct ) )      // But vcct is screen contact... so it won't be curdup
         {
            csret = vcct->cs.valRes = ERR_DUPCS;
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

   int locrep = vcct->loc.validate();
   if ( locrep != 0 )
   {
      if ( contest->locatorField.getValue() )
         locIl->tIfValid = false;
      if ( contest->locatorField.getValue() && ( locrep == ERR_NOLOC ) )
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

      QString sloc = vcct->loc.loc.getValue().left(4);
      if (sloc.size())
      {
         bool LocOK;

         QString prefix = vcct->ctryMult->basePrefix;
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
                             "This Contact has changed: Shall I log the changes?\n"
                             "\n"
                             "Yes         - Log as shown\n"
                             "No          - Discard changes"
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


}
//---------------------------------------------------------------------------
void QSOLogFrame::setFreq(QString f)
{
    if (curFreq != f)
    {
        curFreq = f;

    }
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
   current = 0;
}
//---------------------------------------------------------------------------
void QSOLogFrame::updateQSODisplay()
{
   if ( contest->districtMult.getValue() )
   {
//      ui->QTHEdit->CharCase = ecUpperCase;
   }
   else
   {
//      ui->QTHEdit->CharCase = ecNormal;
   }
   bool notProtected = !contest->isReadOnly();
   ui->RSTTXEdit->setEnabled(notProtected && contest->RSTField.getValue());
   ui->SerTXEdit->setEnabled(notProtected && contest->serialField.getValue());
   ui->RSTRXEdit->setEnabled(notProtected && contest->RSTField.getValue());
   ui->SerRXEdit->setEnabled(notProtected && contest->serialField.getValue());
   //CallsignEdit->Enabled = false; // leave these to allow searching
   ui->LocEdit->setEnabled(contest->locatorField.getValue());
   ui->CommentsEdit->setEnabled(notProtected);
   ui->ModeComboBoxGJV->setEnabled(notProtected);
   ui->NonScoreCheckBox->setEnabled(notProtected);
   ui->DeletedCheckBox->setEnabled(notProtected);
   ui->GJVOKButton->setEnabled(notProtected);
   ui->GJVForceButton->setEnabled(notProtected);
   ui->radioEdit->setEnabled(notProtected);
   ui->frequencyEdit->setEnabled(notProtected);
   ui->rotatorHeadingEdit->setEnabled(notProtected);

   bool exchangeNeeded = contest->otherExchange .getValue() || contest->districtMult.getValue();
   ui->QTHEdit->setEnabled( exchangeNeeded );

   ui->ModeButton->setEnabled(notProtected);
   ui->SecondOpComboBox->setEnabled(notProtected);
   ui->MainOpComboBox->setEnabled(notProtected);

   ui->InsertBeforeButton->setEnabled(notProtected);
   ui->InsertAfterButton->setEnabled(notProtected);

   bool mgm = contest->MGMContestRules.getValue();

   ui->ModeComboBoxGJV->setEnabled(!mgm);
   ui->ModeButton->setEnabled(!mgm);

   on_FontChanged();    // do all style sheets again

   repaint();
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
       for ( OperatorIterator i = contest->oplist.begin(); i != contest->oplist.end(); i++ )
       {
           if (!(*i).isEmpty())
             ops.append(*i);
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
        for ( OperatorIterator i = contest->oplist.begin(); i != contest->oplist.end(); i++ )
        {
            if (!(*i).isEmpty())
              ops.append(*i);
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
//---------------------------------------------------------------------------
void QSOLogFrame::closeContest()
{
   BaseContestLog * currentContest = TContestApp::getContestApp() ->getCurrentContest();
   if (contest == currentContest)
   {
      ui->GJVCancelButton->setEnabled(true);
      ui->GJVCancelButton->setFocus();
   }
   contest = 0;
}
//---------------------------------------------------------------------------
void QSOLogFrame::doGJVEditChange( QObject *Sender )
{
   // sensitive field changed - trigger match scan
   if ( contest )
   {
      getScreenEntry();

      if ( current == ui->CallsignEdit || Sender == ui->CallsignEdit )
      {
         // clear the error list
         contest->DupSheet.clearCurDup();	// as edited, no longer a dup(?)

      }
      if ( current == ui->LocEdit || Sender == ui->LocEdit )
      {
         // force bearing calc
         calcLoc();
      }
      MinosLoggerEvents::SendScreenContactChanged(&screenContact, contest, baseName);
      valid( cmCheckValid ); // make sure all single and cross field
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
    QString mode = mlist[0];

    for (int i = 0; i < hamlibData::supModeList.count(); i++)
    {
        if (mode == hamlibData::supModeList[i])
        {
            oldMode = ui->ModeComboBoxGJV->currentText();
            if (mode == ui->ModeComboBoxGJV->currentText())
            {
                return;
            }
            {
                // set index to new mode
                ui->ModeComboBoxGJV->setCurrentIndex(ui->ModeComboBoxGJV->findText(mode));
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
        lct = ct->addContact( ctmax, 0, false, false, screenContact.mode, dtg(true) );	// "current" doesn't get flag, don't save ContestLog yet
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
             if ( MinosParameters::getMinosParameters() ->yesNoMessage( this, "Change mode to CW?" ) )
             {
                screenContact.mode = hamlibData::CW;
             }
          }
          else
          {
             if ( MinosParameters::getMinosParameters() ->yesNoMessage( this, "Change mode to USB?" ) )
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
        screenContact.frequency = convertFreqStrDisp(curFreq);
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

      if ( mShowYesNoMessage( this, "\"Serial sent\" is too high - please confirm that this is correct?") )
      {
         if ( mShowYesNoMessage( this, "Do you want to enter the missing contacts later?" ) )
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

             int orflag = TO_BE_ENTERED;

             int nct_no = contest->maxSerial + 1;
             do
             {
                // last child is "current contact", and we need to add TO IT
                LoggerContestLog *ct = dynamic_cast<LoggerContestLog *>( contest );
                QString currmode = ui->ModeComboBoxGJV->currentText();
                ct->addContact( nct_no, orflag, true, false, currmode, ctTime ); // last contact
                nct_no++;
             }
             while ( nct_no < ctno ) ;
         }
      }
      else
      {
         // reset focus to serial sent
         //????????????? But its disabled...
         selectField( ui->SerTXEdit );
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
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);

    connect(timer, &QTimer::timeout, [=]()
    {
        // NB a lambda function
        ui->timeEdit->setCurrentSection(QDateTimeEdit::MinuteSection);
        ui->dateEdit->setCurrentSection(QDateTimeEdit::DaySection);
        timer->deleteLater();
    }
    );

    timer->start(10);
}
void QSOLogFrame::transferDetails(const QSharedPointer<BaseContact> lct, const BaseContestLog *matct )
{
   ui->CallsignEdit->setText(lct->cs.fullCall.getValue());
   ui->LocEdit->setText(lct->loc.loc.getValue());  // also forces update of score etc

   // only transfer qth info if required for this ContestLog
   // and it might be valid...
   if ( contest->districtMult.getValue() || contest->otherExchange.getValue() )
   {
      if ( ( contest->districtMult.getValue() && matct->districtMult.getValue() ) ||
           ( contest->otherExchange.getValue() && matct->otherExchange.getValue() )
         )
      {
        QString exch = lct->extraText.getValue();
        if (exch.size())
        {
           ui->QTHEdit->setText(exch);
        }
      }
   }
   valid( cmCheckValid ); // make sure all single and cross field
   // validation has been done

   // make sure dtg gets entered if needed
   ui->CallsignEdit->setFocus();
   doAutofill();
   ui->CallsignEdit->setFocus();

   doGJVEditChange(ui->CallsignEdit);
   doGJVEditChange(ui->LocEdit);
   doGJVEditChange(ui->QTHEdit);
}
void QSOLogFrame::transferDetails( const ListContact *lct, const ContactList * /*matct*/ )
{
   ui->CallsignEdit->setText(lct->cs.fullCall.getValue());
   ui->LocEdit->setText(lct->loc.loc.getValue());

   // only transfer qth info if required for this ContestLog
   // and it might be valid...
   if ( contest->districtMult.getValue() || contest->otherExchange.getValue() )
   {
      if ( contest->districtMult.getValue() || contest->otherExchange.getValue() )
      {
        QString exch = lct->extraText;
        if (exch.size())
        {
           ui->QTHEdit->setText(exch);
        }
      }
   }
   valid( cmCheckValid ); // make sure all single and cross field
   // validation has been done

   // make sure dtg gets entered if needed
   ui->CallsignEdit->setFocus();
   doAutofill();
   ui->CallsignEdit->setFocus();

   doGJVEditChange(ui->CallsignEdit);
   doGJVEditChange(ui->LocEdit);
   doGJVEditChange(ui->QTHEdit);
}

void QSOLogFrame::transferDetails(QString cs, const QString loc )
{
    ui->CallsignEdit->setText(cs);
    ui->LocEdit->setText(loc);

    valid( cmCheckValid ); // make sure all single and cross field
    // validation has been done

    // make sure dtg gets entered if needed
    ui->CallsignEdit->setFocus();
    doAutofill();
    ui->CallsignEdit->setFocus();

    doGJVEditChange(ui->CallsignEdit);
    doGJVEditChange(ui->LocEdit);
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

   ui->MainOpComboBox->setCurrentText(screenContact.op1);
   ui->SecondOpComboBox->setCurrentText(screenContact.op2);
   sortUnfilledCatchupTime();
   ui->SerTXEdit->setReadOnly(!edit);

   int tne = screenContact.time.notEntered(); // partial dtg will give +fe
   // full dtg gives -ve, none gives 0

   if (tne < 0)
   {
      dtg time(false);
      time.setDate( ui->dateEdit->date() );
      time.setTime( ui->timeEdit->time() );
   }

   MinosLoggerEvents::SendAfterSelectContact(catchup?QSharedPointer<BaseContact>():slct, contest);
   selectField( 0 );
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
   current = 0;            // make sure the focus moves off this button
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
      mShowMessage( "Start of QSOs", this );
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
   current = 0;            // make sure the focus moves off this button
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
      mShowMessage( "End of QSOs", this );
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
    if (index < hamlibData::supModeList.count())
    {
        mode = hamlibData::supModeList[index];

        // send mode change to radio
        if (isRadioLoaded() && radioConnected && !radioError)
        {

            qsoLogModeFlag = true;  // stop updates from radio here
            emit sendModeControl(hamlibData::supModeList[index]);
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
}

void QSOLogFrame::on_ValidateError (int mess_no )
{
      if ( mess_no == -1 )
      {
         errs.clear();
         return ;
      }

      // add the message into the error list
      errs.insert( &errDefs[ mess_no ], &errDefs[ mess_no ] );
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


void QSOLogFrame::setBandMapLoaded()
{
    bandMapLoaded = true;
}
bool QSOLogFrame::isBandMapLoaded()
{
    return bandMapLoaded;
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
