#ifndef TOMEFRAME_H
#define TOMEFRAME_H

#include <QFrame>

namespace Ui {
class tomeFrame;
}

class tomeFrame : public QFrame
{
    Q_OBJECT

public:
    explicit tomeFrame(QWidget *parent = nullptr);
    ~tomeFrame();

private:
    Ui::tomeFrame *ui;
};

#endif // TOMEFRAME_H
