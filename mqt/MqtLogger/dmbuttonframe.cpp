#include <QTextStream>
#include <QPushButton>
#include <QFileDialog>
#include <QDesktopServices>
#include <QFileSystemWatcher>

#include "ContestApp.h"
#include "MShowMessageDlg.h"
#include "MinosLoggerEvents.h"
#include "MinosParameters.h"

#include "dmbuttonframe.h"
#include "tlogcontainer.h"
#include "tsinglelogframe.h"

#include "ui_dmbuttonframe.h"

DMButtonFrame::DMButtonFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::DMButtonFrame)
{
    ui->setupUi(this);

    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::fKey, this, &DMButtonFrame::fKey);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::SandPChanged, this, &DMButtonFrame::sandPChanged);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::DMSender, this, &DMButtonFrame::DMSender);

    TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpDigiFunctionKeyFile, fkeyFileName );

    fButtons << ui->F1Button << ui->F2Button << ui->F3Button << ui->F4Button << ui->F5Button << ui->F6Button;
    fButtons << ui->F7Button << ui->F8Button << ui->F9Button << ui->F10Button << ui->F11Button << ui->F12Button;

    int i = Qt::Key_F1;
    for (auto b: qAsConst(fButtons))
    {
        b->setProperty("KeyNo", i++);
        connect(b, &QPushButton::clicked, this, &DMButtonFrame::fButtonClicked);
    }

    ui->nameLabel->setText(tr("Data Modes Buttons from %1").arg(fkeyFileName));
}

DMButtonFrame::~DMButtonFrame()
{
    delete ui;
}
void DMButtonFrame::DMSender(QString s)
{
    dataSender = s;

    fkeyFileChanged();
    qfsw = new QFileSystemWatcher(this);
    qfsw->addPath(fkeyFileName);
    connect(qfsw, &QFileSystemWatcher::fileChanged, this, &DMButtonFrame::fkeyFileChanged);
}
void DMButtonFrame::fkeyFileChanged()
{
    parseFKeyFile(fkeyFileName, "Digi");

    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    bool sandp = tslf->GJVQSOLogFrame->getSandP();
    showFButtons(sandp);
}
void DMButtonFrame::fButtonClicked()
{
    QPushButton *b = dynamic_cast<QPushButton *>(sender());
    int kno = b->property("KeyNo").toInt();
    fKey(kno);
}
void DMButtonFrame::setContest(BaseContestLog *c)
{
    ct = c;
}
void DMButtonFrame::fKey(int key)
{
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    if (tslf->getContest() == ct)
    {
        if (key >= Qt::Key_F1 && key <= Qt::Key_F12 && fkeys["Digi"].size() == 24)
        {
            int spoffset = tslf->GJVQSOLogFrame->getSandP()?12:0;
            QPair<QString, QString> mess = fkeys["Digi"][key - Qt::Key_F1 + spoffset];

            QString toSend = parseFKeyMessage(mess.second);

            // send transmission to sender app

            RPCGeneralClient rpc(rpcConstants::DMTransmit);
            QSharedPointer<RPCParam>st(new RPCParamStruct);
            st->addMember( toSend, rpcConstants::DMTransmit );
            rpc.getCallArgs() ->addParam( st );
            rpc.queueCall( dataSender );

        }
    }
}

