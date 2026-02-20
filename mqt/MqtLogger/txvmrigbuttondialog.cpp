#include <QMessageBox>
#include <QSettings>
#include "MTrace.h"
#include "regsettings.h"
#include "txvmrigbuttondialog.h"
#include "ui_txvmrigbuttondialog.h"
#include "txkeyerCommonConstants.h"

#include <QDebug>



const int MAX_CW_MESSAGE_LENGTH = 30;


TxVmRigButtonDialog::TxVmRigButtonDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TxVmRigButtonDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    RegSettings settings;
    QByteArray geometry = settings.getSettings().value("TxVmRigButtonDialog/geometry").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);


    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &TxVmRigButtonDialog::on_okButtonClicked);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &TxVmRigButtonDialog::on_cancelbuttonClicked);
    connect(ui->txCwMessageLineEdit, &QLineEdit::editingFinished, this, &TxVmRigButtonDialog::on_txCwMessageEditingFinshed);
    connect(ui->txCwMessageLineEdit, &QLineEdit::cursorPositionChanged, this, &TxVmRigButtonDialog::onCwMessageCursorChanged);
    connect(ui->txSerialMessageLineEdit, &QLineEdit::editingFinished, this, &TxVmRigButtonDialog::on_txSerialMessageEditingFinshed);
    connect(ui->txVmRepeatPauseDur , &QLineEdit::editingFinished, this, &TxVmRigButtonDialog::onVmRepeatPauseDurEditingFinished);
    connect(ui->txVmMessageDur , &QLineEdit::editingFinished, this, &TxVmRigButtonDialog::onVmMessageDurEditingFinished);
}

TxVmRigButtonDialog::~TxVmRigButtonDialog()
{
    delete ui;
}

void TxVmRigButtonDialog::doCloseEvent()
{
    RegSettings settings;
    settings.getSettings().setValue("TxVmRigButtonDialog/geometry", saveGeometry());
}
void TxVmRigButtonDialog::reject()
{
    doCloseEvent();
    QDialog::reject();
}
void TxVmRigButtonDialog::accept()
{
    doCloseEvent();
    QDialog::accept();
}

void TxVmRigButtonDialog::setCwMessageTextBoxVisible(bool visible)
{
    ui->txCwMessageLineEdit->setVisible(visible);
    ui->cwMessageTextLabel->setVisible(visible);
    ui->cwMsgLengthLabel->setVisible(visible);
}

void TxVmRigButtonDialog::setSerialMessageTextBoxVisible(bool visible)
{
    ui->txSerialMessageLineEdit->setVisible(visible);
    ui->serialMessageLabel->setVisible(visible);
}


void TxVmRigButtonDialog::setCwValidatorCwCharList(QString validCwCharList)
{
    validCwCharacterList = validCwCharList;
    populateInfoPanelSupportedCwChars(validCwCharacterList);
}



void TxVmRigButtonDialog::populateInfoPanelSupportedCwChars(QString validCwCharList)
{
    QString alpha;
    QString nonAlpha;

    foreach (const QChar cwChar, validCwCharList)
    {
        if (cwChar.isLetter())
        {
            alpha.append(cwChar);
        }
        else
        {
            nonAlpha.append(cwChar);
        }

    }


    ui->listOfSupportedCwCharsLabel->setText(alpha + "\n" + nonAlpha);

}

void TxVmRigButtonDialog::populateInfoPanelSupportedSpecialChars()
{
    if (supportSpecialCwChars)
    {
        ui->listOfSupportedCwSpecialCharsLabel->setVisible(true);
        QString displayText = supportedCwSpecialCharsList.join(',');
        ui->listOfSupportedCwSpecialCharsLabel->setText(displayText);
    }
    else
    {
        ui->listOfSupportedCwSpecialCharsLabel->setVisible(false);

    }
}

void TxVmRigButtonDialog::setMaxNumberCwCharactersText(int maxNumCwChars)
{
    ui->maxNumCwCharsTxt->setText(QString::number(maxNumCwChars));
}

void TxVmRigButtonDialog::setCwSupportCharsGroupBoxVisible(bool visible)
{
    ui->supportedCwCharsGroupBox->setVisible(visible);
}

void TxVmRigButtonDialog::setCwSupportSpecialCharsGroupBoxVisible(bool visible)
{
     ui->supportedSpecialCwCharsGroupBox->setVisible(visible);
}

void TxVmRigButtonDialog::setCwInfoPanelVisible(bool visible)
{
    ui->cwInfoFrame->setVisible(visible);
    ui->cwCharacterInfoSupportLine->setVisible(visible);
}


void TxVmRigButtonDialog::setCwValidatorMaxCwMessageLength(int maxNumChars)
{
    maximumNumCwChars = maxNumChars;
}



void TxVmRigButtonDialog::setCwSupportSpecialChar(bool radioSupportSpecialChar)
{
    supportSpecialCwChars = radioSupportSpecialChar;
}

void TxVmRigButtonDialog::setSpecialCwCharLists(QMap<QString, QChar> &specialCharMap)
{
    supportedCwSpecialCharsList = specialCharMap.keys();
    populateInfoPanelSupportedSpecialChars();   // for display
}


void TxVmRigButtonDialog::setVmTypeAndRadioModelLabel(QString radioModel)
{
    ui->txVmTypeLbl->setText(QString("%1 - %2").arg(vmData->getType()).arg(radioModel));
}


