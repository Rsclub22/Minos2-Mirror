#include "keyererrormessagewidget.h"
#include <QTimer>

KeyerErrorMessageWidget::KeyerErrorMessageWidget(QWidget *parent)
    :QFrame(parent)
{

    layout = new QHBoxLayout();
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(2);

    QLabel *errorLabelText = new QLabel("Error: ");

    errorText = new QLabel();

    layout->addWidget(errorLabelText);
    layout->addWidget(errorText);
    layout->addStretch();

    setLayout(layout);

}





void KeyerErrorMessageWidget::setErrorMessage(const QString errorMsg)
{
    clearErrorMessage();
    errorText->setText(errorMsg);
}

void KeyerErrorMessageWidget::clearErrorMessage()
{
   errorText->clear();
}


void KeyerErrorMessageWidget::showTemporaryErrorMessage(const QString &msg, int timeoutMs, const QColor &colour)
{

    // Save the current stylesheet
    QString oldStyle = errorText->styleSheet();

    clearErrorMessage();

    // Set error color and text
    errorText->setStyleSheet(QString("color: %1;").arg(colour.name()));

    errorText->setText(msg);

    // Clear after timeoutMs milliseconds
    QTimer::singleShot(timeoutMs, this, [this, oldStyle]() {
        errorText->clear();
        errorText->setStyleSheet(oldStyle);

    });
}
