#include "minoskeyboard.h"
#include "ui_minoskeyboard.h"

#include <QKeyEvent>

MinosKeyboard::MinosKeyboard(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::MinosKeyboard)
{
    ui->setupUi(this);

    QList<QPushButton *> pushButtons = findChildren<QPushButton *>();

    setFocusPolicy(Qt::NoFocus);
    for (auto const &t: qAsConst(pushButtons))
    {
        t->setFocusPolicy(Qt::NoFocus);
        if (t == ui->tabButton)
        {
            ui->tabButton->setObjectName("tab");
            connect(ui->tabButton, &QPushButton::clicked, this, &MinosKeyboard::onTabButton);
        }
        else if (t == ui->returnButton)
        {
            ui->spaceButton->setObjectName(" ");
            connect(ui->spaceButton, &QPushButton::clicked, this, &MinosKeyboard::onKeyButton);
        }
        else if (t == ui->escButton)
        {
            ui->escButton->setObjectName("esc");
            connect(ui->escButton, &QPushButton::clicked, this, &MinosKeyboard::onEscButton);
        }
        else
        {
            t->setObjectName(t->text());
            connect(t, &QPushButton::clicked, this, &MinosKeyboard::onKeyButton);
        }
    }
}

MinosKeyboard::~MinosKeyboard()
{
    delete ui;
}
void MinosKeyboard::sendChar(QChar charToSend)
{
     QApplication::sendEvent(parentWidget()->focusWidget(), new QKeyEvent(QEvent::KeyPress, charToSend.unicode(), Qt::NoModifier, QString(charToSend)));
}


void MinosKeyboard::onKeyButton()
{
    QObject *s = sender();
    if (s)
    {
        QString o = s->objectName();
        sendChar(o[0]);
    }
}
void MinosKeyboard::onTabButton()
{
    // send Tab
}

void MinosKeyboard::onReturnButton()
{
    // send Return
}
void MinosKeyboard::onEscButton()
{
    // send Esc
}
