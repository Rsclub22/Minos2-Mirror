#include <QVariant>
#include "fkeybuttonwidget.h"

FKeyButtonWidget::FKeyButtonWidget(QWidget *parent)
    : QFrame(parent)
{
    layout = new QGridLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(2);

    // Start with default 12 buttons in 4 columns
    createButtons(12, 4);
}

void FKeyButtonWidget::setButtonLayout(int numButtons, int columns)
{
    clearButtons();
    createButtons(numButtons, columns);
}

void FKeyButtonWidget::setButtonText(int buttonNum, const QString &text)
{
    if (buttonNum >= 0 && buttonNum < fButtons.size())
    {
        fButtons[buttonNum]->setText(text);
    }
}

void FKeyButtonWidget::clearAllButtons()
{
    for (auto btn : fButtons)
    {
        btn->setText(QString("F%1").arg(btn->property("KeyNo").toInt() + 1));
    }
}

QPushButton* FKeyButtonWidget::getButton(int index)
{
    if (index >= 0 && index < fButtons.size())
        return fButtons[index];
    return nullptr;
}

void FKeyButtonWidget::clearButtons()
{
    for (auto btn : fButtons)
    {
        layout->removeWidget(btn);
        btn->deleteLater();
    }
    fButtons.clear();
}

void FKeyButtonWidget::createButtons(int numButtons, int columns)
{
    for (int i = 0; i < numButtons; i++)
    {
        QPushButton *btn = new QPushButton(QString("F%1").arg(i + 1), this);
        btn->setProperty("KeyNo", i);
        connect(btn, &QPushButton::clicked, this, [this, i]() {
            emit buttonClicked(i);
        });
        fButtons.append(btn);

        int row = i / columns;
        int col = i % columns;
        layout->addWidget(btn, row, col);
    }
}
