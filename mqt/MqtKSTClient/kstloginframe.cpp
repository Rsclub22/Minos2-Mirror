#include "kstloginframe.h"
#include "QtUtils.h"
#include "KSTLoginFrame.h"
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

   ui->active1rb->setText(services[0]);
   ui->active2rb->setText(services[1]);
   ui->active3rb->setText(services[2]);
   ui->active4rb->setText(services[3]);

   connect(ui->active1rb, &QRadioButton::clicked, this, &KSTLoginFrame::activerb_clicked);
   connect(ui->active2rb, &QRadioButton::clicked, this, &KSTLoginFrame::activerb_clicked);
   connect(ui->active3rb, &QRadioButton::clicked, this, &KSTLoginFrame::activerb_clicked);
   connect(ui->active4rb, &QRadioButton::clicked, this, &KSTLoginFrame::activerb_clicked);
}
void KSTLoginFrame::logincb_stateChanged(int /*arg1*/)
{
    QStringList s;
    QVector<int> v;
    QVector<int> a;

    resetVectors(ui->login1cb, 1, s, v, a);
    resetVectors(ui->login2cb, 2, s, v, a);
    resetVectors(ui->login3cb, 3, s, v, a);
    resetVectors(ui->login4cb, 4, s, v, a);

    mainWindow->kstChatSelection = v;
    if (a.count())
    {
        setActive(a[0]);
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

void KSTLoginFrame::setActive(int chat)
{
    if (mainWindow->kstChatSelection.contains(chat))
    {
        switch(chat)
        {
        case 1:
            ui->active1rb->setChecked(true);
            break;
        case 2:
            ui->active2rb->setChecked(true);
            break;
        case 3:
            ui->active3rb->setChecked(true);
            break;
        case 4:
            ui->active4rb->setChecked(true);
            break;
        }
        mainWindow->setActiveChat(chat);
    }
    mainWindow->checkAwayButton();
}
void KSTLoginFrame::checkActive()
{
    if (mainWindow->kstChatSelection.count() > 0 && !mainWindow->kstChatSelection.contains( mainWindow->getActiveChat()))
    {
        int a = mainWindow->kstChatSelection[0];
        setActive(a);
    }
}
void KSTLoginFrame::activerb_clicked()
{
    if (ui->active1rb->isChecked())
    {
        mainWindow->setActiveChat(1);
    }
    else if (ui->active2rb->isChecked())
    {
        mainWindow->setActiveChat(2);
    }
    else if (ui->active3rb->isChecked())
    {
        mainWindow->setActiveChat(3);
    }
    else if (ui->active4rb->isChecked())
    {
        mainWindow->setActiveChat(4);
    }
    QSettings settings(mainWindow->iniName, QSettings::IniFormat);
    settings.setValue("active", QString::number(mainWindow->getActiveChat()));
    mainWindow->checkAwayButton();
}


void KSTLoginFrame::resetVectors(QCheckBox *cb, int c, QStringList &s, QVector<int> &v, QVector<int> &a)
{
    bool cbChecked = cb->isChecked();
    QRadioButton *rb = nullptr;
    switch (c)
    {
    case 1:
        rb = ui->active1rb;
        break;
    case 2:
        rb = ui->active2rb;
        break;
    case 3:
        rb = ui->active3rb;
        break;
    case 4:
        rb = ui->active4rb;
        break;
    }
    if (!mainWindow->kstChatSelection.contains(c) && cbChecked)
    {
        // not selected -> selected

        s.append(QString::number(c));
        v.append(c);
        if (rb)
        {
            rb->setVisible(true);
        }
        setActive(c);
        a.append(c);
    }
    else if (mainWindow->kstChatSelection.contains(c) && !cbChecked)
    {
        // selected -> not selected
        if (rb)
        {
            rb->setVisible(false);
        }
    }
    else if (mainWindow->kstChatSelection.contains(c))
    {
        s.append(QString::number(c));
        v.append(c);
    }
    else if (!mainWindow->kstChatSelection.contains(c) && !cbChecked)
    {
        if (rb)
        {
            rb->setVisible(false);
        }
    }
    mainWindow->checkAwayButton();
}

