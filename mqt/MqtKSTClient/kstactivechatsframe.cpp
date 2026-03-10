#include "kstmainwindow.h"

#include "kstactivechatsframe.h"

#include "ui_kstactivechatsframe.h"

KSTActiveChatsFrame::KSTActiveChatsFrame(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::KSTActiveChatsFrame)
{
    ui->setupUi(this);


}

KSTActiveChatsFrame::~KSTActiveChatsFrame()
{
    delete ui;
}

void KSTActiveChatsFrame::on_FontChanged()
{

}
void KSTActiveChatsFrame::setActive(int chat)
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
void KSTActiveChatsFrame::checkActive()
{
    if (mainWindow->kstChatSelection.count() > 0 && !mainWindow->kstChatSelection.contains( mainWindow->getActiveChat()))
    {
        int a = mainWindow->kstChatSelection[0];
        setActive(a);
    }
}
void KSTActiveChatsFrame::setLoginTexts(QStringList services)
{
    ui->active1rb->setText(services[0]);
    ui->active2rb->setText(services[1]);
    ui->active3rb->setText(services[2]);
    ui->active4rb->setText(services[3]);

   connect(ui->active1rb, &QRadioButton::clicked, this, &KSTActiveChatsFrame::activerb_clicked);
   connect(ui->active2rb, &QRadioButton::clicked, this, &KSTActiveChatsFrame::activerb_clicked);
   connect(ui->active3rb, &QRadioButton::clicked, this, &KSTActiveChatsFrame::activerb_clicked);
   connect(ui->active4rb, &QRadioButton::clicked, this, &KSTActiveChatsFrame::activerb_clicked);
}
void KSTActiveChatsFrame::activerb_clicked()
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


void KSTActiveChatsFrame::resetVectors(QCheckBox *cb, int c, QStringList &s, QVector<int> &v, QVector<int> &a)
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

