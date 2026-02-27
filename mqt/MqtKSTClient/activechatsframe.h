#ifndef ACTIVECHATSFRAME_H
#define ACTIVECHATSFRAME_H

#include <QFrame>

namespace Ui {
class activeChatsFrame;
}

class activeChatsFrame : public QFrame
{
    Q_OBJECT

public:
    explicit activeChatsFrame(QWidget *parent = nullptr);
    ~activeChatsFrame();

private:
    Ui::activeChatsFrame *ui;
};

#endif // ACTIVECHATSFRAME_H