void TxVmRigButtonDialog::setVmData(TxKeyerParams* vmData_)
{
    vmData = vmData_;
    ui->txVmNameEdit->setText(vmData->getbuttonName());
    ui->txCwMessageLineEdit->setText(vmData->getKeyerCwMessage());
    ui->txVmRepeatChkBox->setChecked(vmData->getKeyerRepeatFlag());
    ui->txVmRepeatPauseDur->setText(QString::number(vmData->getKeyerRepeatPauseDur()));
    ui->txVmMessageDur->setText(QString::number(vmData->getKeyerDuration()));
}


void TxVmRigButtonDialog::setRadioNameLbl(QString radioName)
{
    ui->radioNameLbl->setText(radioName);
}


void TxVmRigButtonDialog::on_txCwMessageEditingFinshed()
{
    QString txt = ui->txCwMessageLineEdit->text();
    checkLengthOfCwMessage(txt.length());

}

void TxVmRigButtonDialog::on_txSerialMessageEditingFinshed()
{

}

void TxVmRigButtonDialog::setDialogForEomType(TxKeyerCommon::KeyerEomTypes eomType)
{
    if (eomType == TxKeyerCommon::KeyerEomTypes::Timer)
    {
        ui->messageDurLbl->setVisible(true);
        ui->txVmMessageDur->setVisible(true);
    }
    else
    {
        ui->messageDurLbl->setVisible(false);
        ui->txVmMessageDur->setVisible(false);
    }

}

bool TxVmRigButtonDialog::checkLengthOfCwMessage(int length)
{
    if (length > MAX_CW_MESSAGE_LENGTH)
    {
        QMessageBox msgBox;
        msgBox.setText(tr("CW Message too long - Max %1 chars.").arg(MAX_CW_MESSAGE_LENGTH));
        msgBox.exec();
        return false;
    }

    return true;
}

void TxVmRigButtonDialog::onVmRepeatPauseDurEditingFinished()
{
    int dur_ = 0;
    validateDur(tr("Repeat Pause"), ui->txVmRepeatPauseDur->text(), dur_);
}

void TxVmRigButtonDialog::onVmMessageDurEditingFinished()
{
    int dur_ = 0;
    validateDur(tr("Message"), ui->txVmMessageDur->text(), dur_);
}

bool TxVmRigButtonDialog::validateDur(QString durName, QString dur, int& dur_)
{
    bool ok;
    int d = dur.trimmed().toInt(&ok);
    if (ok && (d >= REPEAT_DUR_MIN && d <= REPEAT_DUR_MAX))
    {
        dur_ = d;
        return true;
    }

    QMessageBox msgBox;
    msgBox.setText(tr("%1 Duration ").arg(durName) + dur + tr(" - out of range"));
    msgBox.setInformativeText(tr("Please set value between 0 and 180 seconds"));
    msgBox.exec();
    return false;

}

void TxVmRigButtonDialog::on_okButtonClicked()
{
    int repeatPauseDur_ = 0;
    if (!validateDur(tr("Repeat Pause"), ui->txVmRepeatPauseDur->text(), repeatPauseDur_))
    {
        return;
    }

    int messageDur_ = 0;
    if (!validateDur(tr("Message"), ui->txVmMessageDur->text(), messageDur_))
    {
        return;
    }


    if (!checkLengthOfCwMessage(ui->txCwMessageLineEdit->text().length()))
    {
        return;
    }

    QString input = ui->txCwMessageLineEdit->text();
    bool specialCharPresent = false;
    bool allSpecialCharOK = false;
    QStringList specialCharErrorList;

    int currentIndex = input.indexOf(TxKeyerCommon::specialCwCharEscapeChar);
    while (currentIndex != -1)
    {
        specialCharPresent = true;

        QString spChar = input.mid(currentIndex + 1, 2);
        if (supportedCwSpecialCharsList.contains(spChar))
        {
            allSpecialCharOK = true;
        }
        else
        {
            allSpecialCharOK = false;
            spChar = TxKeyerCommon::specialCwCharEscapeChar + spChar;
            specialCharErrorList.append(spChar);
        }

        currentIndex = input.indexOf(TxKeyerCommon::specialCwCharEscapeChar, currentIndex + 1);

    }

    if (specialCharPresent)
    {
        if (allSpecialCharOK)
        {
            trace(QString("Special CW character in message OK"));
        }
        else
        {
            trace(QString("Error in special CW character in message %1").arg(specialCharErrorList.join(',')));
            QMessageBox msgBox;
            msgBox.setText(tr("%1 Special Characters ").arg(tr("Invalid")));
            msgBox.setInformativeText(tr("These special characters are invalid for this radio: %1\nPlease edit or remove").arg(specialCharErrorList.join(',')));
            msgBox.exec();
            return;

        }
    }





    QString name = ui->txVmNameEdit->text();
    vmData->setButtonName(name);
    vmData->setKeyerCwMessage(ui->txCwMessageLineEdit->text());
    vmData->setKeyerRepeatPauseDur(repeatPauseDur_);
    vmData->setKeyerDuration(messageDur_);
    vmData->setKeyerRepeatFlag(ui->txVmRepeatChkBox->isChecked());
    accept();

}


void TxVmRigButtonDialog::on_cancelbuttonClicked()
{
    reject();
}

void TxVmRigButtonDialog::setCwCharInputValidator()
{
    cwCharValidator.setValidCwCharStr(validCwCharacterList + TxKeyerCommon::specialCwCharEscapeChar);
    cwCharValidator.setMaxNumCwChars(maximumNumCwChars);
    //cwCharValidator.setSupportedSpecialChars(supportedCwSpecialCharsList);
    ui->txCwMessageLineEdit->setValidator(&cwCharValidator);
}

void TxVmRigButtonDialog::onCwMessageCursorChanged()
{
    int length = ui->txCwMessageLineEdit->text().length();
    ui->cwMsgLengthLabel->setText(QString::number(length));
}