void DMButtonFrame::sandPChanged(bool s)
{
    showFButtons(s);
}
void DMButtonFrame::showFButtons(bool s)
{
    if (fkeys["Digi"].size() == 24)
    {
        for (int i = 0; i < 12; i++)
        {
            fButtons[i]->setText(fkeys["Digi"][i + (s?12:0)].first);
        }
    }
    else if (fkeys["Digi"].size() == 0)
    {
        for (int i = 0; i < 12; i++)
        {
            fButtons[i]->setText(QString("F%1").arg(i, 1));
        }
    }
    else {
        mShowMessage(tr("Not enough key definitions in %1").arg(fkeyFileName), this);
    }
}
QString DMButtonFrame::parseFKeyMessage(QString mess)
{
    // make sure screenContact is up to date
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    tslf->GJVQSOLogFrame->getScreenEntry();
    ScreenContact *sc = &tslf->GJVQSOLogFrame->screenContact;

    QString txMess;

    // and parse the message

    for (int i = 0; i < mess.length(); i++)
    {
        QChar c = mess[i];
        if (c == '*')
        {
            txMess += ct->mycall.getFullCall();
        }
        else if (c == '#')
        {
            txMess += sc->serials;
        }
        else if (c == '!')
        {
            txMess += sc->cs.getFullCall();
        }
        else if (c == '{')
        {
            int lb = mess.indexOf('}', i);
            if (lb)
            {
                QString macro = mess.mid(i + 1, lb - i - 1).toUpper();
                i = lb;
                if (macro == "MYCALL")
                {
                    txMess += ct->mycall.getFullCall();
                }
                else if (macro == "CALL")
                {
                    txMess += sc->cs.getFullCall();
                }
                else if (macro == "TX")
                {
                    // switch to TX
                }
                else if (macro == "RX")
                {
                    // switch to RX
                }
                else if (macro == "CQ")
                {
                    // switch to CQ (run mode?)

                }
                else if (macro == "EXCH")
                {
                    // This is whatever exchange is required
                    // May have multiple elements!
                    // in particular, includes serial number

                    // we need an exchange definition somewhere
                    // to be able to do this properly

                    bool needSpace = false;
                    if (ct->serialMandatoryField.getValue())
                    {
                        txMess += sc->serials;
                        needSpace = true;
                    }
                    if (ct->otherExchange.getValue() || ct->otherOptionalExchange.getValue())
                    {
                        QString exch = ct->location.getValue();
                        if (!exch.isEmpty())
                        {
                            if (needSpace)
                            {
                                txMess += ' ';
                            }
                            txMess +=exch;
                        }
                    }

                }
                else if (macro == "GRID")
                {
                    txMess += ct->myloc.getLoc();
                }
                else if (macro == "NAME")
                {
                    // we don't currently know our name
                }
                else if (macro == "RUN")
                {
                    // switch to run mode?
                }
                else if (macro == "S&P")
                {
                    // switch to S&P mode

                }
                else if (macro == "SPACE")
                {
                    txMess += ' ';
                }
                else if (macro == "SENTRST")
                {
                    txMess += sc->reps;
                }
                else if (macro == "TIME2")
                {
                    txMess += QDateTime::currentDateTimeUtc().toString("HHmm");
                }
                else if (macro == "ENTER")
                {
                    // simulate "Enter" key
                    // This may well log the contact, leaving nothing
                    // useful in screenContact
                    // which is why N1MM has various "last contact" macros
                    tslf->GJVQSOLogFrame->doGJVOKButton_clicked();
                }
                else if (macro == "WIPE")
                {
                    // wipe QSO - like ESC key
                    tslf->GJVQSOLogFrame->doGJVCancelButton_clicked();
                }
                else
                {
                    mShowMessage(tr("Message <%1> contains unknown macro {%2}").arg(mess).arg(macro), this);
                }
            }
        }
        else
        {
            txMess += c;
        }
    }
    return txMess;
}
void DMButtonFrame::parseFKeyFile(QString fname, QString mode)
{
    fkeys[mode].clear();

    QFile lf(fname);

    if (!lf.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        QString ebuff = QString( "Failed to open Function Key file %1" ).arg(fname );
        MinosParameters::getMinosParameters() ->mshowMessage( ebuff );
        return;
    }
    QTextStream istr(&lf);
    while (!istr.atEnd())
    {
        // loop through file, parsing each line into a_exp entries
        // for each line, call proc_line
        // ignore comment lines. (any non alpha/num char)

        QString buff = istr.readLine(0);
        QStringList a;
        if (buff.isEmpty() || (buff[ 0 ] == '#' ))
        {
            continue;   // skip comment lines
        }
        int cp = buff.indexOf(',');
        a.push_back(buff.left(cp));
        a.push_back(buff.right(buff.length() - cp - 1));

        fkeys[mode].append(QPair<QString, QString>(a[0], a[1]));
    }
}

void DMButtonFrame::on_stopButton_clicked()
{
    // send stop transmission to sender app
    RPCGeneralClient rpc(rpcConstants::DMStopTransmit);
    rpc.queueCall( dataSender );
}

void DMButtonFrame::on_editButton_clicked()
{
    // bring up default file editor on "fkeyFileName"

    QDesktopServices::openUrl(QUrl::fromLocalFile(fkeyFileName));
}

void DMButtonFrame::on_logitButton_clicked()
{
    // simulate "Enter" key
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    tslf->GJVQSOLogFrame->doGJVOKButton_clicked();
}

void DMButtonFrame::on_wipeButton_clicked()
{
    // wipe QSO - like ESC key
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    tslf->GJVQSOLogFrame->doGJVCancelButton_clicked();
}


void DMButtonFrame::on_chooseButton_clicked()
{
    // choose which file to load from
    QString lastf = fkeyFileName;

    QString InitialDir = fkeyFileName;

    QFileInfo qf(InitialDir);

    InitialDir = qf.canonicalFilePath();
    QString Filter = tr("Contact list files (*.mc);;"
                     "All Files (*.*)") ;

    fkeyFileName = QFileDialog::getOpenFileName( this,
                       tr("Open Function Key Definitions"),
                       InitialDir,
                       Filter
                       );

    if (!fkeyFileName.isEmpty())
    {
        TContestApp::getContestApp() ->loggerBundle.setStringProfile( elpDigiFunctionKeyFile, fkeyFileName );

        ui->nameLabel->setText(tr("Data Modes Buttons from %1").arg(fkeyFileName));

        qfsw->removePath(lastf);

        fkeyFileChanged();
        qfsw->addPath(fkeyFileName);
    }
}

