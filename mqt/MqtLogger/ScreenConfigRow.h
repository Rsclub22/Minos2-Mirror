#ifndef SCREENCONFIGROW_H
#define SCREENCONFIGROW_H

#include <QFrame>
#include <QHBoxLayout>

class ScreenConfig;
class ScreenConfigElement;

namespace Ui {
class ScreenConfigRow;
}

class ScreenConfigRow : public QFrame
{
    Q_OBJECT

public:
    Ui::ScreenConfigRow *ui;

    explicit ScreenConfigRow(ScreenConfigElement *parentElement);
    ~ScreenConfigRow();

    QHBoxLayout *hbl = nullptr;
    ScreenConfigElement *parentElement;

    ScreenConfigElement *addLeft(ScreenConfigElement *e);
    void remove(ScreenConfigElement *e);
    ScreenConfigElement * addRight(ScreenConfigElement *e);

    bool checkOk(ScreenConfigElement *e);

private slots:
    void on_addRowBeforeButton_clicked();

    void on_removeRowButton_clicked();

    void on_addRowAfterButton_clicked();

};

#endif // SCREENCONFIGROW_H
