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


TransVertSetupForm::TransVertSetupForm(TransVertParams* _transvertData, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::transVertSetupForm)
{

    ui->setupUi(this);
    transVertData = _transvertData;

    radioFreqEdit = new FocusWatcher(ui->radioFreq);
    targetFreqEdit = new FocusWatcher(ui->targetFreq);

    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    //connect(ui->targetFreq, SIGNAL(editingFinished()), this, SLOT(targetEditFinished()));
    //connect(ui->radioFreq, SIGNAL(editingFinished()), this, SLOT(radioEditFinished()));
    connect(radioFreqEdit, SIGNAL(focusChanged(QObject *, bool, QFocusEvent * )), this, SLOT(radioFreqEditfocusChange(QObject *, bool, QFocusEvent *)));
    connect(targetFreqEdit, SIGNAL(focusChanged(QObject *, bool, QFocusEvent * )), this, SLOT(targetFreqEditfocusChange(QObject *, bool, QFocusEvent *)));

    connect(ui->transVertSwNum, SIGNAL(editingFinished()), this, SLOT(transVertSwNumSel()));


}




void TransVertSetupForm::radioFreqEditfocusChange(QObject * /*obj*/, bool fIn, QFocusEvent * /*event*/)
{
    if (fIn)
    {
        //ui->radioFreq->selectAll();
    }
    else
    {
        // may need to compare real freq here!
        if (Frequency(ui->radioFreq->text().remove('.')) != transVertData->radioFreq)
        {

            if (ui->radioFreq->text().isEmpty() || ui->radioFreq->text() == "0.0")
            {
                transVertData->transVertOffset.clear();
                // display
                ui->offsetFreq->setText(transVertData->transVertOffset.str());
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

void TransVertSetupForm::targetFreqEditfocusChange(QObject * /*obj*/, bool fIn, QFocusEvent * /*event*/)
{
    if (fIn)
    {
        //ui->targetFreq->setSelection(0, ui->targetFreq->text().count());
    }
    else
    {
        // may need to compare real freq here!
        if (Frequency(ui->targetFreq->text().remove('.')) != transVertData->targetFreq) // changed?
        {
            if (ui->targetFreq->text().isEmpty() || ui->targetFreq->text() == "0.0")
            {
                transVertData->transVertOffset.clear();
                // display
                ui->offsetFreq->setText(transVertData->transVertOffset.str());
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
    txf = convertFreqToFullDigit(txf).remove('.');
    transVertData->radioFreq = Frequency(txf);
    // convert target freq
    targetf = convertFreqToFullDigit(targetf).remove('.');
    transVertData->targetFreq = Frequency(targetf);

    // check target freq in band
    if (transVertData->targetFreq >= transVertData->fLow && transVertData->targetFreq <= transVertData->fHigh)
    {
        transVertData->transVertOffset = transVertData->targetFreq - transVertData->radioFreq;

        // display
        ui->offsetFreq->setText(transVertData->transVertOffset.str());

        transVertOffsetOk = true;
        transVertValueChanged = true;
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Target Freq. is out of band for %1").arg(transVertData->band));
        msgBox.exec();
        ui->targetFreq->setFocus();
        return;
    }

}


void TransVertSetupForm::setRadioFreqBox(Frequency f)
{
    ui->radioFreq->setText(f.str());
}

void TransVertSetupForm::setTargetFreqBox(Frequency f)
{
    ui->targetFreq->setText(f.str());
}


void TransVertSetupForm::setOffsetFreqLabel(Frequency f)
{
    ui->offsetFreq->setText(f.str());
}



/********************* TransVert Switch Number *********************************/

// need some validation here...

void TransVertSetupForm::transVertSwNumSel()
{
    QString numSel = ui->transVertSwNum->text().trimmed();
    if (numSel != transVertData->transSwitchNum)
    {
        QRegularExpression re = QRegularExpression(anchoredPattern("\\d*"));    // a digit (\d), zero or more times (*)
        QRegularExpressionMatch rem = re.match(numSel);
        if (rem.hasMatch())
        {
            transVertData->transSwitchNum = numSel;
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



