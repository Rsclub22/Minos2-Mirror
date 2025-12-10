#include "keyererrormessagewidget.h"
#include <Qtimer>

KeyerErrorMessageWidget::KeyerErrorMessageWidget(QWidget *parent)
    :QFrame(parent)
{

    layout = new QHBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(2);

    QLabel *errorLabelText = new QLabel();
    errorLabelText->setText("Error: ");

    errorText = new QLabel();




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
