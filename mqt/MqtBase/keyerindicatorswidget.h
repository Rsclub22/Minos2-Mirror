#ifndef KEYERINDICATORSWIDGET_H
#define KEYERINDICATORSWIDGET_H

#include <QFrame>
#include <QObject>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include  <QHBoxLayout>
class KeyerIndicatorsWidget : public QFrame
{
      Q_OBJECT

public:

    KeyerIndicatorsWidget(QWidget *parent = nullptr);


    void setKeyerAvailableIndicatorOnOff(bool on);
    void setRepeatIndicatorOnOff(bool on);
    void setEOMLabelText(const QString text);

    void clearEOMLabelText();
private:

    QGroupBox *indicatorGrpBox;

    QHBoxLayout *layout;
    QPushButton *keyerAvailIndicator;
    QPushButton *repeatIndicator;
    QLabel *eomText;
};

#endif // KEYERINDICATORSWIDGET_H
