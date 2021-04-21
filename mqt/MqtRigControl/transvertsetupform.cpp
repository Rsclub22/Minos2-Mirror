/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rig Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2018
//
//
//
//
/////////////////////////////////////////////////////////////////////////////




#include "transvertsetupform.h"
#include "rigcommon.h"
#include "rigutils.h"
#include "BandList.h"
#include "cutils.h"
#include <QLineEdit>
#include <QCheckBox>


TransVertSetupForm::TransVertSetupForm(QSharedPointer<scatParams> _radioData, QString _bandName, const QVector<QSharedPointer<BandInfo> > _bands, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::transVertSetupForm),
    radioData(_radioData),
    bandName(_bandName),
    bands(_bands)
{


    ui->setupUi(this);


    radioFreqEdit = new FocusWatcher(ui->radioFreq);
    targetFreqEdit = new FocusWatcher(ui->targetFreq);

    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(radioFreqEdit, &FocusWatcher::focusChanged, this, &TransVertSetupForm::radioFreqEditfocusChange);
    connect(targetFreqEdit, &FocusWatcher::focusChanged, this, &TransVertSetupForm::targetFreqEditfocusChange);

    connect(ui->transVertSwNum, &QLineEdit::editingFinished, this, &TransVertSetupForm::transVertSwNumSel);


}




void TransVertSetupForm::radioFreqEditfocusChange(QObject *obj, bool fIn, QFocusEvent *event)
{
    Q_UNUSED(obj)
    Q_UNUSED(fIn)
    Q_UNUSED(event)

    if (QApplication::focusWidget() == ui->radioFreq || QApplication::focusWidget() == ui->targetFreq)
    {
        return;
    }
    else
    {
        // may need to compare real freq here!
        if (Frequency(ui->radioFreq->text().remove('.')) != radioData->transVertSettings.value(bandName)->radioFreq)
        {

            if (ui->radioFreq->text().isEmpty() || ui->radioFreq->text() == "0.0")
            {
                radioData->transVertSettings.value(bandName)->transVertOffset.clear();
                // display
                setOffsetFreqLabel(radioData->transVertSettings.value(bandName)->transVertOffset);
                return;
            }
            QString txf = ui->radioFreq->text().trimmed().remove(QRegularExpression("^[0]*"));
            if (valInputFreq(txf, tr(RADIO_FREQ_EDIT_ERR_MSG)))
            {
               radioFreqOK = true;
               if (validateFreqTxtInput(convertFreqToFullDigit(ui->targetFreq->text().trimmed().remove(QRegularExpression("^[0]*")))))
               {
                   targetFreqOK = true;
                   calcOffset();
               }
            }
            else
            {
                ui->radioFreq->setFocus();
            }


        }

    }
}

void TransVertSetupForm::targetFreqEditfocusChange(QObject *obj , bool fIn, QFocusEvent *event)
{
    Q_UNUSED(obj)
    Q_UNUSED(fIn)
    Q_UNUSED(event)

    if (QApplication::focusWidget() == ui->radioFreq || QApplication::focusWidget() == ui->targetFreq)
    {
        return;
    }
    else
    {
        // may need to compare real freq here!
        if (Frequency(ui->targetFreq->text().remove('.')) != radioData->transVertSettings.value(bandName)->targetFreq) // changed?
        {
            if (ui->targetFreq->text().isEmpty() || ui->targetFreq->text() == "0.0")
            {
                radioData->transVertSettings.value(bandName)->transVertOffset.clear();
                // display
                setOffsetFreqLabel(radioData->transVertSettings.value(bandName)->transVertOffset);
                return;
            }
            QString targetf = ui->targetFreq->text().trimmed().remove(QRegularExpression("^[0]*"));
            if (valInputFreq(targetf, tr(TARGET_FREQ_EDIT_ERR_MSG)))
            {
               targetFreqOK = true;
               if (validateFreqTxtInput(convertFreqToFullDigit(ui->radioFreq->text().trimmed().remove(QRegularExpression("^[0]*")))))
               {
                   radioFreqOK = true;
                   calcOffset();
               }

            }
            else
            {
                ui->targetFreq->setFocus();
            }
         }
    }
}






/********************* TransVert Offset Freq  *********************************/


