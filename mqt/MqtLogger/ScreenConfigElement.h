#ifndef SCREENCONFIGELEMENT_H
#define SCREENCONFIGELEMENT_H

#include <QFrame>
class ScreenConfigRow;

namespace Ui {
class ScreenConfigElement;
}

class ScreenConfigElement : public QFrame
{
    Q_OBJECT

public:
    explicit ScreenConfigElement(QWidget *parent, ScreenConfigRow *parentrow);
    ~ScreenConfigElement();

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
