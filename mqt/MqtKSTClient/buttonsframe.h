#ifndef BUTTONSFRAME_H
#define BUTTONSFRAME_H

#include <QFrame>

namespace Ui {
class buttonsFrame;
}

class buttonsFrame : public QFrame
{
    Q_OBJECT

public:
    explicit buttonsFrame(QWidget *parent = nullptr);
    ~buttonsFrame();

private:
    Ui::buttonsFrame *ui;
};

#endif // BUTTONSFRAME_H
