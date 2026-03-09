#include "kstloginframe.h"
#include "QtUtils.h"
#include "kstactivechatsframe.h"
#include "kstmainwindow.h"
#include "ui_kstloginframe.h"

KSTLoginFrame::KSTLoginFrame(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::KSTLoginFrame)
{
    ui->setupUi(this);
}

KSTLoginFrame::~KSTLoginFrame()
{
    delete ui;
}

void KSTLoginFrame::on_FontChanged()
{

}

void KSTLoginFrame::setLogins(QStringList selections)
{
    for (auto const &i: QASCONST(selections))
    {
        int s = i.toInt();
        if (s <= 4 && s > 0)
        {
            switch(s)
            {
            case 1:
                ui->login1cb->setChecked(true);
                break;
            case 2:
                ui->login2cb->setChecked(true);
                break;
            case 3:
                ui->login3cb->setChecked(true);
                break;
            case 4:
                ui->login4cb->setChecked(true);
                break;
            }
        }
    }
}
void KSTLoginFrame::setLoginTexts(QStringList services)
{
    ui->login1cb->setText(services[0]);
    ui->login2cb->setText(services[1]);
    ui->login3cb->setText(services[2]);
    ui->login4cb->setText(services[3]);

   connect(ui->login1cb, &QCheckBox::stateChanged, this, &KSTLoginFrame::logincb_stateChanged);
   connect(ui->login2cb, &QCheckBox::stateChanged, this, &KSTLoginFrame::logincb_stateChanged);
   connect(ui->login3cb, &QCheckBox::stateChanged, this, &KSTLoginFrame::logincb_stateChanged);
   connect(ui->login4cb, &QCheckBox::stateChanged, this, &KSTLoginFrame::logincb_stateChanged);

}
void KSTLoginFrame::logincb_stateChanged(int /*arg1*/)
{
    QStringList s;
    QVector<int> v;
    QVector<int> a;

    mainWindow->kstActiveChatsFrame->resetVectors(ui->login1cb, 1, s, v, a);
    mainWindow->kstActiveChatsFrame->resetVectors(ui->login2cb, 2, s, v, a);
    mainWindow->kstActiveChatsFrame->resetVectors(ui->login3cb, 3, s, v, a);
    mainWindow->kstActiveChatsFrame->resetVectors(ui->login4cb, 4, s, v, a);

    mainWindow->kstChatSelection = v;
    if (a.count())
    {
        mainWindow->kstActiveChatsFrame->setActive(a[0]);
    }
    mainWindow->doLoginChanges();
    QSettings settings(mainWindow->iniName, QSettings::IniFormat);
    settings.setValue("service", s.join(":"));
}

void KSTLoginFrame::on_KSTTestButton_clicked()
{
    mainWindow->do_KSTTestButton_clicked();
}

void KSTLoginFrame::do_logincb_stateChanged()
{
    logincb_stateChanged(0);
}

