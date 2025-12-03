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
#include <QLineEdit>
#include <QCheckBox>
#include <QMessageBox>

#include "rigcommon.h"
#include "rigutils.h"
#include "BandList.h"
#include "cutils.h"

#include "transvertsetupform.h"

TransVertSetupForm::TransVertSetupForm(QSharedPointer<scatParams> _radioData, QString _bandName, const QVector<QSharedPointer<BandInfo> > _bands, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::transVertSetupForm),
    radioData(_radioData),
    bandName(_bandName),
    bands(_bands)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    radioFreqEdit = new FocusWatcher(ui->radioFreq);
    targetFreqEdit = new FocusWatcher(ui->targetFreq);

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
            static QRegularExpression qre1("^[0]*");
            QString txf = ui->radioFreq->text().trimmed().remove(qre1);
            if (valInputFreq(txf, tr(RADIO_FREQ_EDIT_ERR_MSG)))
            {
               radioFreqOK = true;
                static QRegularExpression qre2("^[0]*");
               if (validateFreqTxtInput(convertFreqToFullDigit(ui->targetFreq->text().trimmed().remove(qre2))))
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
            static QRegularExpression qre3("^[0]*");
            QString targetf = ui->targetFreq->text().trimmed().remove(qre3);
            if (valInputFreq(targetf, tr(TARGET_FREQ_EDIT_ERR_MSG)))
            {
               targetFreqOK = true;
                static QRegularExpression qre4("^[0]*");
               if (validateFreqTxtInput(convertFreqToFullDigit(ui->radioFreq->text().trimmed().remove(qre4))))
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

    static QRegularExpression qre1("^[0]*");
    QString txf = ui->radioFreq->text().trimmed().remove( qre1);
    QString targetf = ui->targetFreq->text().trimmed().remove(qre1);

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
        msgBox.setText(tr("Target Freq. is out of band for %1\nPlease correct the transverter entries.").arg(radioData->transVertSettings.value(bandName)->band));
        msgBox.exec();

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
    if (f == Frequency(0))
    {
      ui->radioFreq->setText("0.0");
    }
    else
    {
       ui->radioFreq->setText(f.convertFreqStrDispSingleNoTrailZero());
    }

}

void TransVertSetupForm::setTargetFreqBox(Frequency f)
{
    if (f == Frequency(0))
    {
      ui->targetFreq->setText("0.0");
    }
    else
    {
        ui->targetFreq->setText(f.convertFreqStrDispSingleNoTrailZero());
    }
}


void TransVertSetupForm::setOffsetFreqLabel(Frequency f)
{
    if (f == Frequency(0))
    {
      ui->offsetFreq->setText("0.0");
    }
    else
    {
        ui->offsetFreq->setText(f.convertFreqStrDispSingleNoTrailZero());
    }
}



/********************* TransVert Switch Number *********************************/

// need some validation here...

void TransVertSetupForm::transVertSwNumSel()
{
    QString numSel = ui->transVertSwNum->text().trimmed();
    if (numSel != radioData->transVertSettings.value(bandName)->transSwitchNum)
    {
        static QRegularExpression re = QRegularExpression(anchoredPattern("\\d*"));    // a digit (\d), zero or more times (*)
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








void TransVertSetupForm::setUiItemsVisible(bool visible)
{

    ui->radioFreq->setVisible(visible);
    ui->transVertSwNum->setVisible(visible);
    ui->OffsetLabel->setVisible(visible);
}



