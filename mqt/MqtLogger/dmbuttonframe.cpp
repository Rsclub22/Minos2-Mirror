#include <QString>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextStream>
#include <QPushButton>
#include <QFileDialog>
#include <QDesktopServices>
#include <QFileSystemWatcher>

#include "ContestApp.h"
#include "LoggerContest.h"
#include "MShowMessageDlg.h"
#include "MinosLoggerEvents.h"
#include "MinosParameters.h"
#include "rigcommon.h"

#include "MinosRPC.h"
#include "dmbuttonframe.h"
#include "fileutils.h"
#include "tlogcontainer.h"
#include "tsinglelogframe.h"
#include "MTrace.h"

#include "ui_dmbuttonframe.h"

DMButtonFrame::DMButtonFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::DMButtonFrame)
{
    ui->setupUi(this);

    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::fKey, this, &DMButtonFrame::fKey);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::SandPChanged, this, &DMButtonFrame::sandPChanged);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::DMMess, this, &DMButtonFrame::DMMess);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::modeChange, this, &DMButtonFrame::onModeChange);

    TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpDigiFunctionKeyFile, fkeyFileName );

    // If we haven't already done so, copy issue fkey file to a local copy
    // so that an installation can overwrite the original without losing
    // our changes.

    QSharedPointer<ProfileEntry> &dfkd = TContestApp::getContestApp() ->loggerBundle.bundleFile->GetKey( elpDigiFunctionKeyFile );

    if (dfkd->sdefaultval == fkeyFileName)
    {
        QString dstItemPath = ExtractFileDir(fkeyFileName);
        QString fname = ExtractFileName(fkeyFileName);
        QString dname = "my_" + fname;
        dstItemPath = dstItemPath + "/" + dname;
        if (!FileExists(dstItemPath))
        {
            QFile::copy(fkeyFileName, dstItemPath);
            fkeyFileName = dstItemPath;
        }
    }
    fButtons << ui->F1Button << ui->F2Button << ui->F3Button << ui->F4Button << ui->F5Button << ui->F6Button;
    fButtons << ui->F7Button << ui->F8Button << ui->F9Button << ui->F10Button << ui->F11Button << ui->F12Button;

    int i = Qt::Key_F1;
    for (auto b: QASCONST(fButtons))
    {
        b->setProperty("KeyNo", i++);
        b->setText("");
        connect(b, &QPushButton::clicked, this, &DMButtonFrame::fButtonClicked);
    }

    ui->nameLabel->setText(tr("Data Modes Buttons from %1").arg(fkeyFileName));

    ui->FButtonFrame->setEnabled(false);

    ui->fkeysetCombo->addItem(currentName);
}

