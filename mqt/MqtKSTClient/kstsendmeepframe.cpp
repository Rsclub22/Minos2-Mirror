#include <QKeyEvent>

#include "callsign.h"
#include "kstcallsframe.h"
#include "kstmainwindow.h"
#include "kstmsgframe.h"

#include "kstsendmeepframe.h"
#include "ui_kstsendmeepframe.h"

KSTSendMeepFrame::KSTSendMeepFrame(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::KSTSendMeepFrame)
{
    ui->setupUi(this);
    ui->callEdit->setValidator(&mainWindow->ucValidator);
    ui->callEdit->installEventFilter(this);
    ui->msgEdit->installEventFilter(this);
    ui->genmsgButton->setDefault(true);

    QSettings settings(mainWindow->iniName, QSettings::IniFormat);
    sal = static_cast<Salutation>(settings.value("Salutation", esHiName).toInt());

    switch (sal)
    {
    case esNone:
        ui->salNonerb->setChecked(true);
        break;
    case esHi:
        ui->salHirb->setChecked(true);
        break;
    case esHiName:
        ui->salHiNamerb->setChecked(true);
        break;
    }

}

KSTSendMeepFrame::~KSTSendMeepFrame()
{
    delete ui;
}
bool KSTSendMeepFrame::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress )
    {
        QKeyEvent *ke = dynamic_cast<QKeyEvent *>(event);
        if (ke->key() == Qt::Key_Escape)
        {
            if (obj == ui->callEdit)
            {
                ui->callEdit->clear();
            }
            else if (obj == ui->msgEdit)
            {
                ui->msgEdit->clear();
            }
        }
        if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter)
        {
            if (ui->meepButton->isDefault())
            {
                ui->meepButton->click();
            }
            else if (ui->genmsgButton->isDefault())
            {
                ui->genmsgButton->click();
            }
        }
    }

    return false;    // pass the event on
}

void KSTSendMeepFrame::on_FontChanged()
{

}
void KSTSendMeepFrame::setNameFromCall(const Callsign &call, int activeChat)
{
    QSharedPointer<KstUser> user = mainWindow->getUser(KstUser(call, activeChat));

    if (user)
    {
        QString msgText;
        QString hi = tr("Hi");
        switch(sal)
        {
        case esNone:
            break;

        case esHi:
            msgText = hi + " ";
            break;

        case esHiName:
        {
            QStringList name = user->name.split(' ');
            msgText = hi + " " + name[0] + " ";
            break;
        }
        }

        ui->msgEdit->setText(msgText);

        ui->callEdit->setText(user->call.getFullCall());
    }
    else
    {
        ui->msgEdit->clear();
    }

    ui->msgEdit->setFocus();

}
void KSTSendMeepFrame::on_clearMessageButton_clicked()
{
    ui->callEdit->clear();
    ui->msgEdit->clear();
    mainWindow->kstMsgFrame->setFocus();
}
void KSTSendMeepFrame::on_genmsgButton_clicked()
{
    QString msg = ui->msgEdit->text();
    if (!msg.isEmpty())
    {
        QString msg2 = "MSG|" + QString::number(mainWindow->getActiveChat()) + "|0|" + msg + "|0|";
        mainWindow->sendKST(msg2);
    }
    ui->msgEdit->clear();
}

void KSTSendMeepFrame::on_meepButton_clicked()
{
    QString msg = ui->msgEdit->text();
    QString call = ui->callEdit->text();
    if (!msg.isEmpty() && !call.isEmpty())
    {
        QString msg = ui->msgEdit->text();
        if (!msg.isEmpty())
        {
            QString msg2 = "MSG|" + QString::number(mainWindow->getActiveChat()) + "|0|/CQ " + call + " " + msg + "|0|";
            mainWindow->sendKST(msg2);
        }
        ui->msgEdit->clear();
    }
}
void KSTSendMeepFrame::on_callEdit_textChanged(const QString & /*arg1*/)
{
    setDefaultButton(nullptr);
}
void KSTSendMeepFrame::on_msgEdit_textChanged(const QString &/*arg1*/)
{
    setDefaultButton(nullptr);
}
void KSTSendMeepFrame::setDefaultButton(QPushButton *d)
{
    mainWindow->kstCallsFrame->setDefaultButton(false);
    if (d)
    {
        ui->meepButton->setDefault(false);
        ui->genmsgButton->setDefault(false);

        d->setDefault(true);
    }
    else
        if (ui->callEdit->text().isEmpty())
        {
            ui->meepButton->setDefault(false);
            ui->genmsgButton->setDefault(true);
        }
        else
        {
            ui->genmsgButton->setDefault(false);
            ui->meepButton->setDefault(true);
        }
}

void KSTSendMeepFrame::on_salNonerb_clicked()
{
    bool s = ui->salNonerb->isChecked();
    if (s)
    {
        sal = esNone;
        QSettings settings(mainWindow->iniName, QSettings::IniFormat);
        settings.setValue("Salutation", sal);
    }
}

void KSTSendMeepFrame::on_salHirb_clicked()
{
    bool s = ui->salHirb->isChecked();
    if (s)
    {
        sal = esHi;
        QSettings settings(mainWindow->iniName, QSettings::IniFormat);
        settings.setValue("Salutation", sal);
    }
}

void KSTSendMeepFrame::on_salHiNamerb_clicked()
{
    bool s = ui->salHiNamerb->isChecked();
    if (s)
    {
        sal = esHiName;
        QSettings settings(mainWindow->iniName, QSettings::IniFormat);
        settings.setValue("Salutation", sal);
    }
}

