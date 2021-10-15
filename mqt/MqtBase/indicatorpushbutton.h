#ifndef INDICATORPUSHBUTTON_H
#define INDICATORPUSHBUTTON_H

#include <QPushButton>
#include <QObject>

class IndicatorPushButton : public QPushButton
{

    Q_OBJECT

public:
    IndicatorPushButton(QWidget* parent = nullptr);
    virtual ~IndicatorPushButton();

    virtual QSize minimumSizeHint() const override;
    virtual QSize sizeHint() const override;

private:

    QSize buttonSize;
};

#endif // INDICATORPUSHBUTTON_H
