#ifndef CWENTRYWIDGET_H
#define CWENTRYWIDGET_H

#include <QFrame>
#include <QObject>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>


class CwEntryWidget : public QFrame
{
    Q_OBJECT

public:

    CwEntryWidget(QWidget *parent = nullptr);

    QString getCwEntryText();
    void setCwFreeTextIndicatorOnOff(bool on);
    void setValidator(const QString validChars, const int maxNumChars);

    void selectAllText();
signals:

    void cwEntryReturnPressed();

private:

    QHBoxLayout *layout;
    QPushButton *cwPlayingIndicator;
    QLineEdit *cwEntry;


    void addCwEntryWidgets();




};

#endif // CWENTRYWIDGET_H