void TransVertSetupForm::calcOffset()
{

    if (!radioFreqOK || ! targetFreqOK)
    {
        return;

    }
    radioFreqOK = false;
    targetFreqOK = false;

    transVertOffsetOk = false;

    QString txf = ui->radioFreq->text().trimmed().remove( QRegularExpression("^[0]*"));
    QString targetf = ui->targetFreq->text().trimmed().remove(QRegularExpression("^[0]*"));

    // convert radio freq
    txf = convertFreqToFullDigit(txf);
    radioData->transVertSettings.value(bandName)->radioFreq = Frequency(txf);
    // convert target freq
    targetf = convertFreqToFullDigit(targetf);
    radioData->transVertSettings.value(bandName)->targetFreq = Frequency(targetf);

    // check target freq in band
    if (freqInBand(radioData->transVertSettings.value(bandName)->targetFreq, bandName))
    {
        radioData->transVertSettings.value(bandName)->transVertOffset = radioData->transVertSettings.value(bandName)->targetFreq - radioData->transVertSettings.value(bandName)->radioFreq;

        // display
        setOffsetFreqLabel(radioData->transVertSettings.value(bandName)->transVertOffset);

        transVertOffsetOk = true;
        transVertValueChanged = true;
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Target Freq. is out of band for %1").arg(radioData->transVertSettings.value(bandName)->band));
        msgBox.exec();
        ui->targetFreq->setFocus();
        return;
    }

}

bool TransVertSetupForm::freqInBand(Frequency f, QString band)
{
    for (int i = 0; i < bands.count(); i++)
    {
        if (bands[i].data()->uk == band)
        {
            if (f >= bands[i].data()->fLow && f <= bands[i].data()->fHigh)
            {
                return true;
            }
        }
    }

    return false;
}


void TransVertSetupForm::setRadioFreqBox(Frequency f)
{
    ui->radioFreq->setText(f.convertFreqStrDispSingle());
}

void TransVertSetupForm::setTargetFreqBox(Frequency f)
{
    ui->targetFreq->setText(f.convertFreqStrDispSingle());
}


void TransVertSetupForm::setOffsetFreqLabel(Frequency f)
{
    ui->offsetFreq->setText(f.convertFreqStrDispSingle());
}



/********************* TransVert Switch Number *********************************/

// need some validation here...

void TransVertSetupForm::transVertSwNumSel()
{
    QString numSel = ui->transVertSwNum->text().trimmed();
    if (numSel != radioData->transVertSettings.value(bandName)->transSwitchNum)
    {
        QRegularExpression re = QRegularExpression(anchoredPattern("\\d*"));    // a digit (\d), zero or more times (*)
        QRegularExpressionMatch rem = re.match(numSel);
        if (rem.hasMatch())
        {
            radioData->transVertSettings.value(bandName)->transSwitchNum = numSel;
            transVertValueChanged = true;
        }
        else
        {
            QMessageBox msgBox;
            msgBox.setText(QString("Digits only!"));
            msgBox.exec();
            return;
        }
    }


}

QString TransVertSetupForm::getTransVerSwNum()
{
    return ui->transVertSwNum->text().trimmed();
}

void TransVertSetupForm::setTransVerSwNum(QString s)
{
    ui->transVertSwNum->setText(s);
}



void TransVertSetupForm::setEnableTransVertSwBoxVisible(bool visible)
{
     ui->transVertSwNum->setVisible(visible);
     ui->transVertSwNumLbl->setVisible(visible);

}





/***************** Radio Antenna Switch Number  ********************************/

/*
void TransVertSetupForm::antennaNumSwSel()
{
    QString numSel = ui->radioAntSwNum->text().trimmed();
    QRegularExpression re("\\d*");  // a digit (\d), zero or more times (*)
    if (re.exactMatch(numSel))
    {
        transVertData->antSwitchNum = numSel;
        transVertValueChanged = true;
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setText(QString("Digits only!"));
        msgBox.exec();
        return;
    }
}


void TransVertSetupForm::antSwNumVisible(bool visible)
{

    ui->radioAntSwNum->setVisible(visible);
    ui->antSwNumLbl->setVisible(visible);
}

*/




void TransVertSetupForm::setUiItemsVisible(bool visible)
{
    //ui->bandSel->setVisible(visible);
    //ui->enableTransVertSw->setVisible(visible);
    ui->radioFreq->setVisible(visible);
    ui->transVertSwNum->setVisible(visible);
    //ui->BandLabel->setVisible(visible);
    ui->OffsetLabel->setVisible(visible);
}



