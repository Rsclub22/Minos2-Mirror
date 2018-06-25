#ifndef SCREENCONFIGELEMENT_H
#define SCREENCONFIGELEMENT_H

#include <QFrame>
#include "ScreenConfigFile.h"

class ScreenConfigRow;
class SCTypeOption
{
public:
    SCType type;
    QString s;
    QString hint;
};

SCType getScreenType(QString s);
QString getScreenTypeString(SCType s);

namespace Ui {
class ScreenConfigElement;
}

class ScreenConfigElement : public QFrame
{
    Q_OBJECT

public:
    explicit ScreenConfigElement(QWidget *parent, ScreenConfigRow *parentrow);
    ~ScreenConfigElement();

    void setType(QString);
    QString getType();

private slots:
    void on_elementTypeCombo_activated(const QString &arg1);

    void on_addLeftButton_clicked();

    void on_addRightButton_clicked();

    void on_removeButton_clicked();

private:
    Ui::ScreenConfigElement *ui;
    ScreenConfigRow *parentRow;
};

#endif // SCREENCONFIGELEMENT_H
