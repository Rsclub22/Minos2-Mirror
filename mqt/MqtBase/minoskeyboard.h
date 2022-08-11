#ifndef MINOSKEYBOARD_H
#define MINOSKEYBOARD_H

#include <QFrame>

namespace Ui {
class MinosKeyboard;
}

class MinosKeyboard : public QFrame
{
    Q_OBJECT

public:
    explicit MinosKeyboard(QWidget *parent = nullptr);
    ~MinosKeyboard();

private slots:

    void onKeyButton();
    void onEscButton();
    void onTabButton();
    void onReturnButton();

private:
    Ui::MinosKeyboard *ui;

    void sendChar(QChar charToSend);
};

#endif // MINOSKEYBOARD_H
