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
    explicit ScreenConfigRow(QWidget *parent, ScreenConfig *parentc);
    ~ScreenConfigRow();

    void addLeft(ScreenConfigElement *e);
    void remove(ScreenConfigElement *e);
    void addRight(ScreenConfigElement *e);
private slots:
    void on_addBeforeButton_clicked();

    void on_removeButton_clicked();

    void on_addAfterButton_clicked();

private:
    Ui::ScreenConfigRow *ui;
    QHBoxLayout *vbl;
    ScreenConfig *parentDialog;

};

#endif // SCREENCONFIGROW_H
