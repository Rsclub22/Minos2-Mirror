#ifndef KEYERERRORMESSAGEWIDGET_H
#define KEYERERRORMESSAGEWIDGET_H

#include <QFrame>
#include <QObject>
#include <QLabel>
#include <QHBoxLayout>

class KeyerErrorMessageWidget : public QFrame
{
    Q_OBJECT

public:

    KeyerErrorMessageWidget(QWidget *parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;



    void setErrorMessage(const QString errorMsg);
    void clearErrorMessage();
    void showTemporaryErrorMessage(const QString &msg, int timeoutMs, const QColor &colour);

private:

    QHBoxLayout *layout;

    QLabel *errorText;
};

#endif // KEYERERRORMESSAGEWIDGET_H