DMButtonFrame::~DMButtonFrame()
{
    delete ui;
}
void DMButtonFrame::DMMess(AnalysePubSubNotify an)
{
    if (an.getKey() == rpcConstants::DMSender)
    {
        dataSender = an.getPublisherProgram() + "@" + an.getPublisherRouter();

        trace(QString("Datasender set to %1").arg(dataSender));

        fkeyFileChanged();
        qfsw = new QFileSystemWatcher(this);
        qfsw->addPath(fkeyFileName);
        connect(qfsw, &QFileSystemWatcher::fileChanged, this, &DMButtonFrame::fkeyFileChanged);
    }
}
bool  DMButtonFrame::isDataMode()
{
    return  curMode == rigcommon::convertModeToQString(MODE::USB)
        || curMode == rigcommon::convertModeToQString(MODE::LSB)
        || curMode == rigcommon::convertModeToQString(MODE::FM)
        || curMode == hamlibData::PH;

}
void DMButtonFrame::onModeChange(QString mode)
{
    curMode = mode;
    MinosRPC *rpc = MinosRPC::getMinosRPC();
    rpc->publish( rpcConstants::DMCat, rpcConstants::DMMode, mode, psPublished );
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
    fKey(ct, kno, 0);
}
void DMButtonFrame::setContest(BaseContestLog *c)
{
    ct = dynamic_cast<LoggerContestLog *>( c);
    if (ct)
    {
        currentName = ct->currentFKeySet.getValue();
        ui->fkeysetCombo->setCurrentText(currentName);
        QString mode = ct->currentMode.getValue();
        onModeChange(mode);
    }
}
void DMButtonFrame::fKey(BaseContestLog *c, int key, int carr)
{
    if (c && c == ct && isDataMode())
    {
        if (key >= Qt::Key_F1 && key <= Qt::Key_F12 && fkeys["Digi"][currentName].size() == 24)
        {
            TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
            int spoffset = tslf->GJVQSOLogFrame->getSandP()?12:0;
            QPair<QString, QString> mess = fkeys["Digi"][currentName][key - Qt::Key_F1 + spoffset];

            QString toSend = parseFKeyMessage(mess.second);

            // send transmission to sender app

            RPCGeneralClient rpc(rpcConstants::DMTransmit);
            QSharedPointer<RPCParam>st(new RPCParamStruct);
            st->addMember( toSend, rpcConstants::DMTransmit );
            st->addMember(carr, rpcConstants::DMMarkFreq);
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
    ui->FButtonFrame->setEnabled(false);
    MinosRPC *rpc = MinosRPC::getMinosRPC();

    if (fkeys["Digi"][currentName].size() == 24)
    {
        for (int i = 0; i < 12; i++)
        {
            QString keytop = fkeys["Digi"][currentName][i + (s?12:0)].first;

            fButtons[i]->setText(keytop);
        }
        ui->FButtonFrame->setEnabled(true);

        QString fkeystring = getFKeysString();

        rpc->publish( rpcConstants::DMCat, rpcConstants::DMFKeys, fkeystring, psPublished );

    }
    else if (fkeys["Digi"][currentName].size() == 0)
    {
        for (int i = 0; i < 12; i++)
        {
            fButtons[i]->setText(QString("F%1").arg(i, 1));
        }
        rpc->publish( rpcConstants::DMCat, rpcConstants::DMFKeys, "", psRevoked );
    }
    else {
        mShowMessage(tr("Not enough key definitions in %1").arg(fkeyFileName), this);
        rpc->publish( rpcConstants::DMCat, rpcConstants::DMFKeys, "", psRevoked );
    }
}
QString DMButtonFrame::parseFKeyMessage(QString mess)
{
    // make sure screenContact is up to date
    TSingleLogFrame *tslf = LogContainer->getCurrentLogFrame();
    tslf->GJVQSOLogFrame->getScreenEntry();
    ScreenContact *sc = &tslf->GJVQSOLogFrame->screenContact;

    // data is taken now; an {ENTER} may log the call, and clear it
    QString call = sc->cs.getFullCall();
    QString serials = sc->serials;
    QString reps = sc->reps;

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
            txMess += serials;
        }
        else if (c == '!')
        {
            txMess += call;
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
                    txMess += call;
                }
                else if (macro == "SN")
                {
                    txMess += serials;
                }
                else if (macro == "EXCH")
                {
                    // This is whatever exchange is required
                    // May have multiple elements!
                    // in particular, includes serial number

                    // we need an exchange definition somewhere
                    // to be able to do this properly

                    bool needSpace = false;
                    if (ct->serialMandatoryField.getValue() || ct->asymmetricMult.getValue())
                    {
                        txMess += serials;
                        needSpace = true;
                    }
                    if (!ct->asymmetricMult.getValue() && ct->exchangeRequired.getValue())
                    {
                        QString exch = ct->location.getValue();
                        if (!exch.isEmpty() && exch != "-")
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
                else if (macro == "SPACE")
                {
                    txMess += ' ';
                }
                else if (macro == "SENTRST")
                {
                    txMess += reps;
                }
                else if (macro == "TIME2")
                {
                    QString t2 = sc->sentExchange.getValue();
                    if (t2.isEmpty())
                    {
                        t2 = QDateTime::currentDateTimeUtc().toString("HHmm");
                    }
                    txMess += t2;
                    tslf->GJVQSOLogFrame->sentExchange = t2;
                }
                else if (macro == "LOG")
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
                else if (macro == "CALLFIELD")
                {
                    tslf->GJVQSOLogFrame->selectCallField();
                }
                else if (macro == "SERIALFIELD")
                {
                    tslf->GJVQSOLogFrame->selectSnRxField();
                }
                else if (macro == "EXCHANGEFIELD")
                {
                    tslf->GJVQSOLogFrame->selectExchField();
                }
                else
                {
                    trace(QString("Message <%1> contains unknown macro {%2}").arg(mess, macro));
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
    ui->fkeysetCombo->clear();
    nameList.clear();

    QFile lf(fname);

    if (!lf.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        QString ebuff = QString( "Failed to open Function Key file %1" ).arg(fname );
        MinosParameters::getMinosParameters() ->mshowMessage( ebuff );
        return;
    }
    bool retval = false;

    QString s = lf.readAll();
    retval = parseFKeyString(s, mode);
    if (retval == false)
    {
        mShowMessage(tr("Invalid or missing FKey definitions"), this);
    }
    else
    {
        ui->fkeysetCombo->clear();
        ui->fkeysetCombo->addItems(nameList);
        ui->fkeysetCombo->setCurrentText(currentName);
    }
}
bool DMButtonFrame::parseFKeyArray(QJsonArray s, QString keyset, QString mode)
{
    for (const auto &v:QASCONST(s))
    {
        if (v.isArray())
        {
            QJsonArray a = v.toArray();
            if (a.size() == 3)
            {
                QString fk = a[0].toString();
                QString keytop = a[1].toString();
                QString val = a[2].toString();

                QString l = fk + " " + keytop;
                l.replace("&&", "&");
                l.replace("&", "&&");

                fkeys[mode][keyset].append(QPair<QString, QString>(l, val));
            }
        }
    }
    return true;
}
bool DMButtonFrame::parseFKeyString(QString s, QString mode)
{
    QJsonParseError err;
    QJsonDocument json = QJsonDocument::fromJson(s.toUtf8(), &err);
    if (err.error)
    {
        return false;
    }
    else
    {
        if( json.isArray())
        {
            QJsonArray namearray = json.array();
            for (auto const &n: QASCONST(namearray))
            {
                QJsonObject namestruct = n.toObject();
                QString name = namestruct.value("Name").toString();
                nameList.push_back(name);

                QJsonArray run = namestruct.value("Run").toArray();

                if (!parseFKeyArray(run, name, mode) )
                {
                    // always returns true
                }

                QJsonArray sandp = namestruct.value("SandP").toArray();
                if (!parseFKeyArray(sandp, name, mode) )
                {
                    // always returns true
                }
            }
        }
    }
    return true;
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
    QString Filter = tr("Contact list files (*.json);;"
                     "All Files (*.*)") ;

    QString fName = QFileDialog::getOpenFileName( this,
                       tr("Open Function Key Definitions"),
                       InitialDir,
                       Filter
                       );

    if (!fName.isEmpty())
    {
        fkeyFileName = fName;
        TContestApp::getContestApp() ->loggerBundle.setStringProfile( elpDigiFunctionKeyFile, fkeyFileName );

        ui->nameLabel->setText(tr("Data Modes Buttons from %1").arg(fkeyFileName));

        if (qfsw)
        {
            qfsw->removePath(lastf);
        }
        else
        {
            qfsw = new QFileSystemWatcher(this);
            connect(qfsw, &QFileSystemWatcher::fileChanged, this, &DMButtonFrame::fkeyFileChanged);
        }

        fkeyFileChanged();
        qfsw->addPath(fkeyFileName);
    }
}

QString DMButtonFrame::getFKeysString() const
{
    QJsonArray ja;
    for (int i = 0; i < 12; i++)
    {
        QString val = fButtons[i]->text();

        QJsonObject jv;
        jv.insert(QString("F%1").arg(i + 1), val);

        ja.append(jv);
    }
    QJsonDocument json(ja);

    QString message(json.toJson(QJsonDocument::Compact));
    return message;

}

void DMButtonFrame::on_fkeysetCombo_textActivated(const QString &arg1)
{
    currentName = arg1;
    ct->currentFKeySet.setValue(currentName);
    ct->commonSave(false);
}

